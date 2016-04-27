/*
Copyright (c) 2016 Walter William Karas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#if !defined(CHESS_HPP)
#define CHESS_HPP

#include "misc.hpp"
#include "brdsize.hpp"

enum PIECECOLOR { WHITE, BLACK };

inline PIECECOLOR OtherColor(PIECECOLOR c)
  { return(c == WHITE ? BLACK : WHITE); }

enum PIECETYPE { TYPEKING, TYPEQUEEN, TYPEBISHOP, TYPEKNIGHT,
                 TYPEROOK, TYPEPAWN, TYPENOPIECE };

// simple completion status of a move
enum MOVEEVAL
  {
    // move done (not an en passant capture)
    MOVEDONE,
    // en passant capture move performed
    MOVEENPASSANT,
    // move not performed because it is illegal
    ILLEGALMOVE,
    // move not performed because if would allow opponent to take
    // king on next move
    WOULDLOSEKING
  };

// full completion status of move
class MOVESTATUS
  {
  public:
    // simple completions status
    MOVEEVAL status;
    // if status is WOULDLOSEKING, dangerToKing specifies the position
    // of the opponent piece that would take the king on the nex move
    POSITION dangerToKing;

    MOVESTATUS(void) { };

    // constructor if status is not WOULDLOSEKING
    MOVESTATUS(MOVEEVAL s) : status(s) { };
    // constructor if status is WOULDLOSEKING
    MOVESTATUS(MOVEEVAL s, POSITION p) : status(s), dangerToKing(p) { };
  };

// specifies a move
enum MOVETYPE { KINGSIDECASTLE, QUEENSIDECASTLE, NORMALMOVE };
class PIECEMOVE
  {
  public:
    MOVETYPE type;
    POSITION start, end;
    // if moved piece is pawn which has thus arrived last rank, the
    // type of the piece to promote it to.  set to TYPENOPIECE if no
    // promotion.
    PIECETYPE promoteType;

    PIECEMOVE(void) { }

    PIECEMOVE
      (
        MOVETYPE t,
        POSITION s,
        POSITION e,
        PIECETYPE pT = TYPENOPIECE
      ) : type(t), start(s), end(e), promoteType(pT) { }

    // constructor for castling moves
    PIECEMOVE
      (
        MOVETYPE t
      ) : type(t) { }
  };

class PIECE;

// records whether the current move is an en passant capture, or
// follows a two-rank move of a pawn
enum EFFECTENPASSANT { ENPASSANTCAPTURE, AFTERDOUBLEMOVE, OTHERMOVE };
// information (other than the starting and end piece position)
// required to undo the effects of moving a piece
class MOVEUNDODATA
  {
  public:
    // if not null, piece captured by moving piece
    PIECE *capturedPiece;
    // en passant effect
    EFFECTENPASSANT enPassantEffect;
    // if enPassantEffect not OTHERMOVE, records position of
    // previously double-moved pawn
    POSITION saveDoubleMoved;
  };

// evalution of change in relative situation of the two players after
// one or more moves
enum SITUATIONOFKING { KINGLOST, STALEMATE, KINGOK };
class BOARDMETRIC
  {
  public:
    SITUATIONOFKING kingSituation[2]; // indexed by PIECECOLOR
    // change in relative material (total white material -
    // total black material)
    int materialDiff;
  };

// maximum number of pieces of one color on the board
const int MAXPIECES = 16;

// maximum number of locations that a piece can reach in
// a single move
const int MAXMOVES = 28;

// list of moves which result in the greatest gain (or least loss)
// of material.
class BESTMOVES
  {
  public:
    // number of moves in list
    int nMoves;
    PIECEMOVE move[MAXPIECES * MAXMOVES];
  };

// internal representation of chess board
class BOARD
  {
  private:
    // location in brd array points to piece in corresponding position
    // on chess board.  a null pointer means no piece in that location.
    PIECE *brd[NUMROWS][NUMCOLS];
    // flags if the last move was a double pawn move
    BOOL wasLastMoveDoublePawn;
    // if last move was double pawn move, contains the ending position
    // of the pawn.
    POSITION doubleMovedPawn;

    // recursive function to find optimal moves in terms of
    // getting opponent in checkmate or material gain.
    void helpFindBestMoves
      (
        // number of moves to look-ahead
        int lookAhead,
        // color of player that is going to move
        PIECECOLOR moveColor,
        // metric of optimal moves
        BOARDMETRIC &metric,
        // if not null, filled in with list of optimal moves
        BESTMOVES *bestMoves
      );

  public:
    BOARD(void);
    ~BOARD(void);

    PIECE *whatPiece(POSITION p) const
      { return(brd[p.row][p.col]); }

    PIECE *whatPiece(int row, int col) const
      { return(brd[row][col]); }

    // perform a move.  move is not validated (assumed to be legal).
    void doMove
      (
        // starting and ending position of piece to move
        POSITION start,
        POSITION end,
        // data needed to undo move.  caller must delete captured
        // piece (unless move is undone)
        MOVEUNDODATA &undoData
      );

    // undo a move done with doMove
    void undoMove
      (
        // ending and original position of piece (transposed of order
        // as passed to doMove)
        POSITION end,
        POSITION orig,
        // undo data returned by call to doMove
        MOVEUNDODATA undoData
      );

    // returns TRUE if the king of the given color can castle on the
    // given side (as given by whichCastle).  the one preventing
    // condition not cheched for is if the final position of the king
    // places it in check.
    BOOL canCastle(MOVETYPE whichCastle, PIECECOLOR color);

    // do a castle on the given side with the king of given color.
    // no validation, assumed to be legal.
    void castle(MOVETYPE whichCastle, PIECECOLOR color,
                MOVEUNDODATA &undoData);
    // undo a castle.  undoData must be as returned by castle member.
    void undoCastle(MOVETYPE whichCastle, PIECECOLOR color,
                    MOVEUNDODATA &undoData);

    // checks if the piece in the given position can be promoted.
    BOOL canPromote(POSITION where);
    // promotes pawn in given location to piece of given type.
    // assumed to be legal, no validation.
    void promote(POSITION where, PIECETYPE promoteType);
    // "un-promotes" a piece back to being a pawn.  won't work if
    // the piece was not originally a pawn.
    void restorePawn(POSITION where);

    // do a move with full validation, returning status
    MOVESTATUS doUserMove
      (
        // starting and ending position of piece.  the starting
        // position is assumed to be valid.
        POSITION start,
        POSITION end
      );

    // complete check of whether a castle move can be done
    BOOL userCanCastle(MOVETYPE whichCastle, PIECECOLOR color);

    // returns TRUE if the last move was a double pawn move.  if
    // TRUE, the ending position of the pawn is returned as well.
    BOOL lastMoveDoublePawn(POSITION &whereDoubleMovedPawn) const
      { 
        if (wasLastMoveDoublePawn)
          whereDoubleMovedPawn = doubleMovedPawn;
        return(wasLastMoveDoublePawn);
      }

    // front end for helpFindBestMoves.  simply initializes the
    // material change to 0.
    void findBestMoves
      (
        int lookAhead,
        PIECECOLOR moveColor,
        BOARDMETRIC &metric,
        BESTMOVES *bestMoves
      )
      {
        metric.materialDiff = 0;

        helpFindBestMoves(lookAhead, moveColor, metric, bestMoves);

        return;
      }

  };

// list of possible ending positions if a piece is moved from a fixed
// starting positions
class POSITIONLIST
  {
  public:
    // number of ending positions
    int nMoves;
    POSITION end[MAXMOVES];
  };

// abstract base class for piece.
class PIECE
  {
  private:
    const PIECECOLOR color;
    const PIECETYPE type;
    // material value of piece.
    const int value;
    // number of moves piece has made.
    int moveCount;

  public:

    PIECE(PIECECOLOR c, PIECETYPE t, int v) :
      color(c), type(t), value(v), moveCount(0)
      { }
    virtual ~PIECE(void) { }

    PIECECOLOR whatColor(void) const { return(color); }

    virtual PIECETYPE whatType(void) const { return(type); }

    virtual int whatValue(void) const { return(value); }

    // material value of piece where white is positive and
    // black is negative
    int signedValue(void) const
      { return(color == WHITE ? whatValue() : -whatValue()); }

    // tell piece it was moved (once)
    void moveDone(void) { moveCount++; }
    // tell piece it was "unmoved"
    void moveUndone(void) { moveCount--; }
    // ask piece if it has been moved at least once
    BOOL hasBeenMoved(void) const { return(moveCount > 0); }

    // return list of legal moves for the piece
    virtual void legalMoves
      (
        // tell piece where it is in board
        POSITION start,
        // give it the board, so it knows where other pieces are
        const BOARD &board,
        // struture to fill with ending positions of legal moves
        // from the given start
        POSITIONLIST &moves
      ) const = 0;

  };

#endif
