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

#include "misc.hpp"
#include "brdsize.hpp"
#include "chess.hpp"

extern void OutOfMemory(void);

// piece values.
const int VALUEKING = 0,
	  VALUEPAWN = 2,
	  VALUEROOK = 10,
	  VALUEKNIGHT = 6,
	  VALUEBISHOP = 6,
	  VALUEQUEEN = 18;

class PAWN : public PIECE
  {
  private:
    // pointer to piece to which pawn has been promoted.  null if
    // pawn has not been promoted.
    PIECE *promotePiece;

  public:
    PAWN(PIECECOLOR c) : PIECE(c, TYPEPAWN, VALUEPAWN), promotePiece(0) { }
    virtual ~PAWN(void) { if (promotePiece) delete promotePiece; }

    void promote(PIECETYPE promoteType);
    void restoreToPawn(void);

    virtual PIECETYPE whatType(void) const
      {
	if (promotePiece)
	  return(promotePiece->whatType());
	else
	  return(PIECE::whatType());
      }

    virtual int whatValue(void) const
      {
	if (promotePiece)
	  return(promotePiece->whatValue());
	else
	  return(PIECE::whatValue());
      }

    virtual void legalMoves
      (
	POSITION start,
	const BOARD &board,
        POSITIONLIST &moves
      ) const;

  };

class ROOK : public PIECE
  {
  public:
    ROOK(PIECECOLOR c) : PIECE(c, TYPEROOK, VALUEROOK)  { }

    virtual void legalMoves
      (
	POSITION start,
	const BOARD &board,
        POSITIONLIST &moves
      ) const;

  };

class KNIGHT : public PIECE
  {
  public:
    KNIGHT(PIECECOLOR c) : PIECE(c, TYPEKNIGHT, VALUEKNIGHT)  { }

    virtual void legalMoves
      (
	POSITION start,
	const BOARD &board,
        POSITIONLIST &moves
      ) const;

  };

class BISHOP : public PIECE
  {
  public:
    BISHOP(PIECECOLOR c) : PIECE(c, TYPEBISHOP, VALUEBISHOP)  { }

    virtual void legalMoves
      (
	POSITION start,
	const BOARD &board,
        POSITIONLIST &moves
      ) const;

  };

class QUEEN : public PIECE
  {
  public:
    QUEEN(PIECECOLOR c) : PIECE(c, TYPEQUEEN, VALUEQUEEN)  { }

    virtual void legalMoves
      (
	POSITION start,
	const BOARD &board,
        POSITIONLIST &moves
      ) const;

  };

class KING : public PIECE
  {
  public:
    KING(PIECECOLOR c) : PIECE(c, TYPEKING, VALUEKING)  { }

    virtual void legalMoves
      (
	POSITION start,
	const BOARD &board,
        POSITIONLIST &moves
      ) const;

  };

// put all the pieces for one color at their starting positions on
// the board
LOCAL void setupPieces
  (
    PIECE *board[][NUMCOLS],
    PIECECOLOR color,
    // column in which the non-pawn pieces go
    int backCol,
    // column in which the pawns go
    int pawnCol
  )
  {
    BOOL f = TRUE;
    int row;

    #define NEWPIECE(P, ROWOFFSET, COLOFFSET) \
      f = f && \
          ((board[ROWOFFSET][COLOFFSET] = new P) != (PIECE *) 0);

    NEWPIECE(ROOK(color), 0, backCol);
    NEWPIECE(KNIGHT(color), 1, backCol);
    NEWPIECE(BISHOP(color), 2, backCol);
    NEWPIECE(QUEEN(color), 3, backCol);
    NEWPIECE(KING(color), 4, backCol);
    NEWPIECE(BISHOP(color), 5, backCol);
    NEWPIECE(KNIGHT(color), 6, backCol);
    NEWPIECE(ROOK(color), 7, backCol);

    for (row = 0; row < NUMROWS; row++)
      NEWPIECE(PAWN(color), row, pawnCol);

    if (!f)
      OutOfMemory();

    return;

    #undef NEWPIECE
  }

