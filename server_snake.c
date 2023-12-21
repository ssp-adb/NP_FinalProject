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
    int listenfd, connfd, n, epfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    char usr_msg[1024];
    //time_t ticks;
    // FILE * fp;
    struct usr cli1, cli2;
   
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

    int cnt=0;
    for (;;) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue; /* back to for() */
            else
                printf("accept error");
        }
        cli1.skt = connfd;

        //get ip addr
        inet_ntop(AF_INET, & cliaddr.sin_addr, cli1.ip_addr, sizeof(cli1.ip_addr));
        printf("client connected from %s:%d\n", cli1.ip_addr, cliaddr.sin_port);

        //get client id
        n = read(cli1.skt, cli1.id, MAX_ID_LEN);
        cli1.id[n]=0;

        // send first msg
        strcpy(usr_msg, "You are the 1st user. Wait for the second one!\n");
        write(cli1.skt, usr_msg, strlen(usr_msg));

        //====================For 2nd client=======================
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, & clilen)) < 0) {
            if (errno == EINTR)
                continue; /* back to for() */
            else
                printf("accept error");
        }
        cli2.skt = connfd;

        //get ip addr
        inet_ntop(AF_INET, & cliaddr.sin_addr, cli2.ip_addr, sizeof(cli2.ip_addr));
        printf("client connected from %s:%d\n", cli2.ip_addr, cliaddr.sin_port);

        //get client id
        read(cli2.skt, cli2.id, MAX_ID_LEN);
        cli2.id[n]=0;

        sprintf(usr_msg, "The second user is %s from %s\n", cli2.id, cli2.ip_addr);
        write(cli1.skt, usr_msg, strlen(usr_msg));

        // send second msg
        strcpy(usr_msg, "You are the 2nd user\n");
        write(cli2.skt, usr_msg, strlen(usr_msg));
        sprintf(usr_msg, "The first user is %s from %s\n", cli1.id, cli1.ip_addr);
        write(cli2.skt, usr_msg, strlen(usr_msg));

        if ((childpid = fork()) == 0) {
            /* child process */
            close(listenfd); /* close listening socket */

            chat(cli1, cli2);

            exit(0);
        }
        close(cli1.skt); /* parent closes connected socket */
        close(cli2.skt);
    }
}
