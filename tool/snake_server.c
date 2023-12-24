#include "snake_server.h"

// Call this function for initialization of data
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
    return;
}

// Call this function for initialize 2 snake
// snake1: head = 'O', body = 'X'
// snake2: head = 'o', body = 'x'
void Init_snake(Snake* snake1, Snake* snake2, char data[][COL+1]) {
    // int i, int j, char direction, int length, char head_char, char body_char, char data[][COL+1]
	// init snake1
    *snake1 = init_snake(1, 3, RIGHT, 3, 'O', 'X', data);
    // init snake2
    *snake2 = init_snake(ROW-1, COL-3, LEFT, 3, 'o', 'x', data);
	return;
}

// Call this function for snake pos update and collision dectect
// return 0 if no collision, 1 if snake1 win, 2 if snake2 win, 3 if equal
int update_snake(Snake* snake1, Snake* snake2, char data[][COL+1]) {
    int temp1 = 0;
    int temp2 = 0;

    temp1 = Update_snake_pos(snake1, snake2, data);
    temp2 = Collision_snake(snake1, snake2, data);

    if((temp1 == 1 && temp2 == 2) || (temp1 == 2 && temp2 == 1) || temp1 == 3 || temp2 == 3)
        return 3;
    else if(temp1 == 1 || temp2 == 1)
        return 1;
    else if(temp1 == 2 || temp2 == 2)
        return 2;
    else
        return 0;
}

// Call this function to check if snake1 or snake2 win by eating fruits
// return 0 if no one win, 1 if snake1 win, 2 if snake2 win, 3 if equal
int Check_win(Snake* snake1, Snake* snake2) {
    bool snake1_win = 0;
    bool snake2_win = 0;

    if(check_win(snake1))
        snake1_win = 1;
    if(check_win(snake2))
        snake2_win = 1;
    
    if(snake1_win && snake2_win)
        return 3;
    else if(snake1_win)
        return 1;
    else if(snake2_win)
        return 2;
    else
        return 0;
}