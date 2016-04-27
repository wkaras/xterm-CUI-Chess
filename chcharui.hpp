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

#if !defined(CHCHARUI_HPP)
#define CHCHARUI_HPP

#include "misc.hpp"
#include "brdsize.hpp"

// middle layer of user interface
class CHESSCHARUSERIFACE
  {
  public:
    // display empty chess board and empty message area
    CLASSMEMBER void initScreen(void);

    // show the abbreviation for a piece at a given location on
    // the displayed chess board
    CLASSMEMBER void showPiece(POSITION whereBoard, const char *abbrev);

    // clear a displayed piece at a given location on the chess board
    CLASSMEMBER void clearPiece(POSITION whereBoard);

    // show a message in the message area of the screen.  the
    // parameter is a pointer to an array of pointers to strings.
    // the function logically concatenates these strings into a
    // single message string.  the message is displayed left-justified
    // in the message area.  line-breaks are inserted as necessary in
    // place of blanks in the logical message string.
    CLASSMEMBER void showMessage(const char **textList);

    // clear last message displayed
    CLASSMEMBER void clearMessage(void);

    // show a message and wait for the user to press a key in response.
    // user may press a key corresponding to the list of characters
    // specified by keyList, or the escape key.  if the escape key is
    // pressed the function returns FALSE.  Otherwise, the function
    // returns TRUE, and keyIndex is set to the index of character in
    // keyList corresponding to the key pressed.
    CLASSMEMBER BOOL showMessage
      (
        // same as textList parameter in first showMessage
        const char **textList,
        // list of accepted character keys.  if null, any key is
        // accepted.
        const char *keyList,
        // if not null, pointer to variable to set to the index of
        // the key pressed
        uint *keyIndex
      );

    // display a message, then let the user select a position on the 
    // chess board.  if the user pressed the escape key, the function
    // returns FALSE.  Otherwise the function returns TRUE, and the
    // selected position is returned in whereBoard.
    CLASSMEMBER BOOL selectPosition
      (
        const char **textList,
        POSITION &whereBoard
      );

    // highlights a position on the board
    CLASSMEMBER void setSelect(POSITION whereBoard);
    // clears highlight of a position on the board
    CLASSMEMBER void clearSelect(POSITION whereBoard);
  };

// only instance of this class
extern CHESSCHARUSERIFACE ChessCharUI;

#endif
