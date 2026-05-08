#include "board.h"

#include <cstdlib>
#include <utility>

namespace {
std::unique_ptr<Piece> make_piece(Color color, PieceType type) {
    return std::make_unique<Piece>(color, type);
}

Color opposite_color(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}
}

Board::Board() {
    reset_initial_position();
}

void Board::reset_initial_position() {
    for (auto& row : board_) {
        for (auto& cell : row) {
            cell.reset();
        }
    }

    place_piece(make_piece(Color::White, PieceType::Rook), Square(0, 0));
    place_piece(make_piece(Color::White, PieceType::Knight), Square(0, 1));
    place_piece(make_piece(Color::White, PieceType::Bishop), Square(0, 2));
    place_piece(make_piece(Color::White, PieceType::Queen), Square(0, 3));
    place_piece(make_piece(Color::White, PieceType::King), Square(0, 4));
    place_piece(make_piece(Color::White, PieceType::Bishop), Square(0, 5));
    place_piece(make_piece(Color::White, PieceType::Knight), Square(0, 6));
    place_piece(make_piece(Color::White, PieceType::Rook), Square(0, 7));

    place_piece(make_piece(Color::Black, PieceType::Rook), Square(7, 0));
    place_piece(make_piece(Color::Black, PieceType::Knight), Square(7, 1));
    place_piece(make_piece(Color::Black, PieceType::Bishop), Square(7, 2));
    place_piece(make_piece(Color::Black, PieceType::Queen), Square(7, 3));
    place_piece(make_piece(Color::Black, PieceType::King), Square(7, 4));
    place_piece(make_piece(Color::Black, PieceType::Bishop), Square(7, 5));
    place_piece(make_piece(Color::Black, PieceType::Knight), Square(7, 6));
    place_piece(make_piece(Color::Black, PieceType::Rook), Square(7, 7));

    for (int col = 0; col < 8; ++col) {
        place_piece(make_piece(Color::White, PieceType::Pawn), Square(1, col));
        place_piece(make_piece(Color::Black, PieceType::Pawn), Square(6, col));
    }

    en_passant_available_ = false;
    en_passant_target_ = Square();
    en_passant_pawn_square_ = Square();
}

std::string Board::canonical_position() const {
    std::string out;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board_[row][col]) {
                out += board_[row][col]->canonical_code();
            }
            out += ",";
        }
    }
    return out;
}

const Piece* Board::piece_at(const Square& square) const {
    if (!square.is_valid()) {
        return nullptr;
    }
    return board_[square.row()][square.col()].get();
}

Piece* Board::piece_at(const Square& square) {
    if (!square.is_valid()) {
        return nullptr;
    }
    return board_[square.row()][square.col()].get();
}

bool Board::is_empty(const Square& square) const {
    return piece_at(square) == nullptr;
}

bool Board::is_in_check(Color color) const {
    const Square king_square = find_king(color);
    if (!king_square.is_valid()) {
        return false;
    }
    return is_square_attacked(king_square, opposite_color(color));
}

bool Board::needs_promotion(const Square& square) const {
    const Piece* piece = piece_at(square);
    if (piece == nullptr || piece->type() != PieceType::Pawn) {
        return false;
    }
    if (piece->color() == Color::White) {
        return square.row() == 7;
    }
    return square.row() == 0;
}

bool Board::promote_pawn(const Square& square, PieceType new_type, std::string& error) {
    Piece* piece = piece_at(square);
    if (piece == nullptr || piece->type() != PieceType::Pawn) {
        error = "Error: no pawn available for promotion.";
        return false;
    }
    if (!needs_promotion(square)) {
        error = "Error: pawn is not on promotion rank.";
        return false;
    }
    if (new_type == PieceType::Pawn || new_type == PieceType::King) {
        error = "Error: invalid promotion piece.";
        return false;
    }

    const Color color = piece->color();
    board_[square.row()][square.col()] = std::make_unique<Piece>(color, new_type);
    board_[square.row()][square.col()]->mark_moved();
    return true;
}