BOARD::BOARD(void)
  {
    int row, col;

    for (row = 0; row < NUMROWS; row++)
      for (col = 0; col < NUMCOLS; col++)
	brd[row][col] = (PIECE *) 0;

    setupPieces(brd, WHITE, 0, 1);
    setupPieces(brd, BLACK, 7, 6);

    wasLastMoveDoublePawn = FALSE;

    return;
  }

BOARD::~BOARD(void)
  {
    int r, c;

    for (r = 0; r < NUMROWS; r++)
      for (c = 0; c < NUMROWS; c++)
	{
	  if (brd[r][c])
	    delete brd[r][c];
	}

    return;
  }

void BOARD::doMove
  (
    POSITION start,
    POSITION end,
    MOVEUNDODATA &undoData
  )
  {
    undoData.capturedPiece = brd[end.row][end.col];
    undoData.enPassantEffect = OTHERMOVE;

    brd[end.row][end.col] = brd[start.row][start.col];
  
    brd[start.row][start.col] = (PIECE *) 0;
    brd[end.row][end.col]->moveDone();

    if (brd[end.row][end.col]->whatType() == TYPEPAWN)
      {
	if (wasLastMoveDoublePawn)
	  if ((doubleMovedPawn.row == end.row) &&
	      (doubleMovedPawn.col == start.col) &&
	      (start.row != end.row) &&
	      !undoData.capturedPiece)
	      // these last two tests are necessary to handle situations
	      // where the same color is moved twice in a row when
	      // looking for check in "canCastle" and looking for
	      // stalemate
	    {
	      // en passant capture
	      undoData.capturedPiece =
		brd[doubleMovedPawn.row][doubleMovedPawn.col];
	      brd[doubleMovedPawn.row][doubleMovedPawn.col] =
		(PIECE *) 0;
	      undoData.saveDoubleMoved = doubleMovedPawn;
	      undoData.enPassantEffect = ENPASSANTCAPTURE;
	      wasLastMoveDoublePawn = FALSE;
	      return;
	    }
	{
	  int diff = start.col - end.col;
	  if (diff < 0)
	    diff = -diff;
	  if (diff == 2)
	    {
	      if (wasLastMoveDoublePawn)
		{
		  undoData.saveDoubleMoved = doubleMovedPawn;
		  undoData.enPassantEffect = AFTERDOUBLEMOVE;
		}
	      doubleMovedPawn = end;
	      wasLastMoveDoublePawn = TRUE;
	      return;
	    }
	}
      }

    if (wasLastMoveDoublePawn)
      {
	undoData.saveDoubleMoved = doubleMovedPawn;
	undoData.enPassantEffect = AFTERDOUBLEMOVE;
      }
    wasLastMoveDoublePawn = FALSE;

    return;
  }

void BOARD::undoMove
  (
    POSITION end,
    POSITION orig,
    MOVEUNDODATA undoData
  )
  {
    brd[orig.row][orig.col] = brd[end.row][end.col];

    brd[orig.row][orig.col]->moveUndone();

    if (undoData.enPassantEffect == ENPASSANTCAPTURE)
      {
	brd[end.row][orig.col] = undoData.capturedPiece;
	brd[end.row][end.col] = (PIECE *) 0;
      }
    else
      brd[end.row][end.col] = undoData.capturedPiece;

    if (undoData.enPassantEffect == OTHERMOVE)
      wasLastMoveDoublePawn = FALSE;
    else
      {
	wasLastMoveDoublePawn = TRUE;
	doubleMovedPawn = undoData.saveDoubleMoved;
      }

    return;
  }

