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
#include "chessui.hpp"
#include "chcharui.hpp"

CHESSUSERIFACE ChessUI;

const char whiteText[] = "White",
           blackText[] = "Black";

LOCAL const char *colorText(PIECECOLOR color)
  { return(color == WHITE ? whiteText : blackText); }

// returns piece abbreviation to be displayed in board squares
// for a given piece
LOCAL const char *pieceAbbrev
  (
    PIECECOLOR color,
    PIECETYPE type
  )
  {
    if (color == WHITE)
      switch (type)
        {
        case TYPEKING:   return("WK");
        case TYPEQUEEN:  return("WQ");
        case TYPEBISHOP: return("WB");
        case TYPEKNIGHT: return("WN");
        case TYPEROOK:   return("WR");
        case TYPEPAWN:   return("WP");
        }
    else
      switch (type)
        {
        case TYPEKING:   return("BK");
        case TYPEQUEEN:  return("BQ");
        case TYPEBISHOP: return("BB");
        case TYPEKNIGHT: return("BN");
        case TYPEROOK:   return("BR");
        case TYPEPAWN:   return("BP");
        }
    return(""); // dummy return
  }

const char kingText[] = " King",
           queenText[] = " Queen",
           bishopText[] = " Bishop",
           knightText[] = " Knight",
           rookText[] = " Rook",
           pawnText[] = " Pawn";

// fills in two entries in a message textList with color/name of
// a piece
LOCAL void pieceText
  (
    PIECECOLOR color,
    PIECETYPE type,
    const char **textList
  )
  {
    *(textList++) = colorText(color);

    switch (type)
      {
      case TYPEKING:
        *textList = kingText;
        return;

      case TYPEQUEEN:
        *textList = queenText;
        return;

      case TYPEBISHOP:
        *textList = bishopText;
        return;

      case TYPEKNIGHT:
        *textList = knightText;
        return;

      case TYPEROOK:
        *textList = rookText;
        return;

      case TYPEPAWN:
        *textList = pawnText;
        return;
      }
  }

// base textLists for messages.  terminated by null pointers.  some
// have null pointer place holders for variable portions of message.

const char *noMemory[] =
  {
    "Insufficient memory.  Press any key to exit:",
    (char *) 0
  };

const char *checkMated[] =
  {
    (char *) 0,
    " King in checkmate.  Press any key to exit:",
    (char *) 0
  };
const int MATEDCOLORINDEX = 0;

const char *staleMate[] =
  {
    (char *) 0,
    " King in stalemate.  Press any key to exit:",
    (char *) 0
  };
const int STALEMATEDCOLORINDEX = 0;

const char *selectPiece[] =
  {
    (char *) 0,
    " Player:  Select piece to move with arrow keys, then hit enter:",
    (char *) 0
  };
const int SELECTCOLORINDEX = 0;

const char *illegalSelection[] =
  {
    "That location does not contain one of your pieces."
    "  Press any key to try again:",
    (char *) 0
  };

const char *moveIllegal[] =
  {
    "You cannot move the selected piece to that location."
    "  Press any key to try again:",
    (char *) 0
  };

const char *wouldLoseKing[] =
  {
    "This move would result in your King being taken by the ",
    (char *) 0,
    (char *) 0,
    ".  Press any key to select another move:",
    (char *) 0
  };
const int DANGERINDEX = 1;

const char *castleQueenRook[] =
  {
    "Do you want to castle with Queen's Rook? (Type Y or N):",
    (char *) 0
  };

const char *castleKingRook[] =
  {
    "Do you want to castle with King's Rook? (Type Y or N):",
    (char *) 0
  };

const char *selectDest[] =
  {
    "Select destination of move with arrow keys, then hit enter:",
    (char *) 0
  };

const char *promoteToWhat[] =
  {
    "Do you want your Pawn to be promoted to a Queen, Bishop, "
    "Knight or Rook?  (Type Q, B, K or R):",
    (char *) 0
  };

// character key lists

const char promoteOptions[] = "qQbBkKrR";
const PIECETYPE promoteType[] = { TYPEQUEEN, TYPEBISHOP, TYPEKNIGHT,
                                  TYPEROOK };

const char yesNoAnswer[] = "yYnN";
const int YESINDEX = 0;
const int NOINDEX = 1;

