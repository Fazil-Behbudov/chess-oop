#include "piece.h"

// New pieces start as "not moved" (important for castling/pawn rules).
Piece::Piece(Color color, PieceType type) : color_(color), type_(type), has_moved_(false) {}

Color Piece::color() const {
    return color_;
}

PieceType Piece::type() const {
    return type_;
}

bool Piece::has_moved() const {
    return has_moved_;
}

void Piece::mark_moved() {
    has_moved_ = true;
}

std::string Piece::symbol() const {
    // Unicode symbols used for board display.
    if (color_ == Color::White) {
        switch (type_) {
            case PieceType::King: return "♔";
            case PieceType::Queen: return "♕";
            case PieceType::Rook: return "♖";
            case PieceType::Bishop: return "♗";
            case PieceType::Knight: return "♘";
            case PieceType::Pawn: return "♙";
        }
    } else {
        switch (type_) {
            case PieceType::King: return "♚";
            case PieceType::Queen: return "♛";
            case PieceType::Rook: return "♜";
            case PieceType::Bishop: return "♝";
            case PieceType::Knight: return "♞";
            case PieceType::Pawn: return "♟";
        }
    }
    return "?";
}

std::string Piece::canonical_code() const {
    std::string code;
    // Canonical serialization uses color prefix + piece letter (e.g. wQ, bN).
    code += (color_ == Color::White ? 'w' : 'b');
    switch (type_) {
        case PieceType::King: code += 'K'; break;
        case PieceType::Queen: code += 'Q'; break;
        case PieceType::Rook: code += 'R'; break;
        case PieceType::Bishop: code += 'B'; break;
        case PieceType::Knight: code += 'N'; break;
        case PieceType::Pawn: code += 'P'; break;
    }
    return code;
}
