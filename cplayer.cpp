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

#include <limits.h>

#include "brdsize.hpp"
#include "chess.hpp"
#include "cplayer.hpp"
#include "chessui.hpp"

// returns a measurement of how much of the board is "covered" by
// the pieces of a given player.  lots of extra points are given
// for "covering" the location around the opponent's king
LOCAL int coverage
  (
    const BOARD &board,
    PIECECOLOR color,
    POSITION whereEnemyKing
  )
  {
    unsigned char map[NUMROWS][NUMCOLS];
    // bit flags for elements of map
    const unsigned char KINGAREA = 1;
    const unsigned char COVERED = 2;
    POSITION where;
    POSITIONLIST moves;
    int m, result = 0;

    // initialize map
    for (where.row = 0; where.row < NUMROWS; where.row++)
      for (where.col = 0; where.col < NUMCOLS; where.col++)
        map[where.row][where.col] = 0;

    // set covered flag on all covered locations
    for (where.row = 0; where.row < NUMROWS; where.row++)
      for (where.col = 0; where.col < NUMCOLS; where.col++)
	{
	  if (board.whatPiece(where))
	    if ((board.whatPiece(where)->whatColor() == color) &&
	        (board.whatPiece(where)->whatType() != TYPEPAWN))
	      {
		board.whatPiece(where)->legalMoves(where, board, moves);
		for (m = 0; m < moves.nMoves; m++)
		  map[moves.end[m].row][moves.end[m].col] = COVERED;
	      }
	}

    // flag locations to which enemy king could move
    board.whatPiece(whereEnemyKing)->
      legalMoves(whereEnemyKing, board, moves);
    for (m = 0; m < moves.nMoves; m++)
      map[moves.end[m].row][moves.end[m].col] |= KINGAREA;

    // accumulate coverage points
    for (where.row = 0; where.row < NUMROWS; where.row++)
      for (where.col = 0; where.col < NUMCOLS; where.col++)
	{
          if (map[where.row][where.col] & COVERED)
	    {
	      if (map[where.row][where.col] & KINGAREA)
		result += 20;
	      else
		result += 1;
	    }
	}

    return(result);
  }

LOCAL POSITION whereKing
  (
    const BOARD &board,
    PIECECOLOR color
  )
  {
    POSITION where;

    for (where.row = 0; ; where.row++)
      for (where.col = 0; where.col < NUMCOLS; where.col++)
	{
	  if (board.whatPiece(where))
	    if (board.whatPiece(where)->whatColor() == color)
	      if (board.whatPiece(where)->whatType() == TYPEKING)
		return(where);
	}
  }

// positive difference between two integers
LOCAL inline int absDiff(int a, int b)
  {
    a -= b;
    return(a < 0 ? -a : a);
  }

// the change in difference from a goal value between a starting
// and an ending value
LOCAL int oneDimHowClose(int start, int end, int goal)
  {
    int a = absDiff(start, goal);
    int diff = a - absDiff(end, goal);

    if ((a < 2) && (diff > 0))
      return(0);

    return(diff);
  }

// the change in "threat" metric caused by a given piece moving from
// a starting position to an ending position in terms of trying to
// threaten a goal position
LOCAL int threatChange
  (
    POSITION start,
    POSITION end,
    POSITION goal,
    PIECE *p
  )
  {
    if (p->whatType() == TYPEPAWN)
      return(absDiff(start.col, end.col) * 2);

    return(oneDimHowClose(start.row, end.row, goal.row) +
           oneDimHowClose(start.col, end.col, goal.col));
  }

// for a given list of moves, find the one that best improves the
// "development" of a player's pieces.  development is a combination
// of covering alot of the board, getting pieces close to the enemy's
// king, and restricting the enemy king's movement.
LOCAL int bestDevelopMove 
  (
    BOARD &board,
    PIECECOLOR moveColor,
    BESTMOVES &bestMoves
  )
  {
    POSITION whereEnemyKing = whereKing(board, OtherColor(moveColor));
    int testMetric, bestMetric = INT_MIN;
    int bestIndex, testIndex;
    MOVEUNDODATA undoData;

    for (testIndex = 0; testIndex < bestMoves.nMoves; testIndex++)
      {
	// engage in castlephilia
	if (bestMoves.move[testIndex].type != NORMALMOVE)
	  return(testIndex);

	board.doMove
	  (
	    bestMoves.move[testIndex].start,
	    bestMoves.move[testIndex].end,
	    undoData
	  );

	if (bestMoves.move[testIndex].promoteType != TYPENOPIECE)
	  board.promote(bestMoves.move[testIndex].end,
			bestMoves.move[testIndex].promoteType);

	testMetric = undoData.capturedPiece ?
		     undoData.capturedPiece->whatValue() * 16 : 0;
	testMetric += coverage(board, moveColor, whereEnemyKing) +
		      threatChange
		        (
			  bestMoves.move[testIndex].start,
			  bestMoves.move[testIndex].end,
			  whereEnemyKing,
			  board.whatPiece(bestMoves.move[testIndex].end)
		        ) * 4;

	if (testMetric > bestMetric)
	  {
	    bestIndex = testIndex;
	    bestMetric = testMetric;
	  }

	if (bestMoves.move[testIndex].promoteType != TYPENOPIECE)
	  board.restorePawn(bestMoves.move[testIndex].end);

	board.undoMove
	  (
	    bestMoves.move[testIndex].end,
	    bestMoves.move[testIndex].start,
	    undoData
	  );
      }

    return(bestIndex);
  }

GAMESTATUS COMPUTERPLAYER::play(BOARD &board) const
  {
    BOARDMETRIC metric;
    BESTMOVES bestMoves;
    int best;

    ChessUI.thinkingMessage(whatColor());
    board.findBestMoves(lookAhead, whatColor(), metric, &bestMoves);
    if (metric.kingSituation[whatColor()] != KINGOK)
      {
	// see if checkmate/stalemate current or predicted
	if (lookAhead > 2)
	  board.findBestMoves(2, whatColor(), metric, &bestMoves);

	if (metric.kingSituation[whatColor()] == KINGLOST)
	  {
	    ChessUI.clearMessage();
	    ChessUI.mated(whatColor());
	    return(GAMEOVER);
	  }
	else if (metric.kingSituation[whatColor()] == STALEMATE)
	  {
	    ChessUI.clearMessage();
	    ChessUI.staleMated(whatColor());
	    return(GAMEOVER);
	  }
      }

    best = bestDevelopMove(board, whatColor(), bestMoves);

    if (!ChessUI.computerMove(board, whatColor(),
                              bestMoves.move[best]))
      return(GAMEOVER);
    return(GAMECONTINUE);
  }
