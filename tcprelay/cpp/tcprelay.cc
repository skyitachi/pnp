//
// Created by skyitachi on 2017/2/9.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <thread>

#define MAXSIZE 4096
#define SA struct sockaddr

int main() {
    int listenfd, connfd, relayfd, n, optval;
    socklen_t optlen = sizeof(optval);
    struct sockaddr_in relayaddr, servaddr;
    const char *remote = "127.0.0.1";
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // TODO why reuse address
    getsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);
    bzero(&relayaddr, sizeof(relayaddr));
    bzero(&servaddr, sizeof(servaddr));
    relayaddr.sin_family = AF_INET;
    relayaddr.sin_port = htons(9001);
    relayaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9002);
    inet_pton(AF_INET, remote, &servaddr.sin_addr);
    bind(listenfd, (SA *) &relayaddr, sizeof(relayaddr));
    listen(listenfd, 1/* backlog */);
    for (;;) {
        relayfd = accept(listenfd, (SA *) NULL, NULL);
        struct sockaddr_in request;
        socklen_t len = sizeof(request);
        if (getsockname(relayfd, (SA *) &request, &len) != -1) {
            printf("port number is %d\n", ntohs(request.sin_port));
        }
        connfd = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(connfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
            printf("has error %s\n", strerror(errno));
        }
        if (getsockname(connfd, (SA *) &request, &len) != -1) {
            printf("port number is %d\n", ntohs(request.sin_port));
        }
        std::thread forward([&connfd, &relayfd] {
            printf("in the forward thread\n");
            int cnt = 0, n;
            char buff[MAXSIZE];
            while ((n = read(relayfd, buff, MAXSIZE)) > 0) {
                write(connfd, buff, n);
                cnt += n;
            }
            // TODO necessary why
            shutdown(connfd, SHUT_WR);
            printf("receive %d bytes data\n", cnt);
            printf("out of the thread1\n");
        });

        std::thread th([&connfd, &relayfd] {
            printf("in the back thread\n");
            int n;
            char buff[MAXSIZE];
            while ((n = read(connfd, buff, MAXSIZE)) > 0) {
                buff[n] = 0;
                write(relayfd, buff, strlen(buff));
                printf("receive from remote server msg: %s\n", buff);
            }
            // TODO necessary
            shutdown(relayfd, SHUT_WR);
            printf("out of the thread2\n");
        });
        th.join();
        forward.join();

        printf("in the main thread\n");
    }
}


