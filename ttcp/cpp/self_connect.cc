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

struct thread_info {
    int connfd;
};
void *thr_fn(void *arg) {
    struct thread_info *info = (struct thread_info *)arg;
    printf("in the thread, connect fd is %d\n", info->connfd);
    return ((void *) 0);
}
int main() {
    pthread_t ntid;
    struct thread_info *tinfo;
    tinfo = (struct thread_info *)malloc(sizeof(struct thread_info));
    int err;
    tinfo->connfd = 100;
    err = pthread_create(&ntid, NULL, thr_fn, tinfo);
    printf("in the main thread\n");
    return 0;
}
