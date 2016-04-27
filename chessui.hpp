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

#if !defined(CHESSUI_HPP)
#define CHESSUI_HPP

#include "misc.hpp"
#include "chess.hpp"

// highest layer of user interface
class CHESSUSERIFACE
  {
  public:
    // tell user heap memory exhausted
    CLASSMEMBER void outOfMemory(void);
    // initialize user interface, with board in given configuration
    CLASSMEMBER void init(const BOARD &board);
    // tell user the king of the given color is in checkmate
    CLASSMEMBER void mated(PIECECOLOR color);
    // tell user the king of the given color is in stalemate
    CLASSMEMBER void staleMated(PIECECOLOR color);
    // ask user how to move a piece of the given color on the board.
    // display the move and also invoke the doUserMove member function
    // of board for the move.
    CLASSMEMBER BOOL userMove(BOARD &board, PIECECOLOR color);
    // tell the user the player of the given color is thinking about
    // their next move
    CLASSMEMBER void thinkingMessage(PIECECOLOR color);
    // clear the last Message
    CLASSMEMBER void clearMessage(void);
    // display a move that has been selected by the computer.
    // also invokes the doMove member of board for the move.
    CLASSMEMBER BOOL computerMove
      (
        BOARD &board,
        PIECECOLOR color,
        PIECEMOVE &moveInfo
      );
  };

// only instance of this class
extern CHESSUSERIFACE ChessUI;

#endif
