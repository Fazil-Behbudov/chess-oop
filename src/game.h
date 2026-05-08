#pragma once

#include <string>

#include "board.h"

/**
 * High-level game loop and user interaction layer.
 * Parses commands, delegates rules to Board, and manages game result.
 */
class Game {
public:
    Game();
    /// Run interactive/stream game until end condition.
    int run();

private:
    Board board_;
    Color turn_;
    std::string result_;
    bool finished_;
    bool interactive_ui_;
    bool white_bottom_view_;

    /// Parse and route one user command.
    bool handle_command(const std::string& command);
    /// Handle standard coordinate move.
    bool handle_move(const std::string& command);
    /// Handle O-O / O-O-O.
    bool handle_castling(const std::string& command);
    /// Ask and apply pawn promotion choice.
    bool handle_promotion(const Square& destination);
    /// Evaluate check/checkmate/stalemate after a successful move.
    void finalize_turn_after_move();
    /// Draw board (single-board mode in interactive UI).
    void render_board() const;
    /// Toggle side to move.
    void switch_turn();
};
