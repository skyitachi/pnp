#include <iostream>
#include <sys/time.h>

int main() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    std::cout << tv.tv_usec << std::endl;
    struct timespec tp = {0, 0};
    clock_gettime(CLOCK_REALTIME, &tp);
    std::cout << tp.tv_nsec << std::endl;
    return 0;
}