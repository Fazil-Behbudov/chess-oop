#pragma once

#include <string>

enum class Color {
    White = 0,
    Black = 1
};

enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

class Piece {
public:
    Piece(Color color, PieceType type);
    virtual ~Piece() = default;

    Color color() const;
    PieceType type() const;
    bool has_moved() const;
    void mark_moved();

    std::string symbol() const;
    std::string canonical_code() const;

private:
    Color color_;
    PieceType type_;
    bool has_moved_;
};
