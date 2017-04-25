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
#include <sys/time.h>
#include <thread>
#include <zconf.h>

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
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(sockfd, servaddr, sizeof(struct sockaddr));
    // Note: UDP does not need listen
//    listen(sockfd, 1);
    struct sockaddr_in src_addr;
    socklen_t sock_len = sizeof(struct sockaddr);
    for(;;) {
        struct Message msg = {0, 0};
        const int msgLen = sizeof(Message);
        ssize_t cnt = recvfrom(sockfd, &msg, msgLen, 0, (struct sockaddr *)&src_addr, &sock_len);
        if (cnt < 0) {
            perror("server recv error");
        } else if (cnt != msgLen) {
            fprintf(stderr, "server read %zd bytes message expect %d bytes data\n", cnt, msgLen);
            exit(1);
        }
        printf("server receive request is %lld\n", msg.request);

        msg.response = now();
        ssize_t nw = sendto(sockfd, &msg, msgLen, 0, (struct sockaddr *)&src_addr, sock_len);
        if (nw < 0) {
            perror("server udp send error");
        } else if (nw != msgLen) {
            fprintf(stderr, "server send %zd bytes data expects %d bytes data\n", nw, msgLen);
        }
        printf("server send response is %lld\n", msg.response);
    }
}

void run_client(const struct sockaddr* servaddr) {
    // UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // UDP is connectionless protocol
//    if ((connect(sockfd, servaddr, sizeof(struct sockaddr))) < 0) {
//        fprintf(stderr, "connect server error: %s\n", strerror(errno));
//        exit(1);
//    }
    // send, maybe block
    std::thread thr([&sockfd, &servaddr] () {
        int msgLen = sizeof(Message);
        while(true) {
            struct Message msg = {0, 0};
            msg.request = now();
            ssize_t nw = sendto(sockfd, &msg, msgLen, 0, servaddr, sizeof(struct sockaddr));
            if (nw < 0) {
                perror("client send error");
            } else if (nw != msgLen) {
                fprintf(stderr, "client send %zd bytes data expect %d bytes data\n", nw, msgLen);
            }
//            printf("client send request: %lld\n", msg.request);
            usleep(1000 * 500); // 500ms
        }
    });

    // no need to get src_addr
//    struct sockaddr_in src_addr;
//    socklen_t src_addr_len;
//    int cnt = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&src_addr, &src_addr_len);
    while(true) {
        struct Message msg = {0, 0};
        int msgLen = sizeof(Message);
        ssize_t cnt = recvfrom(sockfd, &msg, sizeof(msg), 0, NULL, NULL);
        if (cnt < 0) {
            perror("client recv error");
        } else if (cnt != msgLen) {
            fprintf(stderr, "client receive %zd bytes data expect %d bytes data\n", cnt, msgLen);
        }
        int64_t c = now();
        int64_t offset = (c + msg.request)/ 2 - msg.response;
        printf("client and server offset is %lld usec\n", offset);
    }
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
    if (argc < 2) {
        printf("Usage:\nServer: %s -s\nClient %s host\n", argv[0], argv[0]);
        exit(1);
    }
    struct sockaddr_in servaddr;
    // boilerplate, bzero
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10001);
    if (strcmp(argv[1], "-s") != 0) { // client
        set_addr(argv[1], &servaddr.sin_addr);
        run_client((const struct sockaddr*)&servaddr);
    } else { // server
        servaddr.sin_addr.s_addr = INADDR_ANY;
        start_server((const struct sockaddr *)&servaddr);
    }
    return 0;
}

