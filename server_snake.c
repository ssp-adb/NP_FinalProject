#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <tool/snake_server.h>

#define MAXLINE 512
#define MAX_ID_LEN 50
#define MAX_EVENTS 100
#define MAX_CLINETS 256

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    ;
    return;
}

struct usr {
    int skt;
    char id[MAX_ID_LEN];
    char ip_addr[MAXLINE];
};

struct usr waiting_clients[MAX_CLINETS];

// void app_end(struct usr leave_usr, struct usr cli1, struct usr cli2){
//     int n;
//     char buf[MAXLINE], leave_msg[MAXLINE];

//     // dealing with ctrl+D ending process
//     if(leave_usr.skt == cli1.skt){
//         for(;;){
//             n = read(cli2.skt, buf, MAXLINE);
//             if(n == 0){
//                 strcpy(leave_msg, "(");
//                 strcat(leave_msg, cli2.id);
//                 strcat(leave_msg, " left the room)\n");
//                 send(cli1.skt, leave_msg, strlen(leave_msg), 0);
//                 shutdown(cli1.skt, SHUT_WR);
//                 return;
//             }
//         }
//     }
//     else{
//         for(;;){
//             n = read(cli1.skt, buf, MAXLINE);
//             if(n == 0){
//                 strcpy(leave_msg, "(");
//                 strcat(leave_msg, cli1.id);
//                 strcat(leave_msg, " left the room)\n");
//                 send(cli2.skt, leave_msg, strlen(leave_msg), 0);
//                 shutdown(cli2.skt, SHUT_WR);
//                 return;
//             }
//         }
//     }
// }

typedef struct {
    char data[ROW+1][COL+1];
    int fruit_eaten;
    int enemy_eaten;
    int winner;
} send_info;

send_info *create_info(char dt[ROW+1][COL+1], int f_e, int e_e, int winner){
    send_info *tmp_info = malloc(sizeof(send_info));
    // copying data
    for(int i = 0; i <= ROW; i++) {
        for(int j = 0; j <= COL; j++) {
            tmp_info->data[i][j] = dt[i][j];
        }
    }
    tmp_info->fruit_eaten = f_e;
    tmp_info->enemy_eaten = e_e;
    tmp_info->winner = winner;

    return tmp_info;
}

void snake_game(struct usr cli1, struct usr cli2){
    // game initialize
    srand(time(NULL));

    int fruit_eaten_1 = 0, fruit_eaten_2 = 0;
    int winner = 0;
    char data[ROW+1][COL+1] = {0};
    Snake snake1, snake2;

    init_data(data);
    Init_snake(&snake1, &snake2, data);

    // communicate with clients
    int epfd, connfd, n;
    char input[MAXLINE];
    send_info *cli1_info, *cli2_info;

    struct epoll_event ev;
    struct epoll_event *events = (struct epoll_event*) malloc(sizeof(struct epoll_event) * MAX_EVENTS);

    epfd = epoll_create(MAX_EVENTS);

    // 1st client
    ev.events = EPOLLIN;
    ev.data.fd = cli1.skt;
    epoll_ctl(epfd, EPOLL_CTL_ADD, cli1.skt, &ev);
    
    // 2nd client
    ev.data.fd = cli2.skt;
    epoll_ctl(epfd, EPOLL_CTL_ADD, cli2.skt, &ev);

    // running server and game
    while(!winner){ // if winner NOT come out yet
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, 100);
        if(nfds < 0 && errno == EINTR) printf("timeout (0.1s)\n");
        else{
            for(int i = 0; i < nfds; i++){
                connfd = events[i].data.fd;
                if(events[i].events & EPOLLIN){
                    if(connfd == cli1.skt){
                        // get input (non blocking)
                        if((n = recv(cli1.skt, input, MAXLINE, MSG_DONTWAIT)) <= 0){
                            if(errno == EAGAIN || errno == EWOULDBLOCK){
                                //non-blocking recv nothing, maintain original direction
                                break; 
                            }
                            else if(n==0){
                                //client 1 leaves game in other ways
                                snake1.direction = 'q';
                            }
                        }
                        else snake1.direction = input[0];
                    }
                    else if(connfd == cli2.skt){
                        if((n = recv(cli2.skt, input, MAXLINE, MSG_DONTWAIT)) <= 0){
                            if(errno == EAGAIN || errno == EWOULDBLOCK){
                                //non-blocking recv nothing, maintain original direction
                                break; 
                            }
                            else if(n==0){
                                //client 1 leaves game in other ways
                                snake2.direction = 'q';
                            }
                        }
                        else snake2.direction = input[0];
                    }
                }
            }
        }
        if(snake1.direction == 'q'){
            winner = 2;
            break;
        }
        if(snake2.direction == 'q'){
            winner = 1;
            break;
        }
        if((winner = update_snake(&snake1, &snake2, data)))
            break;
        if((winner = Check_win(&snake1, &snake2)))
            break;
        fruit_eaten_1 = snake1.length-3;
        fruit_eaten_2 = snake2.length-3;

        cli1_info = create_info(data, fruit_eaten_1, fruit_eaten_2, winner);
        cli2_info = create_info(data, fruit_eaten_1, fruit_eaten_2, winner);
        
        // sending data to client
        send(cli1.skt, (void *)cli1_info, sizeof(send_info), MSG_DONTWAIT);
        send(cli2.skt, (void *)cli2_info, sizeof(send_info), MSG_DONTWAIT);

        usleep(120000);
    }

    cli1_info = create_info(data, fruit_eaten_1, fruit_eaten_2, winner);
    cli2_info = create_info(data, fruit_eaten_1, fruit_eaten_2, winner);

    // blocking output
    send(cli1.skt, (void *)cli1_info, sizeof(send_info), 0);
    send(cli2.skt, (void *)cli2_info, sizeof(send_info), 0);
}

