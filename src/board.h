#pragma once

#include <array>
#include <memory>
#include <string>

#include "piece.h"
#include "square.h"

/**
 * Chessboard state and rule engine.
 * Owns all pieces and validates/applies moves.
 */
class Board {
public:
    Board();

    /// Restore standard initial position.
    void reset_initial_position();
    /// 64-square canonical string used by tests.
    std::string canonical_position() const;

    /// Read-only piece lookup.
    const Piece* piece_at(const Square& square) const;
    /// Mutable piece lookup.
    Piece* piece_at(const Square& square);
    /// True if no piece is on the square.
    bool is_empty(const Square& square) const;
    /// True if the given side king is currently attacked.
    bool is_in_check(Color color) const;
    /// True if the pawn on square must be promoted now.
    bool needs_promotion(const Square& square) const;
    /// Replace promotable pawn with chosen piece type.
    bool promote_pawn(const Square& square, PieceType new_type, std::string& error);
    /// True if side has at least one legal move.
    bool has_any_legal_move(Color color);

    /// Apply a normal move if legal.
    bool move_piece_basic(const Square& origin, const Square& destination, Color turn, std::string& error);
    /// Apply castling if legal (king_side=true for O-O, false for O-O-O).
    bool castle(Color turn, bool king_side, std::string& error);

private:
    using Row = std::array<std::unique_ptr<Piece>, 8>;
    std::array<Row, 8> board_;

    void place_piece(std::unique_ptr<Piece> piece, const Square& square);
    bool is_path_clear(const Square& origin, const Square& destination) const;
    bool is_geometrically_legal_move(
        const Piece& piece,
        const Square& origin,
        const Square& destination,
        bool is_capture,
        std::string& error) const;
    /// Locate king square for a side.
    Square find_king(Color color) const;
    /// True if any piece of by_color attacks square.
    bool is_square_attacked(const Square& square, Color by_color) const;
    /// Attack predicate used by check detection.
    bool piece_attacks_square(const Piece& piece, const Square& origin, const Square& target) const;
    /// Move legality test without committing board changes.
    bool is_legal_move_no_commit(const Square& origin, const Square& destination, Color turn);
    /// Castling legality test without committing board changes.
    bool can_castle_no_commit(Color turn, bool king_side);
    /// Simulate move and verify king safety, then rollback.
    bool would_leave_king_in_check(
        const Square& origin,
        const Square& destination,
        Color moving_color,
        bool is_en_passant_capture = false,
        const Square& en_passant_capture_square = Square());

    // En passant state:
    // - available: whether an en passant capture is currently legal
    // - target: empty square that the capturing pawn moves to
    // - pawn_square: square occupied by the pawn that can be captured
    bool en_passant_available_;
    Square en_passant_target_;
    Square en_passant_pawn_square_;
};
