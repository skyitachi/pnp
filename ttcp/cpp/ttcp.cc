#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <netinet/tcp.h>

struct SessionMessage {
    int32_t number;
    int32_t length;
} __attribute__ ((__packed__));

struct PayloadMessage {
    int32_t length;
    char data[0]; // 任意长的字节
};

struct ThreadArg {
    int connfd;
};

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

void *respond_thread(void *arg) {
    printf("created thread to respond\n");
    struct ThreadArg *tArg = (struct ThreadArg *) arg;
    struct timeval time_start, time_end, time_result;

    // record after the connection established
    gettimeofday(&time_start, NULL);

    int connfd = tArg->connfd, length, number, total_len;
    ssize_t received;
    struct SessionMessage sessionMessage;
    received = recv(connfd, &sessionMessage, sizeof(sessionMessage), 0);
    if (received != sizeof(sessionMessage)) {
        fprintf(stderr, "recv error\n");
        exit(1);
    }
    number = ntohl(sessionMessage.number);
    length = ntohl(sessionMessage.length);
    printf("SessionMessage number is %d, length is %d\n", number, length);
    ssize_t n, snt, cnt = 0;
    total_len = sizeof(int32_t) + length;
    PayloadMessage *payload = static_cast<PayloadMessage *>(malloc(total_len));
    while ((n = read(connfd, payload, total_len)) > 0) {
        cnt += n;
        if (cnt < total_len) continue;
        else if (cnt > total_len) {
            printf("server receive data error\n");
        }
        snt = write(connfd, &length, sizeof(sessionMessage.length));
        if (snt != sizeof(sessionMessage.length)) {
            fprintf(stderr, "server write ack error\n");
            exit(1);
        }
        cnt = 0;
    }
    gettimeofday(&time_end, NULL);
    timersub(&time_end, &time_start, &time_result);
    double total = number * length * 1.0 / 1024 / 1024;
    printf("Total Received %.2lf MB Data, Rate is %.2lf MB/s\n",
           total, total * 100000 / (time_result.tv_sec * 1000000 + time_result.tv_usec));
    free(tArg);
    close(connfd);
    return ((void *) 0);
}

// server start listen
void start_server(const char *host, const int port) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t socklen;
    // original socket function
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "cannot create socket");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    set_addr(host, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, 1);
    printf("Start Listening on port: %d\n", port);
    for (;;) {
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &socklen);
        // SO_LINGER
        printf("client port is %d\n", ntohs(clientaddr.sin_port));
        // start new thread
        struct ThreadArg *tArg = (struct ThreadArg *) malloc(sizeof(struct ThreadArg));
        tArg->connfd = connfd;
        pthread_t tid;
        int err = pthread_create(&tid, NULL, respond_thread, tArg);
        if (err != 0) {
            fprintf(stderr, "create thread error\n");
        }
    }
}

void request(const char *host, const int port, int length, int number) {
    struct timeval time_start, time_end, time_result;
    int connfd;
    int32_t ack;
    ssize_t snt, n;
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    set_addr(host, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    connfd = socket(AF_INET, SOCK_STREAM, 0);
    // TCP_NODELAY
    // Note: no effect?
    int optval = 1; // none zero: enable option, 0: disable the option
    if ((setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &optval, (socklen_t)sizeof(optval)) < 0)) {
        fprintf(stderr, "set TCP_NODELAY error: %s\n", strerror(errno));
    }
    if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        fprintf(stderr, "Some Error Happens %s\n", strerror(errno));
        exit(1);
    }
    printf("Connection Established\n");
    // record after the connection established
    gettimeofday(&time_start, NULL);
    // Note: should use network byte order, macos host byte order like the network byte order
    struct SessionMessage sessionMessage;
    sessionMessage.length = htonl(length);
    sessionMessage.number = htonl(number);
    // send session message
    snt = send(connfd, &sessionMessage, sizeof(sessionMessage), 0);
    if (snt != sizeof(sessionMessage)) {
        fprintf(stderr, "send error\n");
        exit(1);
    }
    // make payload
    int total_len = sizeof(int32_t) + length;
    PayloadMessage *payload = static_cast<PayloadMessage *>(malloc(total_len));
    payload->length = htonl(length);
    for (int i = 0; i < length; i++) {
        payload->data[i] = "0123456789ABCDEF"[i % 16];
    }

    for (int i = 0; i < number; i++) {
        snt = write(connfd, payload, total_len);
        if (snt != total_len) {
            fprintf(stderr, "client write error\n");
            exit(1);
        }
        n = read(connfd, &ack, sizeof(ack));
        if (n != sizeof(ack)) {
            fprintf(stderr, "client read ack error: %s\n", strerror(errno));
            exit(1);
        }
    }
    gettimeofday(&time_end, NULL);
    timersub(&time_end, &time_start, &time_result);
    double total = number * length * 1.0 / 1024 / 1024;
    printf("Total Received %.2lf MB Data, Rate is %.2lf MB/s\n",
           total, total * 100000 / (time_result.tv_sec * 1000000 + time_result.tv_usec));
    close(connfd);
}

int main(int argc, char **argv) {
    int opt;
    char *host = "localhost";
    bool server = true;
    int number = 0, length = 0, port = 10001;
    while ((opt = getopt(argc, argv, "rch:p:n:l:")) != -1) {
        switch (opt) {
            case 'r':
                server = true;
                break;
            case 'c':
                server = false;
                break;
            case 'h':
                host = optarg;
                printf("%s\n", optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'n':
                number = atoi(optarg);
                break;
            case 'l':
                length = atoi(optarg);
                break;
            default:
                fprintf(stderr,
                        "Usage: %s [-r] [-c] [-h hostname] [-p port] \
                    [-n number] [-l length]\n", argv[0]);

                exit(EXIT_FAILURE);
        }
    }
    printf("number=%d, length=%d, hostname=%s\n", number, length, host);
    if (server) {
        start_server(host, port);
    } else {
        request(host, port, length, number);
    }
    return 0;
}

