#include "client.h"

void init_screen() {
	initscr();
	// hide curses
	curs_set(0);
	// disable line buffering
	raw();
	// disable echo
	noecho();
	// disable blocking
	nodelay(stdscr, TRUE);
	// start color
	start_color();
	// use default color
	use_default_colors();
	// init color pair
	init_pair(WHITE, -1, COLOR_WHITE);
	init_pair(BLACK, -1, COLOR_BLACK);
	init_pair(GREEN, -1, COLOR_GREEN);
	init_pair(RED, -1, COLOR_RED);
	init_pair(BLUE, -1, COLOR_BLUE);
    init_pair(YELLOW, -1, COLOR_YELLOW);
    init_pair(CYAN, -1, COLOR_CYAN);
	init_pair(T_WHITE, COLOR_WHITE, -1);
	init_pair(T_RED, COLOR_RED, -1);
	init_pair(GAME_OVER_COLOR, COLOR_WHITE, COLOR_RED);

	return;
}

char get_input(char last) {
    char input;
    while(1) {
        input = getch();
        if(input == ERR) // no input 
            return 'N';
        else if(input != last) { // input set a new direction
            if(input == 'a' && last != 'd' 
				|| input == 'w' && last != 's'
				|| input == 's' && last != 'w'
				|| input == 'd' && last != 'a'
				|| input == 'q')
                return input;
        }
        // input didn't set a new direction, continue the loop to clean the input
    }
}

void draw(char data[][COL+1], int fruit_eaten) {
	//clear vscreen
	clear();

	// print "OBJECTIVE: Kill the other snake or Eat 120 fruits first\n"
	attron(COLOR_PAIR(T_WHITE));
	printw("OBJECTIVE: ");
	attron(COLOR_PAIR(T_RED));
	printw("Kill the other snake");
	attron(COLOR_PAIR(T_WHITE));
	printw(" or ");
	attron(COLOR_PAIR(T_RED));
	printw("Eat 120 fruits first\n");
	attroff(COLOR_PAIR(T_RED));

	// print "Fruits eaten: %d\n"
	attron(COLOR_PAIR(T_WHITE));
	printw("Fruits eaten: ");
	attron(COLOR_PAIR(T_RED));
	printw("%d\n", fruit_eaten);
	attroff(COLOR_PAIR(T_RED));

	// print Game board
    for(int i = 0; i <= ROW; i++) {
        for(int j = 0; j <= COL; j++) {
			char temp = data[i][j];
			int color;
            switch(temp) {
                case '.':
                    color = BLACK;
                    break;
                case 'O':
                    color = BLUE;
                    break;
                case 'X':
                    color = GREEN;
                    break;
                case 'o':
                    color = CYAN;
                    break;
                case 'x':
                    color = YELLOW;
                    break;
                case 'F':
                    color = RED;
                    break;
                case '#':
                    color = WHITE;
                    break;
            }
			
			attron(COLOR_PAIR(color));
			printw("  ");
			attroff(COLOR_PAIR(color));
        }
        printw("\n");
    }

	// refresh to screen
	refresh();
	
    return;
}

void draw_game_over(int game_over_type) {
	attron(COLOR_PAIR(GAME_OVER_COLOR));
	if(game_over_type == -3)
		mvprintw(27, 48, "Equal!");
	else if(game_over_type == -1)
		mvprintw(27, 48, "You win!");
	else if(game_over_type == -2)
		mvprintw(27, 48, "You lose!");

    mvprintw(28, 48, "Press any key to exit...");
	attroff(COLOR_PAIR(GAME_OVER_COLOR));
	refresh();
	// block until user input
	nodelay(stdscr, FALSE);
	getch();
	endwin();
}