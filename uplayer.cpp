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

#include "chess.hpp"
#include "uplayer.hpp"
#include "chessui.hpp"

GAMESTATUS USERPLAYER::play(BOARD &board) const
  {
    BOARDMETRIC metric;

    board.findBestMoves(2, whatColor(), metric, (BESTMOVES *) 0);
    switch (metric.kingSituation[whatColor()])
      {
      case KINGLOST:
        ChessUI.mated(whatColor());
        return(GAMEOVER);

      case STALEMATE:
        ChessUI.staleMated(whatColor());
        return(GAMEOVER);

      case KINGOK:
        break;
      }

    if (!ChessUI.userMove(board, whatColor()))
      return(GAMEOVER);
    return(GAMECONTINUE);
  }
