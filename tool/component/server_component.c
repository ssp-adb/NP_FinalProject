#include "server_component.h"

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

// check win
bool check_win(Snake* snake) {
	if(snake->length == 123)
		return 1;
	return 0;
}