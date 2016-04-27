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
#include "charui.hpp"
#include "chcharui.hpp"

CHESSCHARUSERIFACE ChessCharUI;

// internal representation of displayed board
LOCAL class
  {
  public:
    const char *pieceText;
    BOOL selected;
  }
displayBoard[NUMROWS][NUMCOLS];

const char emptyText[] = "  ";

// width and height of a square of the displayed board, in character
// cells
const int POSITIONHEIGHT = 3;
const int POSITIONWIDTH = 6;

// offsets in # of character cells of where to display the piece
// abbreviation within the board square
const int PIECETEXTROWOFFSET = 1;
const int PIECETEXTCOLOFFSET = 2;

// the starting row and starting, ending column of the message
// display area
const int MSGSTARTROW = 1;
const int MSGSTARTCOL = 50;
const int MSGENDCOL = 78;

// returns TRUE if the board square corresponding to the given
// board position should be displayed as white
inline BOOL isWhite(POSITION board)
  { return((board.row + board.col) & 1); }

void CHESSCHARUSERIFACE::initScreen(void)
  {
    POSITION board, screen, position;
    BOOL inverse;

    // display empty board
    screen.row = 0;
    for (board.row = 0; board.row < NUMROWS; board.row++)
      for (position.row = 0; position.row < POSITIONHEIGHT; position.row++)
	{
	  screen.col = 0;
	  for (board.col = 0; board.col < NUMCOLS; board.col++)
	    {
	      inverse = isWhite(board);
	      for (position.col = 0; position.col < POSITIONWIDTH;
		   position.col++)
		{
		  CharUI.showChar(screen, ' ', inverse);
		  screen.col++;
		}
	    }
	  screen.row++;
	}

    // initialize internal representation of displayed board
    for (board.row = 0; board.row < NUMROWS; board.row++)
      for (board.col = 0; board.col < NUMCOLS; board.col++)
	{
	  displayBoard[board.row][board.col].pieceText = emptyText;
	  displayBoard[board.row][board.col].selected = FALSE;
	}

    return;
  }

// display piece abbrevation at given location on board
LOCAL void showBoardText(POSITION where, const char *text, BOOL inverse)
  {
    where.row *= POSITIONHEIGHT;
    where.row += PIECETEXTROWOFFSET;
    where.col *= POSITIONWIDTH;
    where.col += PIECETEXTCOLOFFSET;

    while (*text)
      {
	CharUI.showChar(where, *text, inverse);
	text++;
	where.col++;
      }

    return;
  }

void CHESSCHARUSERIFACE::showPiece(POSITION whereBoard, const char *abbrev)
  {
    BOOL inverse = isWhite(whereBoard);

    if (displayBoard[whereBoard.row][whereBoard.col].selected)
      inverse = !inverse;

    showBoardText(whereBoard, abbrev, inverse);

    displayBoard[whereBoard.row][whereBoard.col].pieceText = abbrev;

    return;
  }

void CHESSCHARUSERIFACE::clearPiece(POSITION whereBoard)
  {
    displayBoard[whereBoard.row][whereBoard.col].pieceText = emptyText;
    showPiece(whereBoard, emptyText);

    return;
  }

LOCAL int firstBlankMsgRow = MSGSTARTROW;

// clear last message displayed in message area
LOCAL void clearLastMsg(void)
  {
    int row = MSGSTARTROW, col;

    while (row < firstBlankMsgRow)
      {
	for (col = MSGSTARTCOL; col <= MSGENDCOL; col++)
	  CharUI.showChar(row, col, ' ', FALSE);
	row++;
      }
    firstBlankMsgRow = MSGSTARTROW;
  }

// move to next character in message
LOCAL inline void incTextList
  (
    // pointer to pointer to current string
    const char **&textList,
    // index of current character within current string
    int &index
  )
  {
    index++;
    if (!(*textList)[index])
      {
	textList++;
	index = 0;
      }

    return;
  }

// move to next blank space in message
LOCAL int toEndWord
  (
    // pointer to pointer to current string
    const char **&textList,
    // index of current character within current string
    int &index
  )
  {
    int nChars = 0;

    if (!*textList)
      return(0);

    while ((*textList)[index] == ' ')
      {
	incTextList(textList, index);
	nChars++;
	if (!*textList)
	  return(nChars);
      }

    while ((*textList)[index] != ' ')
      {
	incTextList(textList, index);
	nChars++;
	if (!*textList)
	  return(nChars);
      }

    return(nChars);
  }

