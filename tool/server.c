#include "server.h"

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

// Call this function to update snake position in data, and check collision with wall
// return 0 if no collision, 1 if snake1 win, 2 if snake2 win, 3 if equal
int Update_snake_pos(Snake* snake1, Snake* snake2, char data[][COL+1]) {
    int snake1_collision = 0;
    int snake2_collision = 0;

    // update snake1
    if(update_snake_pos(snake1, data) < 0)
        snake1_collision = 1;
    // update snake2
    if(update_snake_pos(snake2, data) < 0)
        snake2_collision = 1;
    
    if(snake1_collision && snake2_collision)
        return 3;
    else if(snake1_collision)
        return 2;
    else if(snake2_collision)
        return 1;
    else
        return 0;
}

// Call this function to check for collision of all snakes with itself or other snake
// Call after Update_snake_pos()
// return 0 if no collision, 1 if snake1 win, 2 if snake2 win, 3 if equal
int Collision_snake(Snake* snake1, Snake* snake2, char data[][COL+1]) {

    bool collision_snake1 = 0;
    bool collision_snake2 = 0;

    // check if snake1's head and snake2's head hit each other
    if(snake1->pos_i[snake1->head] == snake2->pos_i[snake2->head] && snake1->pos_j[snake1->head] == snake2->pos_j[snake2->head])
        return 3;

    // snake1 lose
    // check if snake1's head hit snake2's body
    if(collision_snake(snake1, snake2, data))
        collision_snake1 = 1;
    // check if snake1's head hit snake1's body
    if(collision_snake(snake1, snake1, data))
        collision_snake1 = 1;
    //

    // snake2 lose
    // check if snake2's head hit snake1's body
    if(collision_snake(snake2, snake1, data))
        collision_snake2 = 1;
    // check if snake2's head hit snake2's body
    if(collision_snake(snake2, snake2, data))
        collision_snake2 = 1;
    //

    if(collision_snake1 && collision_snake2)
        return 3;
    else if(collision_snake1)
        return 2;
    else if(collision_snake2)
        return 1;
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