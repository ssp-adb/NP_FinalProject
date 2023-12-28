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

typedef struct{
    char data[ROW+1][COL+1];
    int fruit_eaten;
    int enimy_eaten;
    int winner;
} send_info;

send_info *self, *enemy;

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
    char buf[] = "yes";
    send(sockfd, buf, strlen(buf), 0);

    char buff[1024];
    memset(buff, 0, sizeof(buf));
    recv(sockfd, buff, sizeof(buf), 0);
    printf("%s", buff);
    fflush(stdout);

    int tmp = rand()%4;
    char dir;
    switch (tmp)
    {
    case 0:
        dir = 'w';
        break;
    case 1:
        dir = 'a';
        break;
    case 2:
        dir = 's';
        break;
    case 3:
        dir = 'd';
        break;
    default:
        break;
    }

    init_screen();

    while(true){
        dir = get_input(dir);
        recv(sockfd, self, sizeof(self), 0);
        recv(sockfd, enemy, sizeof(enemy), 0);
        draw(self->data, self->fruit_eaten, self->enimy_eaten);
    }
    return 0;
}