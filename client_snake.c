#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <stdbool.h>

#include "tool/snake_client.h"

#define and &&
#define or ||

typedef struct{
    char data[ROW+1][COL+1];
    int fruit_eaten;
    int enemy_eaten;
    int winner;
} send_info;

//send_info *self, *enemy;

int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s server_ip\n", argv[0]);
        exit(1);
    }
    srand(time(NULL));
    
    printf("Welcome to snake game!\nPlease input your name: ");
    char name[50];
    memset(name, 0, sizeof(name));
    scanf("%s", name);

    /*
    printf("%s", name);
    fflush(stdout);
    */

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    if(sockfd < 0){
        perror("socket");
        exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9880);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        exit(1);
    }

    send(sockfd, name, strlen(name), 0);

    sleep(1);
    char buf[] = "yes\n";
    send(sockfd, buf, strlen(buf), 0);

    char cli[10];
    recv(sockfd, cli, sizeof(cli), 0);
    printf("%s\n", cli);
    fflush(stdout);
    // char buff[1024];
    // memset(buff, 0, sizeof(buf));
    // recv(sockfd, buff, sizeof(buf), 0);
    // printf("%s", buff);
    // fflush(stdout);

    //int tmp = rand()%4;
    char dir = (cli[0] == '1') ? 'd' : 'a';
    int p;
    if(dir == 'd'){
        printf("You are the first player, your snake is at left up side at beginning!\n");
        p=1;
    }else{
        printf("You are the second player, your snake is at right down side at beginning!\n");
        p=2;
    }
    //sleep(2);

    init_screen();
    
    send_info *self = malloc(sizeof(send_info));

    recv(sockfd, self, sizeof(send_info), 0);
    draw(self->data, self->fruit_eaten, self->enemy_eaten, p);

    while(true){
        dir = get_input(dir);
        send(sockfd, &dir, sizeof(dir), MSG_DONTWAIT);

        recv(sockfd, self, sizeof(send_info), 0);
        draw(self->data, self->fruit_eaten, self->enemy_eaten, p);
        if (self->winner !=0){
            break;
        }
    }
    int status = self->winner;
    if (status == 1 and cli[0]=='1'){
        draw_game_over(-1);
    }else if (status == 1 and cli[0]=='2'){
        draw_game_over(-2);
    }else if (status == 2 and cli[0]=='1'){
        draw_game_over(-2);
    }else if (status == 2 and cli[0]=='2'){
        draw_game_over(-1);
    }else if (status == 3){
        draw_game_over(-3);
    }
    return 0;
}