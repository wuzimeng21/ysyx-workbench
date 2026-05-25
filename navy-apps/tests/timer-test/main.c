#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
    printf("====Timer test start====\n");

    struct timeval tv;
    int count = 0;

    while (count < 100) {
        usleep(500000);
        assert(gettimeofday(&tv, NULL) == 0);
        printf("[%d] Time: %ld.%06ld\n", count, tv.tv_sec, tv.tv_usec);
        count++;
    }

    printf("====Timer test finish===\n");
    return 0;
}