BOOL BOARD::canCastle(MOVETYPE whichCastle, PIECECOLOR color)
  {
    int col = color == WHITE ? 0 : 7;
    int row, rowStep;
    BOARDMETRIC metric;

    if (whichCastle == QUEENSIDECASTLE)
      {
	row = 0;
	rowStep = 1;
      }
    else
      {
	row = 7;
	rowStep = -1;
      }

    // make sure king & rook in initial positions and have never been
    // moved.
    if (!brd[row][col])
      return(FALSE);
    if (brd[row][col]->hasBeenMoved())
      return(FALSE);
    if (!brd[4][col])
      return(FALSE);
    if (brd[4][col]->hasBeenMoved())
      return(FALSE);

    // make sure no pieces in between
    for ( ; ; )
      {
	row += rowStep;
	if (row == 4)
	  break;
	if (brd[row][col])
	  return(FALSE);
      }

    // make sure king is not in check
    findBestMoves(1, OtherColor(color), metric, (BESTMOVES *) 0);
    if (metric.kingSituation[color] == KINGLOST)
      return(FALSE);

    // make sure king would not be in check in intermediate position
    brd[4 - rowStep][col] = brd[4][col];
    brd[4][col] = (PIECE *) 0;
    findBestMoves(1, OtherColor(color), metric, (BESTMOVES *) 0);
    brd[4][col] = brd[4 - rowStep][col];
    brd[4 - rowStep][col] = (PIECE *) 0;

    return(metric.kingSituation[color] == KINGOK);
  }

void BOARD::castle
  (
    MOVETYPE whichCastle,
    PIECECOLOR color,
    MOVEUNDODATA &undoData
  )
  {
    int col = color == WHITE ? 0 : 7;

    if (whichCastle == QUEENSIDECASTLE)
      {
	brd[3][col] = brd[0][col];
	brd[2][col] = brd[4][col];
	brd[0][col] = (PIECE *) 0;
	brd[4][col] = (PIECE *) 0;
	brd[3][col]->moveDone();
	brd[2][col]->moveDone();
      }
    else
      {
	brd[5][col] = brd[7][col];
	brd[6][col] = brd[4][col];
	brd[7][col] = (PIECE *) 0;
	brd[4][col] = (PIECE *) 0;
	brd[5][col]->moveDone();
	brd[6][col]->moveDone();
      }

    if (wasLastMoveDoublePawn)
      {
	undoData.enPassantEffect = AFTERDOUBLEMOVE;
	undoData.saveDoubleMoved = doubleMovedPawn;
	wasLastMoveDoublePawn = FALSE;
      }
    else
      undoData.enPassantEffect = OTHERMOVE;


    return;
  }

void BOARD::undoCastle
  (
    MOVETYPE whichCastle,
    PIECECOLOR color,
    MOVEUNDODATA &undoData
  )
  {
    int col = color == WHITE ? 0 : 7;

    if (whichCastle == QUEENSIDECASTLE)
      {
	brd[0][col] = brd[3][col];
	brd[4][col] = brd[2][col];
	brd[3][col] = (PIECE *) 0;
	brd[2][col] = (PIECE *) 0;
	brd[0][col]->moveUndone();
	brd[4][col]->moveUndone();
      }
    else
      {
	brd[7][col] = brd[5][col];
	brd[4][col] = brd[6][col];
	brd[5][col] = (PIECE *) 0;
	brd[6][col] = (PIECE *) 0;
	brd[7][col]->moveUndone();
	brd[4][col]->moveUndone();
      }

    if (undoData.enPassantEffect == AFTERDOUBLEMOVE)
      {
	wasLastMoveDoublePawn = TRUE;
        doubleMovedPawn = undoData.saveDoubleMoved;
      }
    else
      wasLastMoveDoublePawn = FALSE;

    return;
  }

BOOL BOARD::canPromote(POSITION where)
  {
    PIECE *p = whatPiece(where);

    if (!p)
      return(FALSE);

    if (where.col != (p->whatColor() == WHITE ? (NUMCOLS - 1) : 0))
      return(FALSE);

    return(p->whatType() == TYPEPAWN);
  }

void BOARD::promote(POSITION where, PIECETYPE promoteType)
  {
    ((PAWN *) whatPiece(where))->promote(promoteType);
    return;
  }

void BOARD::restorePawn(POSITION where)
  {
    ((PAWN *) whatPiece(where))->restoreToPawn();
    return;
  }

