#ifndef SERVER_COMPONENT_H
#define SERVER_COMPONENT_H

#include "snake_component.h"

typedef struct {
	int tail, head;
	int pos_i[MAX_SNAKE_LENGTH], pos_j[MAX_SNAKE_LENGTH];
	int length;
	char direction;
	char head_char, body_char;
} Snake;

void init_data(char data[][COL+1]);
Snake init_snake(int i, int j, char direction, int length, char head_char, char body_char, char data[][COL+1]);
int update_snake_pos(Snake* snake, char data[][COL+1]);
bool collision_snake(Snake* snake1, Snake* snake2, char data[][COL+1]);
void generate_fruit(char data[][COL+1]);
bool check_win(Snake* snake);

#endif