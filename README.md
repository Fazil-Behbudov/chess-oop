# Terminal Chess (C++)

This project implements a text-mode chess game in C++ with move input from standard input (e.g. `e2e4`, `O-O`, `O-O-O`) and final canonical-output formatting compatible with the provided test harness.

## Implemented functionalities

### Core gameplay
- Board initialization with all standard pieces.
- Move input in coordinate notation: `a1a8` style.
- Commands: `/quit`, `/draw`, `/resign`.
- Turn alternation (white/black).
- Capture handling and own-piece destination rejection.
- Illegal-move rejection without changing board state.

### Piece movement and rules
- Geometric legality for all pieces.
- Obstacle checks for rook/bishop/queen.
- Pawn rules:
  - one-step forward,
  - two-step forward from initial rank,
  - diagonal capture,
  - en passant.
- Castling:
  - kingside and queenside (`O-O`, `O-O-O`),
  - move-history constraints (king/rook must not have moved),
  - path-clear checks.
- Promotion:
  - triggered on last rank,
  - interactive choice `{Q,R,B,N}`.

### Check/game-end logic
- Check detection.
- Self-check prevention (moves that leave own king in check are refused).
- Checkmate detection.
- Stalemate detection.
- Automatic result setting:
  - `1-0`, `0-1`, `1/2-1/2`, `?-?`.

### Output/test compatibility
- Last output line is always:
  - `<canonical_position> <result>`
- Canonical position format matches assignment requirement (64 comma-separated squares, trailing comma).
- Compatible with `test/test-level.sh`.

### UI behavior
- Interactive mode: single-board redraw (screen clear + redraw) instead of stacking boards.
- Board view rotates after each legal move.
- Non-interactive mode (pipes/files): stable output for automated tests.

---

## Code structure and separation of responsibilities

- `Square` (`square.h/.cpp`): coordinate parsing/conversion (`e4` <-> indices).
- `Piece` (`piece.h/.cpp`): piece model (type/color/moved-state), symbols, canonical serialization.
- `Board` (`board.h/.cpp`): board state + chess rule engine (move validation, check logic, castling/en passant/promotion helpers).
- `BoardView` (`board_view.h/.cpp`): rendering only (keeps UI formatting out of rule engine).
- `Game` (`game.h/.cpp`): command loop, interaction flow, result lifecycle.
- `main.cpp`: minimal entry point.

This split was chosen to keep logic reusable and testable: game rules are independent from terminal rendering.

---

## Main technical choices (and why)

1. **`std::unique_ptr<Piece>` board storage**
   - Safe ownership of pieces without manual memory management.
   - Allows move simulation by transferring ownership during hypothetical checks.

2. **Rule validation in `Board`**
   - Centralized legality rules prevent UI-layer duplication.
   - Supports both real move application and no-commit simulations.

3. **Simulation/rollback for king safety**
   - Used for self-check prevention and legal-move search.
   - Ensures correctness for complex cases like en passant and castling.

4. **Dedicated `BoardView` module**
   - Enforces modularity: rendering changes do not affect rule engine.
   - Makes orientation/rotation behavior isolated and maintainable.

---

## Challenges faced

1. **Correct self-check handling**
   - Challenge: validating a move requires understanding resulting board threats.
   - Solution: simulate move, evaluate check status, rollback safely.

2. **En passant correctness**
   - Challenge: rule is time-sensitive and stateful (valid only immediately after specific pawn move).
   - Solution: explicit en-passant state (`available`, `target`, `capturable pawn square`) reset/update on each move.

3. **Checkmate/stalemate detection**
   - Challenge: must prove absence of *any* legal move for side to play.
   - Solution: exhaustive side-move scan with no-commit legality checks.
---

## Build instructions

From repository root:

```bash
make
```

This builds the executable:

```bash
./src/echecs
```

Clean build artifacts:

```bash
make clean
```

---

## Run instructions

### Interactive

```bash
./src/echecs
```

Then enter moves/commands, for example:
- `e2e4`
- `O-O`
- `/draw`
- `/resign`
- `/quit`

### From a game file / stdin pipe

```bash
grep -v '^#' test/data/1-leg-knight-1.txt | ./src/echecs
```

---

## Run the provided tests

```bash
cd test
bash test-level.sh 1 ../src/echecs
bash test-level.sh 2 ../src/echecs
bash test-level.sh 3 ../src/echecs
bash test-level.sh 4 ../src/echecs
```

These scripts compare the last output line against reference canonical position + result and check exit code.
