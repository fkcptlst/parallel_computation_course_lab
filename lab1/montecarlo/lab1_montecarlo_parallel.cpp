#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

long long N;
int T;

long long M;

double PI = 0.0;

pthread_mutex_t mutex;

void* thread_function(void *arg);

int main(int argc,char *argv[])
{
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    pthread_t thread[T];
    int ids[T];
    for (int i = 0;i < T; i++){
        ids[i] = i;
        pthread_create(&thread[i], NULL, thread_function, &ids[i]);
    }
    for (int i = 0;i < T; i++) {
        pthread_join(thread[i], NULL);
        // printf("Thread %d done!\n", i);
    }
    PI = 4 * M / N;
    // printf("result=%lf\n", PI);
    return 0;
}

void* thread_function(void *arg)
{
    unsigned int seed = 0;
    int sample = N/T;
    long long m = 0;
    int i;
    for(i = 0;i < sample;i++){
        double x = 1.0*rand_r(&seed)/RAND_MAX;
        double y = 1.0*rand_r(&seed)/RAND_MAX;
        if(x * x + y * y < 1.0){
            m++;
        }
    }
    pthread_mutex_lock(&mutex);
    M += m;
    pthread_mutex_unlock(&mutex);
    return NULL;
}
