#pragma once

#include <array>
#include <memory>
#include <ostream>
#include <string>

#include "piece.h"
#include "square.h"

class Board {
public:
    Board();

    void reset_initial_position();
    void display(std::ostream& os) const;
    std::string canonical_position() const;

    const Piece* piece_at(const Square& square) const;
    Piece* piece_at(const Square& square);
    bool is_empty(const Square& square) const;
    bool is_in_check(Color color) const;
    bool needs_promotion(const Square& square) const;
    bool promote_pawn(const Square& square, PieceType new_type, std::string& error);
    bool has_any_legal_move(Color color);

    bool move_piece_basic(const Square& origin, const Square& destination, Color turn, std::string& error);
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
    Square find_king(Color color) const;
    bool is_square_attacked(const Square& square, Color by_color) const;
    bool piece_attacks_square(const Piece& piece, const Square& origin, const Square& target) const;
    bool is_legal_move_no_commit(const Square& origin, const Square& destination, Color turn);
    bool can_castle_no_commit(Color turn, bool king_side);
    bool would_leave_king_in_check(
        const Square& origin,
        const Square& destination,
        Color moving_color,
        bool is_en_passant_capture = false,
        const Square& en_passant_capture_square = Square());

    bool en_passant_available_;
    Square en_passant_target_;
    Square en_passant_pawn_square_;
};