bool Board::has_any_legal_move(Color color) {
    if (can_castle_no_commit(color, true) || can_castle_no_commit(color, false)) {
        return true;
    }

    for (int from_row = 0; from_row < 8; ++from_row) {
        for (int from_col = 0; from_col < 8; ++from_col) {
            const Piece* piece = board_[from_row][from_col].get();
            if (piece == nullptr || piece->color() != color) {
                continue;
            }
            const Square origin(from_row, from_col);
            for (int to_row = 0; to_row < 8; ++to_row) {
                for (int to_col = 0; to_col < 8; ++to_col) {
                    if (from_row == to_row && from_col == to_col) {
                        continue;
                    }
                    if (is_legal_move_no_commit(origin, Square(to_row, to_col), color)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Board::move_piece_basic(const Square& origin, const Square& destination, Color turn, std::string& error) {
    if (!origin.is_valid() || !destination.is_valid()) {
        error = "Error: coordinates must be in [a-h][1-8].";
        return false;
    }

    if (origin.row() == destination.row() && origin.col() == destination.col()) {
        error = "Error: origin and destination are identical.";
        return false;
    }

    Piece* from_piece = piece_at(origin);
    if (from_piece == nullptr) {
        error = "Error: origin square is empty.";
        return false;
    }

    if (from_piece->color() != turn) {
        error = "Error: this is not your turn for that piece.";
        return false;
    }

    Piece* to_piece = piece_at(destination);
    if (to_piece != nullptr && to_piece->color() == from_piece->color()) {
        error = "Error: destination occupied by your own piece.";
        return false;
    }

    bool is_capture = (to_piece != nullptr);
    bool is_en_passant_capture = false;
    Square en_passant_capture_square;
    if (from_piece->type() == PieceType::Pawn && !is_capture) {
        // Special case: diagonal pawn move to empty target can be en passant.
        const int drow = destination.row() - origin.row();
        const int dcol = destination.col() - origin.col();
        const int direction = (from_piece->color() == Color::White) ? 1 : -1;
        if (std::abs(dcol) == 1 && drow == direction && en_passant_available_ &&
            destination.row() == en_passant_target_.row() && destination.col() == en_passant_target_.col()) {
            Piece* passant_piece = piece_at(en_passant_pawn_square_);
            if (passant_piece != nullptr && passant_piece->type() == PieceType::Pawn &&
                passant_piece->color() != from_piece->color()) {
                is_capture = true;
                is_en_passant_capture = true;
                en_passant_capture_square = en_passant_pawn_square_;
            }
        }
    }

    if (!is_geometrically_legal_move(*from_piece, origin, destination, is_capture, error)) {
        return false;
    }
    if (would_leave_king_in_check(
            origin, destination, from_piece->color(), is_en_passant_capture, en_passant_capture_square)) {
        error = "Error: move leaves your king in check.";
        return false;
    }

    if (is_en_passant_capture) {
        board_[en_passant_capture_square.row()][en_passant_capture_square.col()].reset();
    }
    board_[destination.row()][destination.col()] = std::move(board_[origin.row()][origin.col()]);
    board_[destination.row()][destination.col()]->mark_moved();

    // Any successful move cancels previous en passant rights first.
    en_passant_available_ = false;
    en_passant_target_ = Square();
    en_passant_pawn_square_ = Square();
    Piece* moved_piece = board_[destination.row()][destination.col()].get();
    if (moved_piece != nullptr && moved_piece->type() == PieceType::Pawn) {
        const int drow = destination.row() - origin.row();
        if (std::abs(drow) == 2) {
            // A two-step pawn advance enables en passant for exactly next move.
            const int direction = (moved_piece->color() == Color::White) ? 1 : -1;
            en_passant_available_ = true;
            en_passant_target_ = Square(origin.row() + direction, origin.col());
            en_passant_pawn_square_ = destination;
        }
    }
    return true;
}

bool Board::castle(Color turn, bool king_side, std::string& error) {
    const int row = (turn == Color::White) ? 0 : 7;
    const Square king_from(row, 4);
    const Square rook_from(row, king_side ? 7 : 0);
    const Square king_to(row, king_side ? 6 : 2);
    const Square rook_to(row, king_side ? 5 : 3);

    Piece* king = piece_at(king_from);
    Piece* rook = piece_at(rook_from);
    if (king == nullptr || king->type() != PieceType::King || king->color() != turn) {
        error = "Error: king not in castling position.";
        return false;
    }
    if (rook == nullptr || rook->type() != PieceType::Rook || rook->color() != turn) {
        error = "Error: rook not in castling position.";
        return false;
    }
    if (king->has_moved() || rook->has_moved()) {
        error = "Error: castling forbidden after king/rook moved.";
        return false;
    }
    if (!is_path_clear(king_from, rook_from)) {
        error = "Error: castling path is blocked.";
        return false;
    }
    if (is_in_check(turn)) {
        error = "Error: castling while in check is forbidden.";
        return false;
    }

    // Tentatively move king+rook, validate king safety, then keep or rollback.
    auto king_piece = std::move(board_[king_from.row()][king_from.col()]);
    auto rook_piece = std::move(board_[rook_from.row()][rook_from.col()]);
    board_[king_to.row()][king_to.col()] = std::move(king_piece);
    board_[rook_to.row()][rook_to.col()] = std::move(rook_piece);

    const bool king_in_check = is_in_check(turn);
    if (king_in_check) {
        king_piece = std::move(board_[king_to.row()][king_to.col()]);
        rook_piece = std::move(board_[rook_to.row()][rook_to.col()]);
        board_[king_from.row()][king_from.col()] = std::move(king_piece);
        board_[rook_from.row()][rook_from.col()] = std::move(rook_piece);
        error = "Error: castling leaves king in check.";
        return false;
    }

    board_[king_to.row()][king_to.col()]->mark_moved();
    board_[rook_to.row()][rook_to.col()]->mark_moved();
    en_passant_available_ = false;
    en_passant_target_ = Square();
    en_passant_pawn_square_ = Square();
    return true;
}

void Board::place_piece(std::unique_ptr<Piece> piece, const Square& square) {
    if (!square.is_valid()) {
        return;
    }
    board_[square.row()][square.col()] = std::move(piece);
}

bool Board::is_path_clear(const Square& origin, const Square& destination) const {
    const int drow = destination.row() - origin.row();
    const int dcol = destination.col() - origin.col();
    const int step_row = (drow == 0) ? 0 : (drow > 0 ? 1 : -1);
    const int step_col = (dcol == 0) ? 0 : (dcol > 0 ? 1 : -1);

    int row = origin.row() + step_row;
    int col = origin.col() + step_col;
    while (row != destination.row() || col != destination.col()) {
        if (board_[row][col] != nullptr) {
            return false;
        }
        row += step_row;
        col += step_col;
    }
    return true;
}

bool Board::is_geometrically_legal_move(
    const Piece& piece,
    const Square& origin,
    const Square& destination,
    bool is_capture,
    std::string& error) const {
    const int drow = destination.row() - origin.row();
    const int dcol = destination.col() - origin.col();
    const int abs_drow = std::abs(drow);
    const int abs_dcol = std::abs(dcol);

    switch (piece.type()) {
        case PieceType::Knight: {
            if ((abs_drow == 2 && abs_dcol == 1) || (abs_drow == 1 && abs_dcol == 2)) {
                return true;
            }
            error = "Error: illegal knight move.";
            return false;
        }
        case PieceType::King: {
            if (abs_drow <= 1 && abs_dcol <= 1) {
                return true;
            }
            error = "Error: illegal king move.";
            return false;
        }
        case PieceType::Rook: {
            if (drow != 0 && dcol != 0) {
                error = "Error: illegal rook move.";
                return false;
            }
            if (!is_path_clear(origin, destination)) {
                error = "Error: piece cannot jump over obstacles.";
                return false;
            }
            return true;
        }
        case PieceType::Bishop: {
            if (abs_drow != abs_dcol) {
                error = "Error: illegal bishop move.";
                return false;
            }
            if (!is_path_clear(origin, destination)) {
                error = "Error: piece cannot jump over obstacles.";
                return false;
            }
            return true;
        }
        case PieceType::Queen: {
            const bool line = (drow == 0 || dcol == 0);
            const bool diagonal = (abs_drow == abs_dcol);
            if (!line && !diagonal) {
                error = "Error: illegal queen move.";
                return false;
            }
            if (!is_path_clear(origin, destination)) {
                error = "Error: piece cannot jump over obstacles.";
                return false;
            }
            return true;
        }
        case PieceType::Pawn: {
            const int direction = (piece.color() == Color::White) ? 1 : -1;
            const int start_row = (piece.color() == Color::White) ? 1 : 6;

            if (dcol == 0) {
                if (is_capture) {
                    error = "Error: pawn cannot capture straight ahead.";
                    return false;
                }
                if (drow == direction) {
                    return true;
                }
                if (drow == 2 * direction && origin.row() == start_row && !piece.has_moved()) {
                    const Square middle(origin.row() + direction, origin.col());
                    if (!is_empty(middle)) {
                        error = "Error: pawn cannot jump over obstacles.";
                        return false;
                    }
                    return true;
                }
                error = "Error: illegal pawn advance.";
                return false;
            }

            if (abs_dcol == 1 && drow == direction && is_capture) {
                return true;
            }

            error = "Error: illegal pawn move.";
            return false;
        }
    }

    error = "Error: unsupported piece move.";
    return false;
}

Square Board::find_king(Color color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = board_[row][col].get();
            if (piece != nullptr && piece->color() == color && piece->type() == PieceType::King) {
                return Square(row, col);
            }
        }
    }
    return Square();
}

bool Board::is_square_attacked(const Square& square, Color by_color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = board_[row][col].get();
            if (piece == nullptr || piece->color() != by_color) {
                continue;
            }
            if (piece_attacks_square(*piece, Square(row, col), square)) {
                return true;
            }
        }
    }
    return false;
}

bool Board::piece_attacks_square(const Piece& piece, const Square& origin, const Square& target) const {
    const int drow = target.row() - origin.row();
    const int dcol = target.col() - origin.col();
    const int abs_drow = std::abs(drow);
    const int abs_dcol = std::abs(dcol);

    if (drow == 0 && dcol == 0) {
        return false;
    }

    switch (piece.type()) {
        case PieceType::Pawn: {
            const int direction = (piece.color() == Color::White) ? 1 : -1;
            return drow == direction && abs_dcol == 1;
        }
        case PieceType::Knight:
            return (abs_drow == 2 && abs_dcol == 1) || (abs_drow == 1 && abs_dcol == 2);
        case PieceType::King:
            return abs_drow <= 1 && abs_dcol <= 1;
        case PieceType::Rook:
            return (drow == 0 || dcol == 0) && is_path_clear(origin, target);
        case PieceType::Bishop:
            return abs_drow == abs_dcol && is_path_clear(origin, target);
        case PieceType::Queen:
            return ((drow == 0 || dcol == 0) || (abs_drow == abs_dcol)) && is_path_clear(origin, target);
    }
    return false;
}

bool Board::is_legal_move_no_commit(const Square& origin, const Square& destination, Color turn) {
    if (!origin.is_valid() || !destination.is_valid()) {
        return false;
    }

    Piece* from_piece = piece_at(origin);
    if (from_piece == nullptr || from_piece->color() != turn) {
        return false;
    }

    Piece* to_piece = piece_at(destination);
    if (to_piece != nullptr && to_piece->color() == from_piece->color()) {
        return false;
    }

    bool is_capture = (to_piece != nullptr);
    bool is_en_passant_capture = false;
    Square en_passant_capture_square;
    if (from_piece->type() == PieceType::Pawn && !is_capture) {
        const int drow = destination.row() - origin.row();
        const int dcol = destination.col() - origin.col();
        const int direction = (from_piece->color() == Color::White) ? 1 : -1;
        if (std::abs(dcol) == 1 && drow == direction && en_passant_available_ &&
            destination.row() == en_passant_target_.row() && destination.col() == en_passant_target_.col()) {
            Piece* passant_piece = piece_at(en_passant_pawn_square_);
            if (passant_piece != nullptr && passant_piece->type() == PieceType::Pawn &&
                passant_piece->color() != from_piece->color()) {
                is_capture = true;
                is_en_passant_capture = true;
                en_passant_capture_square = en_passant_pawn_square_;
            }
        }
    }

    // Reuse same validators as real move, but do not mutate final state.
    std::string error;
    if (!is_geometrically_legal_move(*from_piece, origin, destination, is_capture, error)) {
        return false;
    }
    if (would_leave_king_in_check(
            origin, destination, from_piece->color(), is_en_passant_capture, en_passant_capture_square)) {
        return false;
    }
    return true;
}

bool Board::can_castle_no_commit(Color turn, bool king_side) {
    const int row = (turn == Color::White) ? 0 : 7;
    const Square king_from(row, 4);
    const Square rook_from(row, king_side ? 7 : 0);
    const Square king_to(row, king_side ? 6 : 2);
    const Square rook_to(row, king_side ? 5 : 3);

    Piece* king = piece_at(king_from);
    Piece* rook = piece_at(rook_from);
    if (king == nullptr || rook == nullptr) {
        return false;
    }
    if (king->type() != PieceType::King || rook->type() != PieceType::Rook) {
        return false;
    }
    if (king->color() != turn || rook->color() != turn) {
        return false;
    }
    if (king->has_moved() || rook->has_moved()) {
        return false;
    }
    if (!is_path_clear(king_from, rook_from)) {
        return false;
    }
    if (is_in_check(turn)) {
        return false;
    }

    // Simulate castling and ensure final king square is safe.
    auto king_piece = std::move(board_[king_from.row()][king_from.col()]);
    auto rook_piece = std::move(board_[rook_from.row()][rook_from.col()]);
    board_[king_to.row()][king_to.col()] = std::move(king_piece);
    board_[rook_to.row()][rook_to.col()] = std::move(rook_piece);
    const bool legal = !is_in_check(turn);
    king_piece = std::move(board_[king_to.row()][king_to.col()]);
    rook_piece = std::move(board_[rook_to.row()][rook_to.col()]);
    board_[king_from.row()][king_from.col()] = std::move(king_piece);
    board_[rook_from.row()][rook_from.col()] = std::move(rook_piece);
    return legal;
}

bool Board::would_leave_king_in_check(
    const Square& origin,
    const Square& destination,
    Color moving_color,
    bool is_en_passant_capture,
    const Square& en_passant_capture_square) {
    // Full move simulation (including en passant capture), then rollback.
    auto moving_piece = std::move(board_[origin.row()][origin.col()]);
    auto captured_piece = std::move(board_[destination.row()][destination.col()]);
    std::unique_ptr<Piece> en_passant_captured_piece;
    if (is_en_passant_capture && en_passant_capture_square.is_valid()) {
        en_passant_captured_piece =
            std::move(board_[en_passant_capture_square.row()][en_passant_capture_square.col()]);
    }
    board_[destination.row()][destination.col()] = std::move(moving_piece);

    const bool still_in_check = is_in_check(moving_color);

    moving_piece = std::move(board_[destination.row()][destination.col()]);
    board_[origin.row()][origin.col()] = std::move(moving_piece);
    board_[destination.row()][destination.col()] = std::move(captured_piece);
    if (is_en_passant_capture && en_passant_capture_square.is_valid()) {
        board_[en_passant_capture_square.row()][en_passant_capture_square.col()] =
            std::move(en_passant_captured_piece);
    }

    return still_in_check;
}
