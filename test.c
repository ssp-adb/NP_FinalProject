#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#define ROW 51
#define COL 51

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#define WHITE 1
#define BLACK 2
#define GREEN 3
#define RED 4

struct Snake {
	int tail = 0, head = 1;
	int pos_i[2500] = {1, 1}, pos_j[2500] = {1, 2};
	int length = 2;
	char direction;
};

void init_data(char data[][COL+1]);
void draw(char data[][COL+1]);
char get_input(char curr);
int update_snake_pos(Snake* snake, char data[][COL+1]);

int main() {
	Snake snake;
	snake.direction = RIGHT;
	
	char data[ROW+1][COL+1];
	init_data(data);
	
	initscr();
    curs_set(0);
	raw();
	noecho();
	nodelay(stdscr, TRUE);
	start_color();
	use_default_colors();
	init_pair(WHITE, -1, COLOR_WHITE);
	init_pair(BLACK, -1, COLOR_BLACK);
	init_pair(GREEN, -1, COLOR_GREEN);
	init_pair(RED, -1, COLOR_RED);
	
	clear();
	draw(data);
	refresh();
	
	while(1) {
		if((snake.direction = get_input(snake.direction)) == 'q')
			break;
		if(update_snake_pos(&snake, data) < 0)
			break;
		clear();
		draw(data);
		//printw("The snake direction: %c\n", snake.direction);
		refresh();
		usleep(200000);
	}
	endwin();
	return 0;
}

void init_data(char data[][COL+1]) {
	for(int i = 0; i <= ROW; i++) {
		for(int j = 0; j <= COL; j++) {
			if(i == 0 || j == 0 || i == ROW || j == COL)
				data[i][j] = '#';
			else
				data[i][j] = '.';
		}
	}
	data[1][1] = 'X';
	data[1][2] = 'O';
}

void draw_data(char data[][COL+1]) {
    for(int i = 0; i <= ROW; i++) {
        for(int j = 0; j <= COL; j++) {
            printw("%c", data[i][j]);
        }
        printw("\n");
    }
    return;
}

void draw(char data[][COL+1]) {
    for(int i = 0; i <= ROW; i++) {
        for(int j = 0; j <= COL; j++) {
			char temp = data[i][j];
			int color;
			if(temp == '.')
				color = BLACK;
			else if(temp == 'O')
				color = RED;
			else if(temp == 'X')
				color = GREEN;
			else if(temp == '#')
				color = WHITE;
			
			attron(COLOR_PAIR(color));
			printw("  ");
			attroff(COLOR_PAIR(color));
        }
        printw("\n");
    }
    return;
}

char get_input(char curr) {
    char input, last = curr;
    while(1) {
        input = getch();
        if(input == ERR) // no input, return last 
            return last;
        else if(input != last) { // input set a new direction
            if(input == 'a' || input == 'w' || input == 's' || input == 'd' || input == 'q')
                return input;
        }
        // input didn't set a new direction, continue the loop to clean the input
    }
}

int update_snake_pos(Snake* snake, char data[][COL+1]) {
	int oldHead = snake->head;
	int oldTail = snake->tail;
	int newHead = (oldHead+1)%2500;
	int newTail = (oldTail+1)%2500;
	switch(snake->direction) {
		case UP:
			snake->pos_i[newHead] = snake->pos_i[oldHead]-1;
			snake->pos_j[newHead] = snake->pos_j[oldHead];
			break;
		case DOWN:
			snake->pos_i[newHead] = snake->pos_i[oldHead]+1;
			snake->pos_j[newHead] = snake->pos_j[oldHead];
			break;
		case LEFT:
			snake->pos_i[newHead] = snake->pos_i[oldHead];
			snake->pos_j[newHead] = snake->pos_j[oldHead]-1;
			break;
		case RIGHT:
			snake->pos_i[newHead] = snake->pos_i[oldHead];
			snake->pos_j[newHead] = snake->pos_j[oldHead]+1;
			break;
	}
	
	// collision detect
	if(data[snake->pos_i[newHead]][snake->pos_j[newHead]] != '.')
		return -1;
	
	// Update head to data
	data[snake->pos_i[newHead]][snake->pos_j[newHead]] = 'O';
	
	// Updata old head to body
	data[snake->pos_i[oldHead]][snake->pos_j[oldHead]] = 'X';
	
	// Updata tail to data
	data[snake->pos_i[oldTail]][snake->pos_j[oldTail]] = '.';
	
	snake->head = newHead;
	snake->tail = newTail;
	
	return 0;
}