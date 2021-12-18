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

  void scrollPage(int direction);

  unsigned int getPosition();
  unsigned int getNumOfLines();
  unsigned int getLineLength(unsigned int line);

  unsigned int x, y;
  unsigned int position;
  unsigned int max_x_position = 0;

  int max_y, max_x;

  unsigned int display_start_line = 0;
  unsigned int y_offset = 0;

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

  position = 0;
  x = 0;
  y = 0;

  while (true)
  {
    getmaxyx(stdscr, max_y, max_x);
    attron(COLOR_PAIR(NORMAL));
    mvprintw(0, 0, "%s", Base::buffer.substr(display_start_line).c_str());
    attron(COLOR_PAIR(CURSOR_COLOR_PAIR));
    cursor_display = buffer.substr(position, 1).c_str();
    if (cursor_display == "\n")
    {
      cursor_display = " ";
    }
    mvprintw(y, x, cursor_display.c_str());
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
      case 'g':
        scrollPage(2);
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
        if (position > 0)
        {
          buffer.erase(position - 1, 1);
          position--;
          x--;
          max_x_position = x;
        }
        break;
      case '\n':
        buffer.insert(position, "\n");
        position++;
        lineDown();
        break;
      case '\t':
        buffer.insert(position, "    ");
        position += 4;
        x += 4;
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
        buffer.insert(position, 1, ch);
        position++;
        x++;
        log(std::to_string(ch));
        break;
      }
    }
  }
}

void Base::scrollPage(int direction)
{
  unsigned int num_of_char_skipped = 0;
  char current;
  unsigned int lines = 0;

  while (lines < direction)
  {
    current = buffer.at(y_offset);
    if (current == '\n')
    {
      lines++;
    }
    num_of_char_skipped++;
    position++;
    y_offset++;
  }

  y += direction;

  display_start_line += num_of_char_skipped;
}

void Base::moveCursorLeft()
{
  if (x > 0)
  {
    x--;
    position--;
    max_x_position = x;
  }
}

void Base::moveCursorRight()
{
  unsigned int lineLength = getLineLength(y);
  if (x + 1 < (mode == INSERT_MODE ? lineLength + 1 : lineLength))
  {
    x++;
    position++;
    max_x_position = x;
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
  int times = y;

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

  pos += x;
  return pos;
}

void Base::lineUp()
{
  unsigned int lineLenght;
  if (y > 0)
  {
    y--;
  }
  lineLenght = getLineLength(y);
  if (x > lineLenght)
  {
    lineLenght = lineLenght;
    if (lineLenght > 0)
    {
      x = lineLenght - 1;
    }
    else
    {
      x = 0;
    }
  }

  if (max_x_position <= lineLenght)
  {
    x = max_x_position;
  }

  position = getPosition();
}

void Base::lineDown()
{
  unsigned int lineLenght;
  if (y + 1 < getNumOfLines())
  {
    y++;
  }

  lineLenght = getLineLength(y);
  if (x > lineLenght)
  {
    if (lineLenght > 0)
    {
      x = lineLenght - 1;
    }
    else
    {
      x = 0;
    }
  }

  if (max_x_position <= lineLenght)
  {
    x = max_x_position;
  }

  position = getPosition();
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
