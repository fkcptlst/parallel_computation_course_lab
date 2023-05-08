#include <stdlib.h>
#include <stdio.h>
#include <math.h>

long long N;
double PI=0.0;

int main(int arg, char* argv[]) 
{
	N = atoi(argv[1]);
    int coe = 1;
    for (int i = 0;i < N;i++) 
    {
        coe = i%2 ? -1 : 1;
        PI += double(coe)/double(2*i + 1);
    }
    PI *= 4;
    // printf("result=%lf\n", PI);
    return 0;
}
