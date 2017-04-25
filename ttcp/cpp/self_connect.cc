//
// Created by skyitachi on 2017/4/15.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>

#define COUNT 65536

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, srvaddr, peeraddr;
    bzero(&servaddr, sizeof(servaddr));
    bzero(&srvaddr, sizeof(srvaddr));
    bzero(&peeraddr, sizeof(peeraddr));
    socklen_t srv_len, peer_len;
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(servaddr.sin_addr));

    for(int i = 0; i < COUNT; i++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (ret == 0) {
            printf("in the success branch\n");
            getsockname(sockfd, (struct sockaddr *)&srvaddr, &srv_len);
            getpeername(sockfd, (struct sockaddr *)&peeraddr, &peer_len);
            printf("self connect success\n");
            break;
        } else if (errno != ECONNREFUSED) {
            printf("connect error: %s", strerror(errno));
            break;
        }
        usleep(5000); // 5ms
    }
    return 0;
}
