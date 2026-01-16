// Written by Rob Probin and Emily Probin, 3 Jan 2026
// © Copyright 2025, Rob Probin and Emily Probin
// こんにちは
// Compile with: 
//               g++ simple_rogue_term.cpp -o simple_rogue_term -std=c++23
// or if you have n older compiler, this works as well:
//               g++ simple_rogue_term.cpp -o simple_rogue_term -std=c++20
#include <cstdlib>
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


void print_at(int x, int y, const string& c)
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

class PrintableObjectInterface {
public:
	virtual void draw() = 0;	// pure virtual function makes this class an abstract base class
private:
	// no data in an interface
};

class MazeElement {
public:
	void draw(int x, int y) { 
		print_at(x , y, character);
	}
	void set_graphic(const string& c) {character = c;}
	const string& get_graphic() const {return character;}
private:
	string character;
};


class Maze {
public:
	void draw_all() { 
		int rows = map.size();
		if(rows == 0) { return; }
		for(int r=0; r < rows; r++)
		{
			int columns = map[r].size();
			for(int c=0; c < columns; c++)
			{
				map[r][c].draw(c+1, r+1);
			}
		}
	}
	bool valid_move(int x, int y) const {
		// check if out of bounds
		// check if wall is there 
		
		if (x > map[0].size() || x < 1 || y > map.size() || y < 1) { // check not going out of bounds
			return false;
		}
		if (map[y-1][x-1].get_graphic() != " ") { // check moving into clear space
			return false;
		}
		return true;
	}
	void clear_cell(int x, int y) {
		map[y-1][x-1].set_graphic(" ");
	}
	Maze(int rows, int columns) : map(rows, vector<MazeElement>(columns)) { create_maze(); }
private:
	void create_maze();
	vector<vector<MazeElement>> map;
};

void Maze::create_maze() {
	int rows = map.size();
		if(rows == 0) { return; }
		for(int r=0; r < rows; r++)
		{
			int columns = map[r].size();
			for(int c=0; c < columns; c++)
			{
				if (std::rand()%10 == 0) { map[r][c].set_graphic("#"); }
				else {map[r][c].set_graphic(" ");}
			}
		}
}

class Player : public  PrintableObjectInterface{
public:
	void move(int delta_x, int delta_y, const Maze& maze)
	{
		int new_x = x + delta_x;
		int new_y = y + delta_y;
		if(maze.valid_move(new_x, new_y))
		{
			x = new_x; y = new_y;
		}
	}
	void draw();
	Player(int start_x, int start_y);
private:
	int x;
	int y;
};

void Player::draw()
{
	print_at(x, y, "@");
}

Player::Player(int start_x, int start_y) : x(start_x), y(start_y) {
}





class Coordinates {
	int x;
	int y;
};



class PrintableObject : public PrintableObjectInterface { // 
public:
	void draw() = 0;		// still virtual, because of inheritance (and same 'signature', i.e. parameters)
	//void draw(int x);	// function overloading
	//void operator+(int x, int y); // operator overloading allows printable objects to be added together, whatever that means :-)
private:
	Coordinates position;
	char representing_char;
};

class AnimateObject : public PrintableObject {
public:
	AnimateObject();
	~AnimateObject();
private:
	
};

// constructor
AnimateObject::AnimateObject()
{
}

AnimateObject::~AnimateObject()
{
}






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


	const int player_start_x = 10;
	const int player_start_y = 10;
	Player player(player_start_x, player_start_y);
	Maze map(TERM_HEIGHT-2, TERM_WIDTH-2);
	map.clear_cell(10,10);
	bool running = true;
	while(running) {
		map.draw_all();
		player.draw();
		at(0, TERM_HEIGHT);
		std::cout << std::flush;

		int keyval = key();
		switch(keyval) {
			case 'w':
				player.move(0,-1,map);
				break;
			case 's':
				player.move(0,1,map);
				break;
			case 'a':
				player.move(-1,0,map);
				break;
			case 'd':
				player.move(1,0,map);
				break;
			case ' ':
				running = false;
				break;
			default:
				std::cout << "Unrecognised key " << keyval << std::endl;
				running = false;
				break;
			}
			
	};

	ttyrestore();
}