MOVESTATUS BOARD::doUserMove
  (
    POSITION start,
    POSITION end
  )
  {
    PIECE *p = whatPiece(start.row, start.col);
    POSITIONLIST moves;
    BOARDMETRIC metric;
    BESTMOVES bestMoves;
    MOVEUNDODATA undoData;
    int m;

    if (!p)
      return(MOVESTATUS(ILLEGALMOVE));

    p->legalMoves(start, *this, moves);
    for (m = 0; m < moves.nMoves; m++)
      {
	if ((moves.end[m].row == end.row) &&
	    (moves.end[m].col == end.col))
	  break;
      }
    if (m == moves.nMoves)
      return(MOVESTATUS(ILLEGALMOVE));

    doMove(start, end, undoData);

    findBestMoves(1, OtherColor(p->whatColor()), metric, &bestMoves);

    if (metric.kingSituation[p->whatColor()] == KINGLOST)
      {
        undoMove(end, start, undoData);
	return(MOVESTATUS(WOULDLOSEKING, bestMoves.move[0].start));
      }

    if (undoData.capturedPiece)
      delete undoData.capturedPiece;
    if (undoData.enPassantEffect == ENPASSANTCAPTURE)
      return(MOVESTATUS(MOVEENPASSANT));
    else
      return(MOVESTATUS(MOVEDONE));
  }

BOOL BOARD::userCanCastle
  (
    MOVETYPE whichCastle,
    PIECECOLOR color
  )
  {
    BOARDMETRIC metric;
    MOVEUNDODATA undoData;

    if (!canCastle(whichCastle, color))
      return(FALSE);

    castle(whichCastle, color, undoData);

    // make sure king is not in check in final position
    findBestMoves(1, OtherColor(color), metric, (BESTMOVES *) 0);

    undoCastle(whichCastle, color, undoData);

    return(metric.kingSituation[color] == KINGOK);
  }


// tables that help in comparing the kingSituation portions of two
// board metrics

class SITUATIONCOMPARETABLE
  {
  private:
    int t[3][3];

  public:
    int * operator [] (int i) { return(t[i]); }
  };

class THISSITUATIONCOMPARETABLE : public SITUATIONCOMPARETABLE
  {
  public:
    THISSITUATIONCOMPARETABLE(void)
      {
	(*this)[KINGLOST][KINGLOST] = 0;
	(*this)[KINGLOST][STALEMATE] = -1;
	(*this)[KINGLOST][KINGOK] = -1;
	(*this)[STALEMATE][KINGLOST] = 1;
	(*this)[STALEMATE][STALEMATE] = 0;
	(*this)[STALEMATE][KINGOK] = -1;
	(*this)[KINGOK][KINGLOST] = 1;
	(*this)[KINGOK][STALEMATE] = 1;
	(*this)[KINGOK][KINGOK] = 0;
      }
  };

LOCAL THISSITUATIONCOMPARETABLE thisSituationCompareTable;

class OTHERSITUATIONCOMPARETABLE : public SITUATIONCOMPARETABLE
  {
  public:
    OTHERSITUATIONCOMPARETABLE(void)
      {
	(*this)[KINGLOST][KINGLOST] = 0;
	(*this)[KINGLOST][STALEMATE] = 1;
	(*this)[KINGLOST][KINGOK] = 1;
	(*this)[STALEMATE][KINGLOST] = -1;
	(*this)[STALEMATE][STALEMATE] = 0;
	(*this)[STALEMATE][KINGOK] = -1;
	(*this)[KINGOK][KINGLOST] = -1;
	(*this)[KINGOK][STALEMATE] = 1;
	(*this)[KINGOK][KINGOK] = 0;
      }
  };

LOCAL OTHERSITUATIONCOMPARETABLE otherSituationCompareTable;

// compare two board metrics.  if the second metric is invalid,
// the first metric is better.  the color parameter gives the player
// for whom the metric is supposed to be better.
LOCAL int compareMetric
  (
    const BOARDMETRIC &a,
    const BOARDMETRIC &b,
    BOOL bValid,
    PIECECOLOR color
  )
  {
    PIECECOLOR otherColor = OtherColor(color);
    int compare;

    if (!bValid)
      return(1);

    compare = thisSituationCompareTable[a.kingSituation[color]]
				       [b.kingSituation[color]];
    if (compare != 0)
      return(compare);

    compare = otherSituationCompareTable[a.kingSituation[otherColor]]
				        [b.kingSituation[otherColor]];
    if (compare != 0)
      return(compare);

    compare = a.materialDiff - b.materialDiff;

    if (color == BLACK)
      compare = -compare;

    return(compare);
  }