// more message text lists

const char *thinking[] =
  {
    (char *) 0,
    " Player thinking, please wait...",
    (char *) 0
  };
const int THINKINGCOLORINDEX = 0;

const char *doCastleQueenRook[] =
  {
    (char *) 0,
    " Player castles with Queen's Rook.  Press any key to continue:",
    (char *) 0
  };
const char *doCastleKingRook[] =
  {
    (char *) 0,
    " Player castles with King's Rook.  Press any key to continue:",
    (char *) 0
  };
const int CASTLECOLORINDEX = 0;

const char *whatMoved[] =
  {
    (char *) 0,
    " Player moves ",
    (char *) 0,
    (char *) 0,
    ".  Press any key to continue:",
    (char *) 0
  };
const int MOVECOLORINDEX = 0;
const int MOVEPIECEINDEX = 2;

const char *whatMovedCaptured[] =
  {
    (char *) 0,
    " Player moves ",
    (char *) 0,
    (char *) 0,
    ", capturing ",
    (char *) 0,
    (char *) 0,
    ".  Press any key to continue:",
    (char *) 0
  };
const int CAPTURINGCOLORINDEX = 0;
const int CAPTURINGPIECEINDEX = 2;
const int CAPTUREDPIECEINDEX = 5;

const char *doPromotion[] =
  {
    (char *) 0,
    " Player promotes Pawn to ",
    (char *) 0,
    (char *) 0,
    ".  Press any key to continue:",
    (char *) 0
  };
const int PROMOTECOLORINDEX = 0;
const int PROMOTEPIECEINDEX = 2;

void CHESSUSERIFACE::outOfMemory(void)
  {
    ChessCharUI.showMessage(noMemory, (char *) 0, (uint *) 0);

    return;
  }

void CHESSUSERIFACE::init(const BOARD &board)
  {
    int row, col;
    PIECE *p;

    ChessCharUI.initScreen();

    for (row = 0; row < NUMROWS; row++)
      for (col = 0; col < NUMCOLS; col++)
        {
          p = board.whatPiece(row, col);
          if (p)
            ChessCharUI.showPiece
              (
                POSITION(row, col),
                pieceAbbrev(p->whatColor(), p->whatType())
              );
        }

    return;
  }

void CHESSUSERIFACE::mated(PIECECOLOR color)
  {
    checkMated[MATEDCOLORINDEX] = colorText(color);
    ChessCharUI.showMessage(checkMated, (char *) 0, (uint *) 0);

    return;
  }

void CHESSUSERIFACE::staleMated(PIECECOLOR color)
  {
    staleMate[STALEMATEDCOLORINDEX] = colorText(color);
    ChessCharUI.showMessage(staleMate, (char *) 0, (uint *) 0);

    return;
  }

