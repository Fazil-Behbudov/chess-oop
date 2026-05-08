#pragma once

#include <optional>
#include <string>

class Square {
public:
    Square();
    Square(int row, int col);

    static std::optional<Square> from_algebraic(const std::string& text);

    bool is_valid() const;
    int row() const;
    int col() const;
    std::string to_algebraic() const;

private:
    int row_;
    int col_;
};
