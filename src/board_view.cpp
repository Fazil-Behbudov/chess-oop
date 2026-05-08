#include "board_view.h"

void BoardView::render(const Board& board, std::ostream& os, bool white_bottom) {
    os << "\n";
    // File labels are mirrored when the board is shown from black perspective.
    os << "     ";
    for (int visual_col = 0; visual_col < 8; ++visual_col) {
        const int col = white_bottom ? visual_col : (7 - visual_col);
        os << static_cast<char>('a' + col) << "     ";
    }
    os << "\n";
    os << "  +-----+-----+-----+-----+-----+-----+-----+-----+\n";
    for (int visual_row = 0; visual_row < 8; ++visual_row) {
        // Map visual row/col to real board indices depending on orientation.
        const int row = white_bottom ? (7 - visual_row) : visual_row;
        os << row + 1 << " ";
        for (int visual_col = 0; visual_col < 8; ++visual_col) {
            const int col = white_bottom ? visual_col : (7 - visual_col);
            os << "|";
            const Piece* piece = board.piece_at(Square(row, col));
            if (piece != nullptr) {
                os << "  " << piece->symbol() << "  ";
            } else {
                os << "     ";
            }
        }
        os << "|\n";
        os << "  +-----+-----+-----+-----+-----+-----+-----+-----+\n";
    }
}