int main(int argc, char ** argv) {
    int listenfd, connfd, n;
    int i, maxi, maxfdp1, nready, sockfd;
    fd_set rset, allset;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    char buf[MAXLINE];
    //time_t ticks;
    // FILE * fp;
    struct usr new_cli;
    struct usr ready_cli;
    ready_cli.skt = -1; // initialize
   
    int reuse = 1;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9880); // Listening port

    bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    listen(listenfd, MAX_EVENTS);

    signal(SIGCHLD, sig_chld); /* must call waitpid() */


    maxfdp1 = listenfd + 1; // initialize
    maxi = -1; // index to waiting_clients[] array
    for (i = 0; i < MAX_CLINETS; i++)
        waiting_clients[i].skt = -1; // -1 meaning available
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    for (;;) {
        rset = allset;
        nready = select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)){ // new client connection
            clilen = sizeof(cliaddr);
            if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) < 0) {
                if (errno == EINTR)
                    continue; /* back to for() */
                else
                    printf("accept error");
            }
            //get socket and store
            new_cli.skt = connfd;

            //get client ip addr
            inet_ntop(AF_INET, & cliaddr.sin_addr, new_cli.ip_addr, sizeof(new_cli.ip_addr));
            printf("client connected from %s:%d\n", new_cli.ip_addr, cliaddr.sin_port);

            //get client id
            n = read(new_cli.skt, new_cli.id, MAX_ID_LEN);
            if(n <= 0) {
                printf("Error read\n");
            }
            new_cli.id[n] = 0;

            for(i = 0; i < MAX_CLINETS; i++){
                if(waiting_clients[i].skt < 0){
                    waiting_clients[i] = new_cli;
                    //send(waiting_clients[i].skt, "okay\n", 5, 0);
                    break;
                }
            }
            if(i == MAX_CLINETS){
                printf("too many clients\n");
                return -1;
            }

            FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd+1 > maxfdp1)
				maxfdp1 = connfd + 1;	/* for select */
			if (i > maxi)
				maxi = i;				/* max index in waiting_clients array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++){ // check all clients for yes/no
            if((sockfd = waiting_clients[i].skt) < 0) {
                continue;
            }
            
            if(FD_ISSET(sockfd, &rset)){
                // check YES or exit
                if((n = read(sockfd, buf, MAXLINE)) == 0){ //connection reset
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    waiting_clients[i].skt = -1;
                }
                else{
                    buf[n]=0;
                    if(strcmp(buf, "yes\n")==0){
                        if(ready_cli.skt == -1){ // first client has not come yet
                            ready_cli = waiting_clients[i];
                            waiting_clients[i].skt = -1;

                            // send first msg
                            send(ready_cli.skt, "1", 1, 0);
                        }
                        else{
                            // send second msg
                            send(waiting_clients[i].skt, "2", 1, 0);

                            if((childpid = fork()) == 0){ //child process
                                close(listenfd);
                                snake_game(ready_cli, waiting_clients[i]);
                                //chat(ready_cli, waiting_clients[i]);

                                exit(0); // finish application
                            }
                            /* parent closes connected socket */
                            close(ready_cli.skt);
                            FD_CLR(ready_cli.skt, &allset);
                            ready_cli.skt = -1;
                            close(waiting_clients[i].skt);
                            FD_CLR(waiting_clients[i].skt, &allset);
                            waiting_clients[i].skt = -1;
                        }
                    }
                }

                if (--nready <= 0)
				    continue;		/* no more readable descriptors */
            }
        }
    }
}
