#include "square.h"

// Default-constructed square is intentionally invalid.
Square::Square() : row_(-1), col_(-1) {}

// Internal board coordinates constructor (0-based row/col).
Square::Square(int row, int col) : row_(row), col_(col) {}

std::optional<Square> Square::from_algebraic(const std::string& text) {
    // Expected format is exactly "<file><rank>", e.g. "e4".
    if (text.size() != 2) {
        return std::nullopt;
    }
    const char file = text[0];
    const char rank = text[1];
    // Valid chessboard bounds: files a..h and ranks 1..8.
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
        return std::nullopt;
    }
    // Convert algebraic coordinates to 0-based indices.
    return Square(rank - '1', file - 'a');
}

bool Square::is_valid() const {
    return row_ >= 0 && row_ < 8 && col_ >= 0 && col_ < 8;
}

int Square::row() const {
    return row_;
}

int Square::col() const {
    return col_;
}

std::string Square::to_algebraic() const {
    if (!is_valid()) {
        return "??";
    }
    std::string out;
    // Convert 0-based indices back to algebraic notation.
    out.push_back(static_cast<char>('a' + col_));
    out.push_back(static_cast<char>('1' + row_));
    return out;
}
