#include "snake_component.h"
#include "server_component.h"
#include "snake_client.h"
#include "snake_server.h"

int main() {
    srand(time(NULL));

    int fruit_eaten_1 = 0, fruit_eaten_2 = 0;
    char data[ROW+1][COL+1] = {0};
    Snake snake1, snake2;

    init_data(data);
    Init_snake(&snake1, &snake2, data);

    init_screen();
    draw(data, 0, 0);

    int winner = 0;
    while(!winner) {
        if((snake1.direction = get_input(snake1.direction)) == 'q') {
            winner = 3;
            break;
        }
		if(winner = update_snake(&snake1, &snake2, data))
            break;
        if(winner = Check_win(&snake1, &snake2))
            break;
        fruit_eaten_1 = snake1.length-3;
        fruit_eaten_2 = snake2.length-3;
        draw(data, fruit_eaten_1, fruit_eaten_2);
        usleep(120000);
    }

    draw(data, fruit_eaten_1, fruit_eaten_2);
    switch(winner) {
        case 1:
            draw_game_over(-1);
            break;
        case 2:
            draw_game_over(-2);
            break;
        case 3:
            draw_game_over(-3);
            break;
    }

    exit(0);
}