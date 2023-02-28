#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

long long N;
double PI = 0.0;

long long m;

int main(int argc,char *argv[])
{
    N = atoi(argv[1]);
    unsigned int seed = 0;
    double x,y;
    for(int i = 0;i < N;i++)
    {
        x = 1.0*rand_r(&seed)/RAND_MAX;
        y = 1.0*rand_r(&seed)/RAND_MAX;
        if(x * x + y * y < 1.0){
            m++;
        }
    }
    PI = 4 * m / N;
    printf("result=%lf\n", PI);
    return 0;
}
