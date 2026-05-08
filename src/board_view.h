#pragma once

#include <ostream>

#include "board.h"

/**
 * UI-only board renderer.
 * Keeps display formatting concerns outside of Board's game-rule logic.
 */
class BoardView {
public:
    static void render(const Board& board, std::ostream& os, bool white_bottom);
};
