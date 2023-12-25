#ifndef SNAKE_SERVER_H
#define SNAKE_SERVER_H

#include "server_component.h"
#include "snake_client.h"

void init_data(char data[][COL+1]);
void Init_snake(Snake* snake1, Snake* snake2, char data[][COL+1]);
int update_snake(Snake* snake1, Snake* snake2, char data[][COL+1]);
int Check_win(Snake* snake1, Snake* snake2);

#endif