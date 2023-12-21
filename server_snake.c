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

void app_end(struct usr leave_usr, struct usr cli1, struct usr cli2){
    int n;
    char buf[MAXLINE], leave_msg[MAXLINE];

    // dealing with ctrl+D ending process
    if(leave_usr.skt == cli1.skt){
        for(;;){
            n = read(cli2.skt, buf, MAXLINE);
            if(n == 0){
                strcpy(leave_msg, "(");
                strcat(leave_msg, cli2.id);
                strcat(leave_msg, " left the room)\n");
                write(cli1.skt, leave_msg, strlen(leave_msg));
                shutdown(cli1.skt, SHUT_WR);
                return;
            }
        }
    }
    else{
        for(;;){
            n = read(cli1.skt, buf, MAXLINE);
            if(n == 0){
                strcpy(leave_msg, "(");
                strcat(leave_msg, cli1.id);
                strcat(leave_msg, " left the room)\n");
                write(cli2.skt, leave_msg, strlen(leave_msg));
                shutdown(cli2.skt, SHUT_WR);
                return;
            }
        }
    }
}

void chat(struct usr cli1, struct usr cli2) {
    int epfd, connfd, n, maxfdp1;
    fd_set rset;
    char buf[MAXLINE], fir2sec_msg[1024], sec2fir_msg[1024];
    char leave_msg[1024];
    struct usr leave_usr;
    int leaving = 0;

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
    for(;;){
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++){
            connfd = events[i].data.fd;
            if(events[i].events & EPOLLIN){
                if(connfd == cli1.skt){ // 1st client
                    n = read(cli1.skt, buf, MAX_ID_LEN);
                    if(n == 0){
                        sprintf(leave_msg, "(%s left the room. Press Ctrl+D to leave.)\n", cli1.id);
                        
                        // sending msg and FIN to another client
                        write(cli2.skt, leave_msg, strlen(leave_msg));
                        shutdown(cli2.skt, SHUT_WR);
                        leave_usr = cli1;
                        leaving = 1;
                        break;
                    }
                    else if(n > 0){
                        buf[n-1]=0;
                        sprintf(fir2sec_msg, "(%s) %s", cli1.id, buf);

                        write(cli2.skt, fir2sec_msg, strlen(fir2sec_msg));
                    }
                }
                else if(connfd == cli2.skt){ // 2nd client
                    // check EOF (ctrl + D)
                    n = read(cli2.skt, buf, MAX_ID_LEN);
                    if(n == 0){
                        sprintf(leave_msg, "(%s left the room. Press Ctrl+D to leave.)\n", cli2.id);
                        
                        // sending msg and FIN to another client
                        write(cli1.skt, leave_msg, strlen(leave_msg));
                        shutdown(cli1.skt, SHUT_WR);
                        leave_usr = cli2;
                        leaving = 1;
                        break;
                    }
                    else if(n > 0){
                        buf[n-1]=0;
                        sprintf(sec2fir_msg, "(%s) %s", cli2.id, buf);

                        write(cli1.skt, sec2fir_msg, strlen(sec2fir_msg));
                    }
                }
            }
        }
        if(leaving) break;
    }
    app_end(leave_usr, cli1, cli2);
    return;
}

int main(int argc, char ** argv) {
    int listenfd, connfd, n;
    int i, maxi, maxfdp1, nready, sockfd;
    fd_set rset, allset;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    char usr_msg[1024], buf[MAXLINE];
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
                    //write(waiting_clients[i].skt, "okay\n", 5);
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
            if((sockfd = waiting_clients[i].skt) < 0) continue;
            
            if(FD_ISSET(sockfd, &rset)){
                if((n = read(sockfd, buf, MAXLINE)) == 0){ //connection reset
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    waiting_clients[i].skt = -1;
                }
                else{
                    buf[n]=0;
                    //if(strcmp(buf, "yes")==0){
                    if(ready_cli.skt == -1){
                        ready_cli = waiting_clients[i];
                        waiting_clients[i].skt = -1;

                        // send first msg
                        strcpy(usr_msg, "You are the 1st user. Wait for the second one!\n");
                        write(ready_cli.skt, usr_msg, strlen(usr_msg));
                    }
                    else{
                        sprintf(usr_msg, "The second user is %s from %s\n", waiting_clients[i].id, waiting_clients[i].ip_addr);
                        write(ready_cli.skt, usr_msg, strlen(usr_msg));
                        // send second msg
                        strcpy(usr_msg, "You are the 2nd user\n");
                        write(waiting_clients[i].skt, usr_msg, strlen(usr_msg));
                        sprintf(usr_msg, "The first user is %s from %s\n", ready_cli.id, ready_cli.ip_addr);
                        write(waiting_clients[i].skt, usr_msg, strlen(usr_msg));

                        if((childpid = fork()) == 0){ //child process
                            close(listenfd);
                            chat(ready_cli, waiting_clients[i]);

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
                    //}
                }

                if (--nready <= 0)
				    continue;		/* no more readable descriptors */
            }
        }
        // //get client id
        // n = read(cli1.skt, cli1.id, MAX_ID_LEN);
        // cli1.id[n]=0;

        // // send first msg
        // strcpy(usr_msg, "You are the 1st user. Wait for the second one!\n");
        // write(cli1.skt, usr_msg, strlen(usr_msg));

        // //====================For 2nd client=======================
        // clilen = sizeof(cliaddr);
        // if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, & clilen)) < 0) {
        //     if (errno == EINTR)
        //         continue; /* back to for() */
        //     else
        //         printf("accept error");
        // }
        // cli2.skt = connfd;

        // //get ip addr
        // inet_ntop(AF_INET, & cliaddr.sin_addr, cli2.ip_addr, sizeof(cli2.ip_addr));
        // printf("client connected from %s:%d\n", cli2.ip_addr, cliaddr.sin_port);

        // //get client id
        // read(cli2.skt, cli2.id, MAX_ID_LEN);
        // cli2.id[n]=0;

        // sprintf(usr_msg, "The second user is %s from %s\n", cli2.id, cli2.ip_addr);
        // write(cli1.skt, usr_msg, strlen(usr_msg));

        // // send second msg
        // strcpy(usr_msg, "You are the 2nd user\n");
        // write(cli2.skt, usr_msg, strlen(usr_msg));
        // sprintf(usr_msg, "The first user is %s from %s\n", cli1.id, cli1.ip_addr);
        // write(cli2.skt, usr_msg, strlen(usr_msg));

        // if ((childpid = fork()) == 0) {
        //     /* child process */
        //     close(listenfd); /* close listening socket */

        //     chat(cli1, cli2);

        //     exit(0);
        // }
        // close(cli1.skt); /* parent closes connected socket */
        // close(cli2.skt);
    }
}