BOOL CHESSUSERIFACE::userMove(BOARD &board, PIECECOLOR color)
  {
    POSITION start, end;
    PIECE *p;
    uint keyIndex;
    MOVESTATUS moveStatus;
    MOVEUNDODATA dummy;

    selectPiece[SELECTCOLORINDEX] = colorText(color);

    // loop to get legal move
    for ( ; ; )
      {
        // loop to get piece to move
        for ( ; ; )
          {
            start.row = start.col = 0;
            if (!ChessCharUI.selectPosition(selectPiece, start))
              return(FALSE);

            p = board.whatPiece(start);
            if (p)
              if (p->whatColor() == color)
                break;

            if (!ChessCharUI.showMessage(illegalSelection,
                                         (char *) 0, (uint *) 0))
              return(FALSE);

            ChessCharUI.clearSelect(start);
          }

        if (board.userCanCastle(QUEENSIDECASTLE, color))
          if ((p->whatType() == TYPEKING) ||
              ((p->whatType() == TYPEROOK) && (start.row == 0)))
            {
              if (!ChessCharUI.showMessage(castleQueenRook,
                                           yesNoAnswer, &keyIndex))
                return(FALSE);
              if ((keyIndex / 2) == YESINDEX)
                {
                  ChessCharUI.clearSelect(start);
                  board.castle(QUEENSIDECASTLE, color, dummy);
                  ChessCharUI.clearPiece(POSITION(0, start.col));
                  ChessCharUI.showPiece
                    (
                      POSITION(3, start.col),
                      pieceAbbrev(color, TYPEROOK)
                    );
                  ChessCharUI.clearPiece(POSITION(4, start.col));
                  ChessCharUI.showPiece
                    (
                      POSITION(2, start.col),
                      pieceAbbrev(color, TYPEKING)
                    );
                  return(TRUE);
                }
            }

        if (board.userCanCastle(KINGSIDECASTLE, color))
          if ((p->whatType() == TYPEKING) ||
              ((p->whatType() == TYPEROOK) && (start.row == 7)))
            {
              if (!ChessCharUI.showMessage(castleKingRook,
                                           yesNoAnswer, &keyIndex))
                return(FALSE);
              if ((keyIndex / 2) == YESINDEX)
                {
                  ChessCharUI.clearSelect(start);
                  board.castle(KINGSIDECASTLE, color, dummy);
                  ChessCharUI.clearPiece(POSITION(7, start.col));
                  ChessCharUI.showPiece
                    (
                      POSITION(5, start.col),
                      pieceAbbrev(color, TYPEROOK)
                    );
                  ChessCharUI.clearPiece(POSITION(4, start.col));
                  ChessCharUI.showPiece
                    (
                      POSITION(6, start.col),
                      pieceAbbrev(color, TYPEKING)
                    );
                  return(TRUE);
                }
            }

        end = start;
        if (!ChessCharUI.selectPosition(selectDest, end))
          return(FALSE);

        moveStatus = board.doUserMove
                       (
                         start,
                         end
                       );

        if ((moveStatus.status == MOVEDONE) ||
            (moveStatus.status == MOVEENPASSANT))
          break;

        if (moveStatus.status == ILLEGALMOVE)
          {
            if (!ChessCharUI.showMessage(moveIllegal, (char *) 0,
                                         (uint *) 0))
              return(FALSE);
          }
        else // king would be taken
          {
            pieceText
              (
                OtherColor(color),
                board.whatPiece(moveStatus.dangerToKing)->whatType(),
                wouldLoseKing + DANGERINDEX
              );
            if (!ChessCharUI.showMessage(wouldLoseKing, (char *) 0,
                                         (uint *) 0))
              return(FALSE);
          }
        ChessCharUI.clearSelect(start);
        ChessCharUI.clearSelect(end);

      } // end loop to get legal move

    ChessCharUI.clearSelect(start);
    ChessCharUI.clearPiece(start);
    ChessCharUI.clearPiece(end);
    if (moveStatus.status == MOVEENPASSANT)
      ChessCharUI.clearPiece(POSITION(end.row, start.col));
    ChessCharUI.showPiece
      (
        end,
        pieceAbbrev(color, p->whatType())
      );

    if (board.canPromote(end))
      {
        if (!ChessCharUI.showMessage(promoteToWhat, promoteOptions,
                                     &keyIndex))
          return(FALSE);
        keyIndex /= 2;
        board.promote(end, promoteType[keyIndex]);
        ChessCharUI.clearPiece(end);
        ChessCharUI.showPiece
          (
            end,
            pieceAbbrev(color, promoteType[keyIndex])
          );
      }
    ChessCharUI.clearSelect(end);

    return(TRUE);
  }

void CHESSUSERIFACE::thinkingMessage(PIECECOLOR color)
  {
    thinking[THINKINGCOLORINDEX] = colorText(color);

    ChessCharUI.showMessage(thinking);

    return;
  }

void CHESSUSERIFACE::clearMessage(void)
  {
    ChessCharUI.clearMessage();

    return;
  }

