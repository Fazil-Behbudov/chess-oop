#pragma once

#include <string>

/// Piece color.
enum class Color {
    White = 0,
    Black = 1
};

/// Logical piece type used by move rules and serialization.
enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

/**
 * Lightweight piece model.
 * Stores color/type + "has moved" flag (castling and pawn first move rules).
 */
class Piece {
public:
    Piece(Color color, PieceType type);
    virtual ~Piece() = default;

    /// Piece side.
    Color color() const;
    /// Piece kind.
    PieceType type() const;
    /// Whether this piece has already moved at least once.
    bool has_moved() const;
    /// Mark piece as moved.
    void mark_moved();

    /// UTF-8 display symbol.
    std::string symbol() const;
    /// Canonical serialization token (e.g. "wK", "bP").
    std::string canonical_code() const;

private:
    Color color_;
    PieceType type_;
    bool has_moved_;
};
