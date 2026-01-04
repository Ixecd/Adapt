#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
// sudo -> sudo perf stat ./main
// sudo record / report / script
// perf top -> dynamic spy the sys perf
int main() {
    printf("hello, pid=%d\n", getpid());
    while (1) {
        printf("begin sleep...\n");
        sleep(1);
    }
    return 0;
}