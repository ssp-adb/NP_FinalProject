#include "server_component.h"

Snake init_snake(int i, int j, char direction, int length, char head_char, char body_char, char data[][COL+1]) {
	Snake snake;
	snake.direction = direction;
	snake.length = length;
	snake.head = length-1;
	snake.tail = 0;
	snake.head_char = head_char;
	snake.body_char = body_char;

	if(direction == RIGHT) {
		for(int k = snake.head, l = 0; k >= snake.tail; k--, l++) {
			snake.pos_i[k] = i;
			snake.pos_j[k] = j-l;
			data[i][j-l] = body_char;
		}
	} else {
		for(int k = snake.head, l = 0; k >= snake.tail; k--, l++) {
			snake.pos_i[k] = i;
			snake.pos_j[k] = j+l;
			data[i][j+l] = body_char;
		}
	}
	
	data[i][j] = head_char;
	return snake;
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
	data[snake->pos_i[oldHead]][snake->pos_j[oldHead]] = snake->body_char;

	// Update head
	if(!hit_wall)
		data[snake->pos_i[newHead]][snake->pos_j[newHead]] = snake->head_char;
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
			data[snake1->pos_i[snake1->head]][snake1->pos_j[snake1->head]] = snake2->body_char;
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

// check win
bool check_win(Snake* snake) {
	if(snake->length == 123)
		return 1;
	return 0;
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