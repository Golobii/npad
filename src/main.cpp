#include <iostream>
#include <ncurses.h>
#include <pthread.h>

#include "../include/fileManager.h"

#define NORMAL 1
#define CURSOR_COLOR_PAIR 2

void *manageKeys(void *threadId);

enum Mode
{
  NORMAL_MODE,
  INSERT_MODE
};

class Base : public FileManager
{
private:
  void init();

  void lineDown();
  void lineUp();
  void moveCursorLeft();
  void moveCursorRight();

  void moveLines(int n);

  unsigned int getPosition();
  unsigned int getNumOfLines();
  unsigned int getLineLength(unsigned int n);

  unsigned int cursor_x, cursor_y;
  unsigned int index;
  unsigned int prevXPosition = 0;

  unsigned int line;

  int max_y, max_x;

  Mode mode = NORMAL_MODE;

public:
  void open(string fileName);
};

void Base::open(string fileName)
{
  init();
  Base::buffer = FileManager::readFile(fileName);

  // pthread_t keysThread;
  // pthread_create(&keysThread, NULL, &manageKeys, (void *)this);
  string cursor_display;

  index = 0;
  cursor_x = 0;
  cursor_y = 0;

  while (true)
  {
    getmaxyx(stdscr, max_y, max_x);
    attron(COLOR_PAIR(NORMAL));
    mvprintw(0, 0, "%s", Base::buffer.c_str());
    attron(COLOR_PAIR(CURSOR_COLOR_PAIR));
    cursor_display = buffer.substr(index, 1).c_str();
    if (cursor_display == "\n")
    {
      cursor_display = " ";
    }
    mvprintw(cursor_y, cursor_x, cursor_display.c_str());
    attroff(COLOR_PAIR(CURSOR_COLOR_PAIR));
    mvprintw(max_y - 1, 0, (mode == NORMAL_MODE ? "--NORMAL--" : "--INSERT--"));
    refresh();
    char ch = getch();
    if (mode == NORMAL_MODE)
    {
      switch (ch)
      {
      case 'q':
        endwin();
        exit(0);
        break;
      case 'i':
        mode = INSERT_MODE;
        break;
      case 'l':
        moveCursorRight();
        break;
      case 'h':
        moveCursorLeft();
        break;
      case 'j':
        lineDown();
        break;
      case 'k':
        lineUp();
        break;
      }
    }
    else if (mode == INSERT_MODE)
    {
      switch (ch)
      {
      case 27: // ESC
        mode = NORMAL_MODE;
        break;
      case 7: // backspace
        if (index > 0)
        {
          buffer.erase(index - 1, 1);
          index--;
          cursor_x--;
          prevXPosition = cursor_x;
        }
        break;
      case '\n':
        buffer.insert(index, "\n");
        index++;
        lineDown();
        break;
      case '\t':
        buffer.insert(index, "    ");
        index += 4;
        cursor_x += 4;
        break;
      case 2: // arrow down
        lineDown();
        break;
      case 5: // arrow right
        moveCursorRight();
        break;
      case 4: // arrow left
        moveCursorLeft();
        break;
      case 3: // arrow up
        lineUp();
        break;
      default:
        buffer.insert(index, 1, ch);
        index++;
        cursor_x++;
        break;
      }
    }
  }
}

void Base::moveLines(int n)
{
  if (n > 0)
  {
    for (unsigned int i = 0; i < n; i++)
    {
      lineUp();
    }
  }
  else if (n < 0)
  {
    for (unsigned int i = 0; i < abs(n); i++)
    {
      lineDown();
    }
  }
}

void Base::moveCursorLeft()
{
  if (cursor_x > 0)
  {
    cursor_x--;
    index--;
    prevXPosition = cursor_x;
  }
}

void Base::moveCursorRight()
{
  unsigned int lineLength = getLineLength(line);
  if (cursor_x + 1 < (mode == INSERT_MODE ? lineLength + 1 : lineLength))
  {
    cursor_x++;
    index++;
    prevXPosition = cursor_x;
  }
}

unsigned int Base::getLineLength(unsigned int line)
{
  unsigned int lineLength = 0;
  for (unsigned int i = 0; i < buffer.length(); i++)
  {
    if (buffer.substr(i, 1)[0] == '\n')
    {
      if (line == 0)
      {
        return lineLength;
      }
      line--;
      lineLength = 0;
    }
    else
    {
      lineLength++;
    }
  }
  return lineLength;
}

unsigned int Base::getNumOfLines()
{
  unsigned int numOfLines = 0;
  for (unsigned int i = 0; i < buffer.length(); i++)
  {
    if (buffer.substr(i, 1)[0] == '\n')
    {
      numOfLines++;
    }
  }
  return numOfLines;
}

unsigned int Base::getPosition()
{
  unsigned int pos = 0;
  char current = buffer.substr(pos, 1).c_str()[0];
  int times = line;

  while (times > 0)
  {
    if (current == '\n')
    {
      times--;
    }
    pos++;
    if (pos >= buffer.length())
    {
      break;
    }
    current = buffer.substr(pos, 1).c_str()[0];
  }

  pos += cursor_x;
  return pos;
}

void Base::lineUp()
{
  unsigned int lineLenght;
  if (line > 0)
  {
    cursor_y--;
    line--;
  }
  lineLenght = getLineLength(line);
  if (cursor_x > lineLenght)
  {
    lineLenght = lineLenght;
    if (lineLenght > 0)
    {
      cursor_x = lineLenght - 1;
    }
    else
    {
      cursor_x = 0;
    }
  }

  if (prevXPosition <= lineLenght)
  {
    cursor_x = prevXPosition;
  }

  index = getPosition();
}

void Base::lineDown()
{
  unsigned int lineLenght;
  if (line + 1 < getNumOfLines())
  {
    cursor_y++;
    line++;
  }

  lineLenght = getLineLength(line);
  if (cursor_x > lineLenght)
  {
    if (lineLenght > 0)
    {
      cursor_x = lineLenght - 1;
    }
    else
    {
      cursor_x = 0;
    }
  }

  if (prevXPosition <= lineLenght)
  {
    cursor_x = prevXPosition;
  }

  index = getPosition();
}

void Base::init()
{
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  refresh();

  init_pair(NORMAL, COLOR_WHITE, COLOR_BLACK);
  init_pair(CURSOR_COLOR_PAIR, COLOR_BLACK, COLOR_RED);
}

void *manageKeys(void *threadId)
{
  while (true)
  {
    int key = getch();
    switch (key)
    {
    case 'q':
      endwin();
      break;

    default:
      break;
    }
  }
}
int main(int argc, char *argv[])
{
  Base base;

  base.open(argv[1]);

  return 0;
}