// display a message in the message area
LOCAL void displayMsg
  (
    const char **textList
  )
  {
    int row = MSGSTARTROW, col = MSGSTARTCOL;
    int showIndex = 0, lookIndex = 0, nChars;
    const char **lookList = textList;

    clearLastMsg();

    while (*textList)
      {
	nChars = toEndWord(lookList, lookIndex);

	if ((col + nChars) > (MSGENDCOL + 1))
	  {
	    // go to next line in message area
	    row++;
	    col = MSGSTARTCOL;
	    while ((*textList)[showIndex] == ' ')
	      incTextList(textList, showIndex);
	  }

	for ( ; ; )
	  {
	    if (textList == lookList)
	      if (showIndex == lookIndex)
		break;
	    CharUI.showChar(row, col++, (*textList)[showIndex], FALSE);
	    incTextList(textList, showIndex);
	  }
      }

    firstBlankMsgRow = row + 1;

    return;
  }

void CHESSCHARUSERIFACE::showMessage
  (
    const char **textList
  )
  {
    displayMsg(textList);

    return;
  }

void CHESSCHARUSERIFACE::clearMessage(void)
  {
    clearLastMsg();

    return;
  }

BOOL CHESSCHARUSERIFACE::showMessage
  (
    const char **textList,
    const char *keyList,
    uint *keyIndex
  )
  {
    uint keyPressed, i;

    displayMsg(textList);

    for ( ; ; )
      {
	keyPressed = CharUI.readKey();

        if (keyPressed == KEYEXIT)
	  return(FALSE);

	if (!keyList)
	  return(TRUE);

	for (i = 0; keyList[i]; i++)
	  {
	    if (keyList[i] == (char) keyPressed)
	      {
		*keyIndex = i;
		return(TRUE);
	      }
	  }
      }
  }

void CHESSCHARUSERIFACE::setSelect(POSITION whereBoard)
  {
    showBoardText
      (
	whereBoard,
	displayBoard[whereBoard.row][whereBoard.col].pieceText,
	!isWhite(whereBoard)
      );
    displayBoard[whereBoard.row][whereBoard.col].selected = TRUE;

    return;
  }

void CHESSCHARUSERIFACE::clearSelect(POSITION whereBoard)
  {
    showBoardText
      (
	whereBoard,
	displayBoard[whereBoard.row][whereBoard.col].pieceText,
	isWhite(whereBoard)
      );
    displayBoard[whereBoard.row][whereBoard.col].selected = FALSE;

    return;
  }

BOOL CHESSCHARUSERIFACE::selectPosition
  (
    const char **textList,
    POSITION &whereBoard
  )
  {
    uint keyPressed;

    displayMsg(textList);

    for ( ; ; )
      {
	showBoardText
	  (
	    whereBoard,
	    displayBoard[whereBoard.row][whereBoard.col].pieceText,
	    !isWhite(whereBoard)
	  );
	keyPressed = CharUI.readKey();
        if (keyPressed == KEYENTER)
	  {
	    displayBoard[whereBoard.row][whereBoard.col].selected =
	      TRUE;
	    return(TRUE);
	  }
	if (!displayBoard[whereBoard.row][whereBoard.col].selected)
	  showBoardText
	    (
	      whereBoard,
	      displayBoard[whereBoard.row][whereBoard.col].pieceText,
	      isWhite(whereBoard)
	    );

        if (keyPressed == KEYEXIT)
	  return(FALSE);

	if ((keyPressed == KEYUP) && (whereBoard.row != 0))
	  whereBoard.row--;
	else if ((keyPressed == KEYDOWN) &&
                 (whereBoard.row != (NUMROWS - 1)))
	  whereBoard.row++;
	else if ((keyPressed == KEYLEFT) &&
                 (whereBoard.col != 0))
	  whereBoard.col--;
	else if ((keyPressed == KEYRIGHT) &&
                 (whereBoard.col != (NUMCOLS - 1)))
	  whereBoard.col++;
      }
  }