void BOARD::helpFindBestMoves
  (
    int lookAhead,
    PIECECOLOR moveColor,
    BOARDMETRIC &metric,
    BESTMOVES *bestMoves
  )
  {
    POSITION where;
    POSITIONLIST moves;
    BOARDMETRIC testMetric;
    BOOL metricSet = FALSE;
    MOVEUNDODATA undoData;
    int m, compareResult;
    int origMaterialDiff = metric.materialDiff;
    MOVETYPE castleType;

    testMetric.kingSituation[WHITE] = KINGOK;
    testMetric.kingSituation[BLACK] = KINGOK;

    // try all possible moves for the given color

    for (where.row = 0; where.row < NUMROWS; where.row++)
      for (where.col = 0; where.col < NUMCOLS; where.col++)
	{
	  if (whatPiece(where))
	    if (whatPiece(where)->whatColor() == moveColor)
	      {
		whatPiece(where)->legalMoves(where, *this, moves);
		for (m = 0; m < moves.nMoves; m++)
		  {
		    testMetric.materialDiff = origMaterialDiff;
		    doMove
		      (
			where,
			moves.end[m],
			undoData
		      );

		    if (undoData.capturedPiece)
		      {
			if (undoData.capturedPiece->whatType() == TYPEKING)
			  {
			    undoMove
			      (
				moves.end[m],
				where,
				undoData
			      );

			    metric.kingSituation[OtherColor(moveColor)] =
			      KINGLOST;

			    if (bestMoves)
			      {
				bestMoves->nMoves = 0;
				bestMoves->move[0] =
				  PIECEMOVE
				    (
				      NORMALMOVE,
				      where,
				      moves.end[m]
				    );
			      }

			    return;
			  }
			testMetric.materialDiff -=
			  undoData.capturedPiece->signedValue();
		      }

		    if (canPromote(moves.end[m]))
		      {
			testMetric.materialDiff -=
			  whatPiece(moves.end[m])->signedValue();
			promote(moves.end[m], TYPEQUEEN);
			testMetric.materialDiff +=
			  whatPiece(moves.end[m])->signedValue();
			if (lookAhead > 1)
			  helpFindBestMoves
			    (
			      lookAhead - 1,
			      OtherColor(moveColor),
			      testMetric,
			      (BESTMOVES *) 0
			    );
			compareResult = compareMetric
					  (
					    testMetric,
					    metric,
				            metricSet,
					    moveColor
					  );
		        if (compareResult >= 0)
			  {
			    if (compareResult > 0)
			      {
				metric = testMetric;
				metricSet = TRUE;
				if (bestMoves)
				  bestMoves->nMoves = 0;
			      }
			    if (bestMoves)
			      bestMoves->move[bestMoves->nMoves++] =
				PIECEMOVE
				  (
				    NORMALMOVE,
				    where,
				    moves.end[m],
				    TYPEQUEEN
				  );
			  }
			testMetric.materialDiff -=
			  whatPiece(moves.end[m])->signedValue();
			restorePawn(moves.end[m]);

			promote(moves.end[m], TYPEKNIGHT);
			testMetric.materialDiff +=
			  whatPiece(moves.end[m])->signedValue();
			if (lookAhead > 1)
			  helpFindBestMoves
			    (
			      lookAhead - 1,
			      OtherColor(moveColor),
			      testMetric,
			      (BESTMOVES *) 0
			    );
			compareResult = compareMetric
					  (
					    testMetric,
					    metric,
				            metricSet,
					    moveColor
					  );
		        if (compareResult >= 0)
			  {
			    if (compareResult > 0)
			      {
				metric = testMetric;
				metricSet = TRUE;
				if (bestMoves)
				  bestMoves->nMoves = 0;
			      }
			    if (bestMoves)
			      bestMoves->move[bestMoves->nMoves++] =
				PIECEMOVE
				  (
				    NORMALMOVE,
				    where,
				    moves.end[m],
				    TYPEKNIGHT
				  );
			  }
			restorePawn(moves.end[m]);
		      }
		    else // no promotion
		      {
			if (lookAhead > 1)
			  helpFindBestMoves
			    (
			      lookAhead - 1,
			      OtherColor(moveColor),
			      testMetric,
			      (BESTMOVES *) 0
			    );
			compareResult = compareMetric
					  (
					    testMetric,
					    metric,
				            metricSet,
					    moveColor
					   );
		        if (compareResult >= 0)
			  {
			    if (compareResult > 0)
			      {
				metric = testMetric;
				metricSet = TRUE;
				if (bestMoves)
				  bestMoves->nMoves = 0;
			      }
			    if (bestMoves)
			      bestMoves->move[bestMoves->nMoves++] =
				PIECEMOVE
				  (
				    NORMALMOVE,
				    where,
				    moves.end[m]
				  );
			  }
		      }

		    undoMove
		      (
			moves.end[m],
			where,
			undoData
		      );

		  } // end of for loop over each legal move for piece

	      }

	} // end of double for loop over each board position

    // only try castling moves if look ahead move than one, since
    // nothing can be captured by doing a castling move
    if (lookAhead > 1)
      {
	castleType = QUEENSIDECASTLE;
	for ( ; ; )
	  {
	    if (canCastle(castleType, moveColor))
	      {
		testMetric.materialDiff = origMaterialDiff;
		castle(castleType, moveColor, undoData);
		helpFindBestMoves
		  (
		    lookAhead - 1,
		    OtherColor(moveColor),
		    testMetric,
		    (BESTMOVES *) 0
		  );
		compareResult = compareMetric
				  (
				    testMetric,
				    metric,
				    metricSet,
				    moveColor
				  );
		if (compareResult >= 0)
		  {
		    if (compareResult > 0)
		      {
			metric = testMetric;
			metricSet = TRUE;
			if (bestMoves)
			  bestMoves->nMoves = 0;
		      }
		    if (bestMoves)
		      bestMoves->move[bestMoves->nMoves++] =
			PIECEMOVE(castleType);
		  }
		undoCastle(castleType, moveColor, undoData);
	      }
	    if (castleType == KINGSIDECASTLE)
	      break;
	    castleType = KINGSIDECASTLE;
	  }

        // see if the loss of the king is the result of a stalemate
	// instead of check mate
        if (metric.kingSituation[moveColor] == KINGLOST)
	  {
	    helpFindBestMoves
	      (
		1,
		OtherColor(moveColor),
		testMetric,
		(BESTMOVES *) 0
	      );
	    if (testMetric.kingSituation[moveColor] != KINGLOST)
	      // king will be lost on next move, but is not in check
	      metric.kingSituation[moveColor] = STALEMATE;
	  }
      }

    return;
  }