BOOL CHESSUSERIFACE::computerMove
  (
    BOARD &board,
    PIECECOLOR color,
    PIECEMOVE &moveInfo
  )
  {
    int backCol = color == WHITE ? 0 : 7;
    MOVEUNDODATA undoData;

    switch (moveInfo.type)
      {
      case KINGSIDECASTLE:
        board.castle(KINGSIDECASTLE, color, undoData);
        ChessCharUI.clearPiece(POSITION(7, backCol));
        ChessCharUI.clearPiece(POSITION(4, backCol));
        ChessCharUI.showPiece
          (
            POSITION(5, backCol),
            pieceAbbrev(color, TYPEROOK)
          );
        ChessCharUI.showPiece
          (
            POSITION(6, backCol),
            pieceAbbrev(color, TYPEKING)
          );
        ChessCharUI.setSelect(POSITION(5, backCol));
        ChessCharUI.setSelect(POSITION(6, backCol));
        doCastleKingRook[CASTLECOLORINDEX] = colorText(color);
        if (!ChessCharUI.showMessage(doCastleKingRook,
                                     (char *) 0, (uint *) 0))
          return(FALSE);
        ChessCharUI.clearSelect(POSITION(5, backCol));
        ChessCharUI.clearSelect(POSITION(6, backCol));
        return(TRUE);
        
      case QUEENSIDECASTLE:
        board.castle(QUEENSIDECASTLE, color, undoData);
        ChessCharUI.clearPiece(POSITION(0, backCol));
        ChessCharUI.clearPiece(POSITION(4, backCol));
        ChessCharUI.showPiece
          (
            POSITION(3, backCol),
            pieceAbbrev(color, TYPEROOK)
          );
        ChessCharUI.showPiece
          (
            POSITION(2, backCol),
            pieceAbbrev(color, TYPEKING)
          );
        ChessCharUI.setSelect(POSITION(3, backCol));
        ChessCharUI.setSelect(POSITION(2, backCol));
        doCastleQueenRook[CASTLECOLORINDEX] = colorText(color);
        if (!ChessCharUI.showMessage(doCastleQueenRook,
                                     (char *) 0, (uint *) 0))
          return(FALSE);
        ChessCharUI.clearSelect(POSITION(3, backCol));
        ChessCharUI.clearSelect(POSITION(2, backCol));
        return(TRUE);
        
      case NORMALMOVE:
        board.doMove(moveInfo.start, moveInfo.end, undoData);
        ChessCharUI.clearPiece(moveInfo.start);
        ChessCharUI.clearPiece(moveInfo.end);
        if (undoData.enPassantEffect == ENPASSANTCAPTURE)
          ChessCharUI.clearPiece(POSITION(moveInfo.end.row,
                                          moveInfo.start.col));
        ChessCharUI.showPiece
          (
            moveInfo.end,
            pieceAbbrev(color,
                        board.whatPiece(moveInfo.end)->whatType())
          );
        ChessCharUI.setSelect(moveInfo.start);
        ChessCharUI.setSelect(moveInfo.end);

        if (undoData.capturedPiece)
          {
            whatMovedCaptured[CAPTURINGCOLORINDEX] = colorText(color);
            pieceText
              (
                color,
                board.whatPiece(moveInfo.end)->whatType(),
                whatMovedCaptured + CAPTURINGPIECEINDEX
              );
            pieceText
              (
                OtherColor(color),
                undoData.capturedPiece->whatType(),
                whatMovedCaptured + CAPTUREDPIECEINDEX
              );

            delete undoData.capturedPiece;

            if (!ChessCharUI.showMessage(whatMovedCaptured,
                                         (char *) 0, (uint *) 0))
              return(FALSE);
          }
        else
          {
            whatMoved[MOVECOLORINDEX] = colorText(color);
            pieceText
              (
                color,
                board.whatPiece(moveInfo.end)->whatType(),
                whatMoved + MOVEPIECEINDEX
              );
            if (!ChessCharUI.showMessage(whatMoved,
                                         (char *) 0, (uint *) 0))
              return(FALSE);
          }

        if (moveInfo.promoteType != TYPENOPIECE)
          {
            board.promote(moveInfo.end, moveInfo.promoteType);
            ChessCharUI.clearPiece(moveInfo.end);
            ChessCharUI.showPiece
              (
                moveInfo.end,
                pieceAbbrev(color, moveInfo.promoteType)
              );
            doPromotion[PROMOTECOLORINDEX] = colorText(color);
            pieceText
              (
                color,
                moveInfo.promoteType,
                doPromotion + PROMOTEPIECEINDEX
              );
            if (!ChessCharUI.showMessage(doPromotion, (char *) 0,
                                         (uint *) 0))
              return(FALSE);
          }
        ChessCharUI.clearSelect(moveInfo.start);
        ChessCharUI.clearSelect(moveInfo.end);

        return(TRUE);

      } // end of switch

    return(FALSE);

  } // end of function
