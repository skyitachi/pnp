//
// Created by skyitachi on 2017/4/18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>

struct Message {
    int64_t request;
    int64_t response;
} __attribute__ ((__packed__));

int64_t now() {
    struct timeval tv= {0, 0};
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void start_server(const struct sockaddr* servaddr) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sockfd, servaddr, sizeof(struct sockaddr));
    listen(sockfd, 1);
    struct sockaddr_in src_addr;
    socklen_t sock_len = sizeof(struct sockaddr);
    for(;;) {
        struct Message msg = {0, 0};
        const int msgLen = sizeof(Message);
        ssize_t cnt = recvfrom(sockfd, &msg, msgLen, 0, (struct sockaddr *)&src_addr, &sock_len);
        if (cnt != msgLen) {
            fprintf(stderr, "server read message error\n");
            exit(1);
        }
        printf("server receive request is %lld\n", msg.request);

        msg.response = now();
        sendto(sockfd, &msg, msgLen, 0, (struct sockaddr *)&src_addr, sock_len);
        printf("server send response is %lld\n", msg.response);
    }
}

void run_client(const struct sockaddr* servaddr) {
    int msgLen = sizeof(Message);
    // UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // no need connect??
    // UDP is connectionless protocol
//    if ((connect(sockfd, servaddr, sizeof(struct sockaddr))) < 0) {
//        fprintf(stderr, "connect server error: %s\n", strerror(errno));
//        exit(1);
//    }
    struct Message msg = {0, 0};
    msg.request = now();
    // send
    if (sendto(sockfd, &msg, sizeof(msg), 0, servaddr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "client write error: %s\n", strerror(errno));
        exit(1);
    }
    printf("client send request: %lld\n", msg.request);

    // no need to get src_addr
//    struct sockaddr_in src_addr;
//    socklen_t src_addr_len;
//    int cnt = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&src_addr, &src_addr_len);
    int cnt = recvfrom(sockfd, &msg, sizeof(msg), 0, NULL, NULL);
    if (cnt != msgLen) {
        perror("client read error\n");
    }
    int64_t c = now();
    int64_t offset = (c + msg.request)/ 2 - msg.response;
    printf("client and server offset is %lld usec\n", offset);
}


void set_addr(const char *hostname, struct in_addr *sin_addr) {
    struct hostent *he;
    char str[INET_ADDRSTRLEN];
    if ((he = gethostbyname(hostname)) == NULL) {
        fprintf(stderr, "resolve %s error\n", hostname);
        exit(1);
    }
    const char *ip = inet_ntop(he->h_addrtype, he->h_addr_list[0], str, sizeof(str));
    inet_pton(he->h_addrtype, ip, sin_addr);
}


int main(int argc, char **argv) {
    struct sockaddr_in servaddr;
    const char* host = "localhost";
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10001);
    set_addr(host, &servaddr.sin_addr);
    if (argc > 1) { // client
        run_client((const struct sockaddr*)&servaddr);
    } else { // server
        start_server((const struct sockaddr *)&servaddr);
    }
    return 0;
}