// list all legal horizontal moves in one direction from a given starting
// point
LOCAL void horzMoves
  (
    int row,
    int col,
    int delta,
    const BOARD &board,
    POSITIONLIST &moves
  )
  {
    int limit = delta == 1 ? NUMCOLS : -1;
    PIECECOLOR color = board.whatPiece(row, col)->whatColor();

    for ( ; ; )
      {
	col += delta;
	if (col == limit)
	  return;
	if (board.whatPiece(row, col))
	  if (board.whatPiece(row, col)->whatColor() == color)
	    return;
	moves.end[moves.nMoves].row = row;
	moves.end[moves.nMoves++].col = col;
	if (board.whatPiece(row, col))
	  return;
      }
  }

// list all legal vertical moves in one direction from a given starting
// point
LOCAL void vertMoves
  (
    int row,
    int col,
    int delta,
    const BOARD &board,
    POSITIONLIST &moves
  )
  {
    int limit = delta == 1 ? NUMROWS : -1;
    PIECECOLOR color = board.whatPiece(row, col)->whatColor();

    for ( ; ; )
      {
	row += delta;
	if (row == limit)
	  return;
	if (board.whatPiece(row, col))
	  if (board.whatPiece(row, col)->whatColor() == color)
	    return;
	moves.end[moves.nMoves].row = row;
	moves.end[moves.nMoves++].col = col;
	if (board.whatPiece(row, col))
	  return;
      }
  }

