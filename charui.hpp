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

#if !defined(CHARUI_HPP)
#define CHARUI_HPP

#include "misc.hpp"
#include "brdsize.hpp"

// lowest layer of user interface
class CHARUSERIFACE
  {
  public:
    CHARUSERIFACE(void);
    ~CHARUSERIFACE(void);

    // show a character on the screen
    CLASSMEMBER void showChar
      (
        // position to show the character at
        int row, int col,
        // character to show
        char c,
        // display character in inverse-video?
        BOOL inverse
      );
        
    CLASSMEMBER void showChar(POSITION p, char c, BOOL inverse)
      { showChar(p.row, p.col, c, inverse); return; }

    // wait for key press by user, return its code.  codes are
    // ASCII codes for characters corresponding to keys, except for
    // those listed below.
    CLASSMEMBER uint readKey(void);
  };

// codes for non-character keys
const uint KEYEXIT = 1 << 8;
const uint KEYENTER = 0x000D;
const uint KEYUP = (1 << 8) | 'A';
const uint KEYDOWN = (1 << 8) | 'B'; 
const uint KEYRIGHT = (1 << 8) | 'C';
const uint KEYLEFT = (1 << 8) | 'D';

// only instance of this class
extern CHARUSERIFACE CharUI;

#endif
