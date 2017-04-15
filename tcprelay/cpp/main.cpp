#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define SIZE 4096
#define SA struct sockaddr

void start_server() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[SIZE];
    time_t ticks;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9001);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, 1/* backlog */);
    for (;;) {
        connfd = accept(listenfd, (SA *) NULL, NULL);
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), ".%24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        close(connfd);
    }
}

void send_socket() {
    const char *src = "127.0.0.1";
    int sockfd;
    char buff[SIZE];
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9001);
    if(inet_pton(AF_INET, src, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error %s\n", src);
    }
    printf("addr is %d\n", servaddr.sin_addr.s_addr);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
    }
    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error\n");
        printf("errorno is %s\n", strerror(errno));
    }
//    // write data
//    const char *buf = "hello world";
//    int a[] = {1, 2, 3};
//    printf("sizeof array is %d\n", sizeof(a));
//    write(sockfd, a, sizeof(a));
    int n;
    while((n = read(sockfd, buff, SIZE)) > 0) {
        buff[n] = 0;
//        fputs(buff, stdout);
    }
    close(sockfd);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: ./main [options] -s (start server), -c (client)\n");
    }
    if (strcmp(argv[1], "-s") == 0) {
        start_server();
    } else if (strcmp(argv[1], "-c") == 0) {
        send_socket();
    }
    return 0;
}
