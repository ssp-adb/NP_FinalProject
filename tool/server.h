#ifndef SERVER_H
#define SERVER_H

#include "./component/server_component.h"

void Init_snake(Snake* snake1, Snake* snake2, char data[][COL+1]);
int Update_snake_pos(Snake* snake1, Snake* snake2, char data[][COL+1]);
int Collision_snake(Snake* snake1, Snake* snake2, char data[][COL+1]);
int Check_win(Snake* snake1, Snake* snake2);

#endif