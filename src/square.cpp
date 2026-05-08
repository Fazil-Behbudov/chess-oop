#include "square.h"

Square::Square() : row_(-1), col_(-1) {}

Square::Square(int row, int col) : row_(row), col_(col) {}

std::optional<Square> Square::from_algebraic(const std::string& text) {
    if (text.size() != 2) {
        return std::nullopt;
    }
    const char file = text[0];
    const char rank = text[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
        return std::nullopt;
    }
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
    out.push_back(static_cast<char>('a' + col_));
    out.push_back(static_cast<char>('1' + row_));
    return out;
}
