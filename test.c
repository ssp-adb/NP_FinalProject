#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#define ROW 51
#define COL 51
#define MAX_SNAKE_LENGTH 123

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#define WHITE 1
#define BLACK 2
#define GREEN 3
#define RED 4
#define BLUE 5
#define T_WHITE 6
#define T_RED 7
#define GAME_OVER_COLOR 8

typedef struct {
	int tail, head;
	int pos_i[MAX_SNAKE_LENGTH], pos_j[MAX_SNAKE_LENGTH];
	int length;
	char direction;
	char head_char, body_char;
}Snake;

void init_data(char data[][COL+1]);
void init_screen(char data[][COL+1]);
void draw(char data[][COL+1], Snake snake);
char get_input(char curr);
int update_snake_pos(Snake* snake, char data[][COL+1]);
bool collision_snake(Snake* snake1, Snake* snake2, char data[][COL+1]);
void generate_fruit(char data[][COL+1]);
void draw_data(char data[][COL+1]);
Snake init_snake(int i, int j, char direction, int length, char head_char, char tail_char, char data[][COL+1]);
void end_game(int end_game_type);
bool check_win(Snake* snake);

int main() {
	srand(time(NULL));
	
	char data[ROW+1][COL+1] = {0};
	int end_game_type = 0; // SERVER 0: equal, 1: snake1 win, 2: snake2 win, CLIENT 0: equal, 1: win, 2: lose
	Snake snake1 = init_snake(1, 3, RIGHT, 3, 'O', 'X', data); // init snake head at (1, 3) with length 3
	init_data(data);
	init_screen(data);
	draw(data, snake1);
	
	bool game_over = false;
	while(!game_over) {
		if((snake1.direction = get_input(snake1.direction)) == 'q')
			break;
		if(update_snake_pos(&snake1, data) < 0) {
			end_game_type = 2;
			break;
		}
		if(collision_snake(&snake1, &snake1, data)) {
			end_game_type = 2;
			break;
		}
		if(check_win(&snake1)) {
			end_game_type = 1;
			break;
		}
		draw(data, snake1);
		usleep(120000);
	}
	draw(data, snake1);
	end_game(end_game_type);
	
	exit(0);
}

Snake init_snake(int i, int j, char direction, int length, char head_char, char body_char, char data[][COL+1]) {
	Snake snake;
	snake.direction = direction;
	snake.length = length;
	snake.head = length-1;
	snake.tail = 0;
	snake.head_char = head_char;
	snake.body_char = body_char;

	for(int k = snake.head, l = 0; k >= snake.tail; k--, l++) {
		snake.pos_i[k] = i;
		snake.pos_j[k] = j-l;
		data[i][j-l] = body_char;
	}
	data[i][j] = head_char;
	return snake;
}

void init_data(char data[][COL+1]) {
	for(int i = 0; i <= ROW; i++) {
		for(int j = 0; j <= COL; j++) {
			if(i == 0 || j == 0 || i == ROW || j == COL)
				data[i][j] = '#';
			else if(data[i][j] == 0)
				data[i][j] = '.';
		}
	}

	// generate 15 fruits
	for(int i = 0; i < 15; i++) {
		generate_fruit(data);
	}
}

void init_screen(char data[][COL+1]) {
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
	init_pair(T_WHITE, COLOR_WHITE, -1);
	init_pair(T_RED, COLOR_RED, -1);
	init_pair(GAME_OVER_COLOR, COLOR_WHITE, COLOR_RED);

	return;
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

void draw(char data[][COL+1], Snake snake) {
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
	printw("%d\n", snake.length-3);
	attroff(COLOR_PAIR(T_RED));

	// print Game board
    for(int i = 0; i <= ROW; i++) {
        for(int j = 0; j <= COL; j++) {
			char temp = data[i][j];
			int color;
			if(temp == '.')
				color = BLACK;
			else if(temp == snake.head_char)
				color = BLUE;
			else if(temp == 'F')
				color = RED;
			else if(temp == snake.body_char)
				color = GREEN;
			else if(temp == '#')
				color = WHITE;
			
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

char get_input(char curr) {
    char input, last = curr;
    while(1) {
        input = getch();
        if(input == ERR) // no input, return last 
            return last;
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

int update_snake_pos(Snake* snake, char data[][COL+1]) {
	int oldHead = snake->head;
	int oldTail = snake->tail;
	int newHead = (oldHead+1)%MAX_SNAKE_LENGTH;
	int newTail;

	bool fruit_eaten = false;
	bool hit_wall = false;

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
	// hit wall
	if(data[snake->pos_i[newHead]][snake->pos_j[newHead]] == '#') 
		hit_wall = true;
	// hit fruit
	if(data[snake->pos_i[newHead]][snake->pos_j[newHead]] == 'F')
		fruit_eaten = true;
	//

	// Updata tail to data, if fruit is eaten, tail is not updated
	if(fruit_eaten) {
		snake->length++;
		generate_fruit(data);
		newTail = oldTail;
	} else {
		data[snake->pos_i[oldTail]][snake->pos_j[oldTail]] = '.';
		newTail = (oldTail+1)%MAX_SNAKE_LENGTH;
	}

	// Updata old head to body
	data[snake->pos_i[oldHead]][snake->pos_j[oldHead]] = 'X';

	// Update head
	if(!hit_wall)
		data[snake->pos_i[newHead]][snake->pos_j[newHead]] = 'O';
	else
		return -1;
	

	snake->head = newHead;
	snake->tail = newTail;
	
	return 0;
}

// check if snake1's head hit snake2's body
bool collision_snake(Snake* snake1, Snake* snake2, char data[][COL+1]) {
	// start from snake 2's tail to snake 2's head-1
	for(int i = snake2->tail; i != snake2->head; i = (i+1)%MAX_SNAKE_LENGTH) {
		if(snake1->pos_i[snake1->head] == snake2->pos_i[i] && snake1->pos_j[snake1->head] == snake2->pos_j[i]) {
			// modify data to show the collision
			data[snake1->pos_i[snake1->head]][snake1->pos_j[snake1->head]] = 'X';
			return 1;
		}
	}
	return 0;
}

// generate a fruit at a random position
void generate_fruit(char data[][COL+1]) {
	int i = rand()%(ROW-1)+1; 
	int j = rand()%(COL-1)+1;
	while(data[i][j] != '.') {
		i = rand()%(ROW-1)+1;
		j = rand()%(COL-1)+1;
	}
	data[i][j] = 'F';
	return;
}

bool check_win(Snake* snake) {
	if(snake->length == 123)
		return true;
	return false;
}

void end_game(int end_game_type) {
	attron(COLOR_PAIR(GAME_OVER_COLOR));
	if(end_game_type == 0)
		mvprintw(27, 48, "Equal!");
	else if(end_game_type == 1)
		mvprintw(27, 48, "You win!");
	else if(end_game_type == 2)
		mvprintw(27, 48, "You lose!");
	attroff(COLOR_PAIR(GAME_OVER_COLOR));
	refresh();
	// block until user input
	nodelay(stdscr, FALSE);
	getch();
	endwin();
	exit(0);
}