// list all legal rook moves from a given starting point
LOCAL void rookMoves
  (
    int row,
    int col,
    const BOARD &board,
    POSITIONLIST &moves
  )
  {
    horzMoves(row, col, 1, board, moves);
    horzMoves(row, col, -1, board, moves);
    vertMoves(row, col, 1, board, moves);
    vertMoves(row, col, -1, board, moves);

    return;
  }

// list all legal diagonal moves from a given starting point in a
// single direction
LOCAL void diagMoves
  (
    int row,
    int col,
    int rowDelta,
    int colDelta,
    const BOARD &board,
    POSITIONLIST &moves
  )
  {
    int rowLimit = rowDelta == 1 ? NUMROWS : -1;
    int colLimit = colDelta == 1 ? NUMCOLS : -1;
    PIECECOLOR color = board.whatPiece(row, col)->whatColor();

    for ( ; ; )
      {
	row += rowDelta;
	col += colDelta;
	if ((row == rowLimit) || (col == colLimit))
	  return;
	if (board.whatPiece(row, col))
	  if (board.whatPiece(row, col)->whatColor() == color)
	    return;
	moves.end[moves.nMoves].row = row;
	moves.end[moves.nMoves++].col = col;
	if (board.whatPiece(row, col))
	  return;
      }
  }

// list all legal bishop moves from a given starting point
LOCAL void bishopMoves
  (
    int row,
    int col,
    const BOARD &board,
    POSITIONLIST &moves
  )
  {
    diagMoves(row, col, 1, 1, board, moves);
    diagMoves(row, col, 1, -1, board, moves);
    diagMoves(row, col, -1, 1, board, moves);
    diagMoves(row, col, -1, -1, board, moves);

    return;
  }

// check if a position is within the board
LOCAL BOOL withinBoard
  (
    int row,
    int col
  )
  {
    return ((0 <= row) && (row < NUMROWS) &&
            (0 <= col) && (col < NUMCOLS));
  }

// row and colomn offsets from a position
class POSITIONOFFSET
  {
  public:
    int row, col;
  };

// list of moves from a given starting position generated by applying
// a table of position offsets
LOCAL void movesFromOffsets
  (
    int row,
    int col,
    int nOffsets,
    const POSITIONOFFSET *offset,
    const BOARD &board,
    POSITIONLIST &moves
  )
  {
    PIECECOLOR color = board.whatPiece(row, col)->whatColor();
    int i;
    PIECE *p;

    for (i = 0; i < nOffsets; i++)
      {
	moves.end[moves.nMoves].row = row + offset[i].row;
	moves.end[moves.nMoves].col = col + offset[i].col;
	if (withinBoard(moves.end[moves.nMoves].row,
		        moves.end[moves.nMoves].col))
	  {
	    p = board.whatPiece
		  (
		    moves.end[moves.nMoves].row = row + offset[i].row,
		    moves.end[moves.nMoves].col = col + offset[i].col
		  );

	    if (!p)
	      moves.nMoves++;
	    else if (p->whatColor() != color)
	      moves.nMoves++;
	  }
      }

    return;
  }

const POSITIONOFFSET kingOffset[] =
  { { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, -1}, { 0, 1 },
    { 1, -1 }, { 1, 0 }, { 1, 1 } };
const int nKingOffsets = ARRAY_LENGTH(kingOffset);

const POSITIONOFFSET knightOffset[] =
  { { 1, 2 }, { 2, 1}, { -1, 2 }, { 2, -1 }, { 1, -2}, { -2, 1 },
    { -1, -2 }, { -2, -1 } };
const int nKnightOffsets = ARRAY_LENGTH(knightOffset);

void PAWN::promote(PIECETYPE promoteType)
  {
    switch (promoteType)
      {
      case TYPEQUEEN:
	promotePiece = new QUEEN(whatColor());
	break;

      case TYPEROOK:
	promotePiece = new ROOK(whatColor());
	break;

      case TYPEBISHOP:
	promotePiece = new BISHOP(whatColor());
	break;

      case TYPEKNIGHT:
	promotePiece = new KNIGHT(whatColor());
	break;
      }

    if (!promotePiece)
      OutOfMemory();

    return;
  }

