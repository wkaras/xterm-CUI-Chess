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

#include <stdlib.h>
#include <string.h>

#include "chess.hpp"
#include "player.hpp"
#include "uplayer.hpp"
#include "cplayer.hpp"
#include "chessui.hpp"

void OutOfMemory(void)
  {
    ChessUI.outOfMemory();
    exit(1);
  }

// define a player based on a command line parameter.  player
// is allocated on heap.
LOCAL PLAYER *defPlayer(PIECECOLOR color, const char *arg)
  {
    BOOL computer = TRUE;
    int lookAhead;
    PLAYER *player;

    if (strcasecmp(arg, "u") == 0)
      computer = FALSE;
    else if (strcasecmp(arg, "c1") == 0)
      lookAhead = 2;
    else if (strcasecmp(arg, "c2") == 0)
      lookAhead = 3;
    else if (strcasecmp(arg, "c3") == 0)
      lookAhead = 4;
    else if (strcasecmp(arg, "c4") == 0)
      lookAhead = 5;
    else if (strcasecmp(arg, "c5") == 0)
      lookAhead = 6;
    else if (strcasecmp(arg, "c6") == 0)
      lookAhead = 7;
    else
      exit(1);

    if (computer)
      player = new COMPUTERPLAYER(color, lookAhead);
    else
      player = new USERPLAYER(color);

    if (!player)
      OutOfMemory();

    return(player);
  }


// define the two players based on command line arguments (or
// defaults)
LOCAL void setupPlayers
  (
    int nArg,
    char **arg,
    PLAYER *&whitePlayer,
    PLAYER *&blackPlayer
  )
  {
    const char *white = "u", *black = "c2";

    if (nArg > 1)
      white = arg[1];
    if (nArg > 2)
      black = arg[2];
    if (nArg > 3)
      exit(1);

    whitePlayer = defPlayer(WHITE, white);
    blackPlayer = defPlayer(BLACK, black);

    return;
  }

int main(int nArg, char **arg)
  {
    BOARD board;
    PLAYER *whitePlayer, *blackPlayer;

    setupPlayers(nArg, arg, whitePlayer, blackPlayer);

    ChessUI.init(board);

    for ( ; ; )
      {
        if (whitePlayer->play(board) == GAMEOVER)
          break;
        if (blackPlayer->play(board) == GAMEOVER)
          break;
      }

    delete whitePlayer;
    delete blackPlayer;

    return(0);
  }
