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

#include "tool/snake_client.h"

int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s server_ip\n", argv[0]);
        exit(1);
    }

    printf("Welcome to snake game!\nPlease input your name: ");
    char name[50];
    memset(name, 0, sizeof(name));
    size_t len = 50;
    getline(&name, &len, stdin);

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
    return 0;
}