void PAWN::restoreToPawn(void)
  {
    delete promotePiece;
    promotePiece = (PIECE *) 0;

    return;
  }

void PAWN::legalMoves
  (
    POSITION start,
    const BOARD &board,
    POSITIONLIST &moves
  ) const
  {
    int delta, limit;
    POSITION doubleMovedPawn;

    if (promotePiece)
      {
	promotePiece->legalMoves(start, board, moves);
	return;
      }
    
    if (whatColor() == WHITE)
      {
	delta = 1;
	limit = NUMCOLS;
      }
    else
      {
	delta = -1;
	limit = -1;
      }

    moves.nMoves = 0;

    // check if en passant capture possible
    if (board.lastMoveDoublePawn(doubleMovedPawn))
      if ((start.col == doubleMovedPawn.col) &&
	  (whatColor() != board.whatPiece(doubleMovedPawn)->whatColor()))
	{
	  if ((start.row - 1) == doubleMovedPawn.row)
	    {
	      moves.end[moves.nMoves].row = start.row - 1;
	      moves.end[moves.nMoves++].col = start.col + delta;
	    }
	  else if ((start.row + 1) == doubleMovedPawn.row)
	    {
	      moves.end[moves.nMoves].row = start.row + 1;
	      moves.end[moves.nMoves++].col = start.col + delta;
	    }
	}

    start.col += delta;

    if (start.col == limit)
      return;

    // check for moves ahead
    if (!board.whatPiece(start))
      {
	moves.end[moves.nMoves++] = start;
	// check if initial double move possible
	if (!hasBeenMoved())
	  if (!board.whatPiece(start.row, start.col + delta))
	    {
	      moves.end[moves.nMoves].row = start.row;
	      moves.end[moves.nMoves++].col = start.col + delta;
	    }
      }

    // check for captures

    if (start.row > 0)
      if (board.whatPiece(start.row - 1, start.col))
	if (board.whatPiece(start.row - 1, start.col)->whatColor() !=
	    whatColor())
	  {
	    moves.end[moves.nMoves].row = start.row - 1;
	    moves.end[moves.nMoves++].col = start.col;
	  }

    if (start.row < (NUMROWS - 1))
      if (board.whatPiece(start.row + 1, start.col))
	if (board.whatPiece(start.row + 1, start.col)->whatColor() !=
	    whatColor())
	  {
	    moves.end[moves.nMoves].row = start.row + 1;
	    moves.end[moves.nMoves++].col = start.col;
	  }

    return;
  }

void ROOK::legalMoves
  (
    POSITION start,
    const BOARD &board,
    POSITIONLIST &moves
  ) const
  {
    moves.nMoves = 0;
    rookMoves(start.row, start.col, board, moves);

    return;
  }

void KNIGHT::legalMoves
  (
    POSITION start,
    const BOARD &board,
    POSITIONLIST &moves
  ) const
  {
    moves.nMoves = 0;
    movesFromOffsets(start.row, start.col, nKnightOffsets, knightOffset,
                     board, moves);

    return;
  }

void BISHOP::legalMoves
  (
    POSITION start,
    const BOARD &board,
    POSITIONLIST &moves
  ) const
  {
    moves.nMoves = 0;
    bishopMoves(start.row, start.col, board, moves);

    return;
  }

void QUEEN::legalMoves
  (
    POSITION start,
    const BOARD &board,
    POSITIONLIST &moves
  ) const
  {
    moves.nMoves = 0;
    rookMoves(start.row, start.col, board, moves);
    bishopMoves(start.row, start.col, board, moves);

    return;
  }

void KING::legalMoves
  (
    POSITION start,
    const BOARD &board,
    POSITIONLIST &moves
  ) const
  {
    moves.nMoves = 0;
    movesFromOffsets(start.row, start.col, nKingOffsets, kingOffset,
                     board, moves);

    return;
  }
