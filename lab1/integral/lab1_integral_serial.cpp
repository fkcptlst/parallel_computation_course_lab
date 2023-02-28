#include <stdlib.h>
#include <stdio.h>
#include <math.h>

long long N;
double PI=0.0;

int main(int arg, char* argv[]) {
	N = atoi(argv[1]);
    for (int i = 0;i < N;i++) {
        PI += (4.0 / (1.0 + ((i + 0.5) / N) * ((i + 0.5) / N))) / N;
    }
    printf("result=%lf\n", PI);
    return 0;
}
