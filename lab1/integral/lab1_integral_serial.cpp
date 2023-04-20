#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

long long N;
double PI=0.0;

int main(int arg, char* argv[]) {
	N = atoi(argv[1]);
    clock_t proc_start_time,proc_end_time;
    proc_start_time = clock();
    for (int i = 0;i < N;i++) {
        PI += (4.0 / (1.0 + ((i + 0.5) / N) * ((i + 0.5) / N))) / N;
    }
    proc_end_time = clock();
    printf("proc time=%lf\n",double(proc_end_time - proc_start_time) / (double)CLOCKS_PER_SEC);
    // printf("result=%lf\n", PI);
    return 0;
}
