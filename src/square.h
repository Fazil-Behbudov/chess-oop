#pragma once

#include <optional>
#include <string>

/**
 * Board coordinate helper.
 * Internal indexing is 0-based: row 0..7, col 0..7.
 */
class Square {
public:
    /// Invalid square by default.
    Square();
    /// Build from internal coordinates.
    Square(int row, int col);

    /// Parse algebraic form (e.g. "e4"). Returns nullopt on invalid input.
    static std::optional<Square> from_algebraic(const std::string& text);

    /// True when row/col are within [0,7].
    bool is_valid() const;
    /// 0-based row.
    int row() const;
    /// 0-based column.
    int col() const;
    /// Convert to algebraic notation (e.g. "e4"), or "??" if invalid.
    std::string to_algebraic() const;

private:
    int row_;
    int col_;
};
