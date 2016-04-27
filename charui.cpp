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

#include <cstdio>
#include <cstdlib>

using namespace std;

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "misc.hpp"
#include "brdsize.hpp"
#include "charui.hpp"

CHARUSERIFACE CharUI;

namespace
{

int tty_fd;

void tty_wr(int c)
  {
    char b[1];
    b[0] = static_cast<char>(c);

    static_cast<void>(write(tty_fd, b, 1));
  }

const int ESC = 0x1b;

void csi()
  {
    tty_wr(ESC);
    tty_wr('[');
  }

// 3rd character of escape sequences
const int CURSOR_HOME = 'H';
const int CLEAR_SCREEN = 'J';

void num2d(int n)
  {
    if (n >= 10)
      tty_wr((n / 10) + '0');
    tty_wr((n % 10) + '0');
  }

int tty_rd()
  {
    unsigned char c;

    if (read(tty_fd, &c, 1) != 1)
      exit(1);

    return(c);
  }

termios save_tios;

}

CHARUSERIFACE::CHARUSERIFACE(void)
  {
    tty_fd = open("/dev/tty", O_RDWR);
    if (tty_fd < 0)
      {
        fprintf(stderr, "Error opening /dev/tty\n");
        exit(1);
      }

    {
      termios tios;

      if (tcgetattr(tty_fd, &tios))
        {
          fprintf(stderr, "Error getting terminal attributes\n");
          exit(1);
        }

      save_tios = tios;

      cfmakeraw(&tios);

      if (tcsetattr(tty_fd, TCSANOW, &tios))
        {
          fprintf(stderr, "Error setting terminal attributes\n");
          exit(1);
        }
    }

    // Hide cursor.
    csi();
    tty_wr('?');
    tty_wr('2');
    tty_wr('5');
    tty_wr('l');

    csi();
    tty_wr(CURSOR_HOME);

    csi();
    tty_wr(CLEAR_SCREEN);

    return;
  }

CHARUSERIFACE::~CHARUSERIFACE(void)
  {
    csi();
    tty_wr(CURSOR_HOME);

    csi();
    tty_wr(CLEAR_SCREEN);

    // Show cursor.
    csi();
    tty_wr('?');
    tty_wr('2');
    tty_wr('5');
    tty_wr('h');

    tcsetattr(tty_fd, TCSANOW, &save_tios);

    close(tty_fd);
  }

void CHARUSERIFACE::showChar(int row, int col, char c, BOOL inverse)
  {
    // Position cursor.
    csi();
    num2d(row + 1);
    tty_wr(';');
    num2d(col + 1);
    tty_wr('H');

    if (inverse)
      {
        csi();
        tty_wr('3');
        tty_wr('0');
        tty_wr(';');
        tty_wr('4');
        tty_wr('7');
        tty_wr('m');
      }

    tty_wr(c);

    if (inverse)
      {
        csi();
        tty_wr('3');
        tty_wr('7');
        tty_wr(';');
        tty_wr('4');
        tty_wr('0');
        tty_wr('m');
      }

    return;
  }

uint CHARUSERIFACE::readKey(void)
  {
    int c = tty_rd();

    if ((c == 'x') || (c == 'X'))
      return(KEYEXIT);

    if (c == ESC)
      {
        c = tty_rd();
        if (c == '[')
          c = (1 << 8) | tty_rd();
        else
          c = ESC;
      }
        

    return(c);
  }
