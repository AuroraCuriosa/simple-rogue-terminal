// Written by Rob Probin and Emily Probin, 3 Jan 2026
// © Copyright 2025, Rob Probin and Emily Probin
// こんにちは
#include <iostream>
#include <string>
#include <sstream>
#include <vector> 
using std::string;
using std::stringstream;
using std::vector;
#include <stdio.h>		// for getchar (easier than std:cin)

#include <termios.h> // POSIX library mmodify terminal to get single characters keys
#include <unistd.h> // for STDIN_FILENO

// for ANSI control character see:
// https://en.wikipedia.org/wiki/ANSI_escape_code
#define CSI "\x1b[" // Control sequence indicator common to several ANSI/VT100 control commands - Escape [ starts

// This is the size of the window. We could as the terminal size it is, but we don't 
// bother for this simple program. Make sure the window is big enough to display the frame.
const int TERM_WIDTH = 80;
const int TERM_HEIGHT = 25;


// =======================================================================================
//     OUTPUT and INPUT utilities
// =======================================================================================

void cls()
{
    std::cout << CSI "2J";
}

void at(int x, int y)
{
	int row = y+1;
	int column = x+1;
    // std::stringstream s; 

    // CSI n ; m H       
    // This is CUP or Cursor Position
    // Moves the cursor to row n, column m. 
    // The values are 1-based, and default to 1 (top left corner) if omitted. 
    // A sequence such as CSI ;5H is a synonym for CSI 1;5H as well 
    // as CSI 17;H is the same as CSI 17H and CSI 17;1H
    std::cout << CSI << row << ";" << column << "H";
}



void print_at(int x, int y, string c)
{
    at(x, y);
    std::cout << c;
}


void print_box(int x, int y, int width, int height)
{
	
	// prepare first and last line
	std::string line(width, '-'); // s == "----"
	line[0] = '+';
	line[width-1] = '+';
	
	// prepare middle lines
	std::string midline(width, ' ');
	midline[0] = '|';
	midline[width-1] = '|';
	
	// print first line
	print_at(x, y, line);
	y ++;
	
	// print middle lines
	for(int i = 0; i < (height-2); i++)
	{
		print_at(x, y, midline);
		y ++;
	}
	// print last line
	print_at(x, y, line);

	// debug only
	//std::cout << line.size() << " " << midline.size() << std::endl;
}

// https://stackoverflow.com/questions/4293355/how-to-detect-key-presses-in-a-linux-c-gui-program-without-prompting-the-user

// https://stackoverflow.com/questions/77643347/non-blocking-keyboard-reading-on-macos-in-c

struct termios tio_save;

void terminal_setup()
{
	struct termios tio;

    tcgetattr(fileno(stdin), &tio);
    tio_save = tio;

    //cfmakeraw(&tio);
	tio.c_lflag &= ~ICANON; // turning off canonical mode makes input unbuffered
	tio.c_lflag &= ~ECHO;	// turn off echo

	// see http://www.unixwiz.net/techtips/termios-vmin-vtime.html
    tio.c_cc[VMIN] = 1;		// minimum 1 character
    tio.c_cc[VTIME] = 0;

    tcsetattr(fileno(stdin), TCSANOW, &tio);
}

void ttyrestore()
{
    tcsetattr(fileno(stdin), TCSAFLUSH,&tio_save);
}


int key()
{   	
   	// Don't use getchar. Don't intermix raw tty input with stdio. Use read instead.
   	// https://stackoverflow.com/questions/77643347/non-blocking-keyboard-reading-on-macos-in-c
	//int val = getchar();
	
	unsigned char buf[1];
    int len;

    len = read(STDIN_FILENO,buf,1);
    // ESC [ A       up arrow 

    if (len > 0) {
        len = buf[0];
    }

	//stringstream ss;
	//ss << std::to_string(len);
	//print_at(6,6,ss.str());

    return len;
}

// =======================================================================================
//     GAME CODE
// =======================================================================================

class Player {
public:
	void move(int delta_x, int delta_y) { x += delta_x; y += delta_y; }
	void draw();
	Player();
private:
	int x;
	int y;
};


void Player::draw()
{
	print_at(x, y, "😀");
}

Player::Player() : x(10), y(10) {
}


class MazeElement {
public:
	void draw() { }
private:
	string x;
};

class Maze {
public:
	void draw() { }
	Maze(int rows, int columns) : map(rows, vector<MazeElement>(columns)) {}
private:
	vector<vector<MazeElement>> map;
};






// =======================================================================================
//     main loop
// =======================================================================================

int main()
{
	terminal_setup();
    cls();
    print_box(0,0, TERM_WIDTH, TERM_HEIGHT);
    
    // see https://en.wikipedia.org/wiki/Emoticons_(Unicode_block)
    // https://stackoverflow.com/questions/12015571/how-to-print-unicode-character-in-c
    //print_at(1,1,"@");
    //print_at(2,2, "😀");
    //print_at(3,3, "\U0001F603");
    //print_at(4,4, "\u0444"); // CYRILLIC CAPITAL LETTER EF
    //print_at(5,5, "Hello"    ",World");
	//at(0, TERM_HEIGHT);

	std::cout << std::flush;
	
	//int val = key();
	//auto s = std::to_string(val);
	//print_at(7,7,s);


	Player player;
	Maze map(TERM_HEIGHT, TERM_WIDTH);
	bool running = true;
	while(running) {
		int keyval = key();
		switch(keyval) {
			case 'w':
				player.move(0,-1);
				break;
			case 's':
				player.move(0,1);
				break;
			case 'a':
				player.move(-1,0);
				break;
			case 'd':
				player.move(1,0);
				break;
			case ' ':
				running = false;
				break;
			default:
				running = false;
				break;
			}
			
			
		player.draw();
		at(0, TERM_HEIGHT);
		std::cout << std::flush;
	};

	ttyrestore();
}

