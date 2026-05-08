#pragma once

#include <string>

#include "board.h"

class Game {
public:
    Game();
    int run();

private:
    Board board_;
    Color turn_;
    std::string result_;
    bool finished_;
    bool interactive_ui_;

    bool handle_command(const std::string& command);
    bool handle_move(const std::string& command);
    bool handle_castling(const std::string& command);
    bool handle_promotion(const Square& destination);
    void finalize_turn_after_move();
    void render_board() const;
    void switch_turn();
};
