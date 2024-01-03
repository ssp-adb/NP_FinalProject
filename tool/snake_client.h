#ifndef SNAKE_CLIENT_H
#define SNAKE_CLIENT_H

#include "snake_component.h"

void init_screen();
char get_input(char last);
void draw(char data[][COL+1], int fruit_eaten1, int fruit_eaten2, int player);
void draw_game_over(int game_over_type);

#endif