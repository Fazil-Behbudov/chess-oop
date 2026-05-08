#include "game.h"
#include "board_view.h"

#include <cctype>
#include <iostream>
#include <regex>
#include <unistd.h>

namespace {
std::string trim(const std::string& input) {
    const auto first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, last - first + 1);
}
}

Game::Game()
    : turn_(Color::White), result_("?-?"), finished_(false), interactive_ui_(false), white_bottom_view_(true) {}

int Game::run() {
    // Keep rich terminal behavior only for real interactive sessions.
    interactive_ui_ = (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
    white_bottom_view_ = true;
    render_board();

    std::string command;
    while (!finished_) {
        if (interactive_ui_) {
            std::cout << "Move (eg. a1a8) ? ";
        }
        if (!std::getline(std::cin, command)) {
            break;
        }
        command = trim(command);
        if (command.empty()) {
            continue;
        }

        (void)handle_command(command);
    }

    std::cout << board_.canonical_position() << " " << result_ << "\n";
    return 0;
}

bool Game::handle_command(const std::string& command) {
    if (command == "/quit") {
        finished_ = true;
        result_ = "?-?";
        return true;
    }
    if (command == "/draw") {
        finished_ = true;
        result_ = "1/2-1/2";
        return true;
    }
    if (command == "/resign") {
        finished_ = true;
        result_ = (turn_ == Color::White) ? "0-1" : "1-0";
        return true;
    }

    // Accept O/o/0 spellings to match assignment input constraints.
    static const std::regex castle_king_pattern("^(O|o|0)-(O|o|0)$");
    static const std::regex castle_queen_pattern("^(O|o|0)-(O|o|0)-(O|o|0)$");
    if (std::regex_match(command, castle_king_pattern) || std::regex_match(command, castle_queen_pattern)) {
        return handle_castling(command);
    }

    static const std::regex move_pattern("^[a-h][1-8][a-h][1-8]$");
    if (!std::regex_match(command, move_pattern)) {
        std::cout << "Error: invalid command format.\n";
        return false;
    }

    return handle_move(command);
}

bool Game::handle_move(const std::string& command) {
    const auto origin = Square::from_algebraic(command.substr(0, 2));
    const auto destination = Square::from_algebraic(command.substr(2, 2));
    if (!origin || !destination) {
        std::cout << "Error: invalid coordinates.\n";
        return false;
    }

    std::string error;
    if (!board_.move_piece_basic(*origin, *destination, turn_, error)) {
        std::cout << error << "\n";
        return false;
    }

    const Piece* moved_piece = board_.piece_at(*destination);
    if (moved_piece) {
        std::cout << "-> move of " << moved_piece->symbol() << " " << origin->to_algebraic()
                  << destination->to_algebraic() << "\n";
    }

    if (board_.needs_promotion(*destination)) {
        if (!handle_promotion(*destination)) {
            return false;
        }
    }

    // Rotate board perspective after each legal move.
    white_bottom_view_ = !white_bottom_view_;
    render_board();
    finalize_turn_after_move();
    return true;
}

bool Game::handle_castling(const std::string& command) {
    const bool king_side = command.size() == 3;
    std::string error;
    if (!board_.castle(turn_, king_side, error)) {
        std::cout << error << "\n";
        return false;
    }

    std::cout << "-> castling " << (king_side ? "O-O" : "O-O-O") << "\n";
    // Castling is also a legal move and rotates view as well.
    white_bottom_view_ = !white_bottom_view_;
    render_board();
    finalize_turn_after_move();
    return true;
}

bool Game::handle_promotion(const Square& destination) {
    while (true) {
        if (interactive_ui_) {
            std::cout << "Promote to (Q,R,B,N) ? ";
        }

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            choice = "Q";
        }
        choice = trim(choice);
        if (choice.size() != 1) {
            std::cout << "Error: promotion must be one of Q,R,B,N.\n";
            continue;
        }

        const char code = static_cast<char>(std::toupper(static_cast<unsigned char>(choice[0])));
        PieceType promotion_type;
        switch (code) {
            case 'Q': promotion_type = PieceType::Queen; break;
            case 'R': promotion_type = PieceType::Rook; break;
            case 'B': promotion_type = PieceType::Bishop; break;
            case 'N': promotion_type = PieceType::Knight; break;
            default:
                std::cout << "Error: promotion must be one of Q,R,B,N.\n";
                continue;
        }

        std::string error;
        if (!board_.promote_pawn(destination, promotion_type, error)) {
            std::cout << error << "\n";
            return false;
        }
        std::cout << "-> promotion to " << code << "\n";
        return true;
    }
}

void Game::render_board() const {
    if (interactive_ui_) {
        // Single-board mode: clear and redraw in place.
        std::cout << "\033[H\033[2J";
    }
    BoardView::render(board_, std::cout, white_bottom_view_);
}

void Game::finalize_turn_after_move() {
    // Evaluate next side position: check, checkmate, or stalemate.
    const Color opponent = (turn_ == Color::White) ? Color::Black : Color::White;
    const bool opponent_in_check = board_.is_in_check(opponent);
    if (opponent_in_check) {
        std::cout << "-> check\n";
    }

    if (!board_.has_any_legal_move(opponent)) {
        finished_ = true;
        if (opponent_in_check) {
            result_ = (turn_ == Color::White) ? "1-0" : "0-1";
            std::cout << "-> checkmate\n";
        } else {
            result_ = "1/2-1/2";
            std::cout << "-> stalemate\n";
        }
        return;
    }

    switch_turn();
}

void Game::switch_turn() {
    turn_ = (turn_ == Color::White) ? Color::Black : Color::White;
}
