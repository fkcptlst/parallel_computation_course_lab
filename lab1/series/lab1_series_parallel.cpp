#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <algorithm>

long long N;
int T;
double PI=0.0;
pthread_mutex_t mutex;

void* thread_function(void* arg);

// input arg: N, num of thread
int main(int arg, char* argv[]) {
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    pthread_t thread[T];
    int ids[T];
    for (int i = 0; i < T; i++) {
        ids[i] = i;
        pthread_create(&thread[i], NULL, thread_function, &ids[i]);
    }

    // wait
    for (int i = 0;i < T;i++) {
        pthread_join(thread[i], NULL);
    }
    // printf("result=%lf\n", PI);
    return 0;
}

void* thread_function(void* arg) {
    int id = *(int*)arg;
    double sum = 0;
    int coe = 1;
    for (int i = id;i < N;i+=T) 
    {
        coe = i%2 ? -1 : 1;
        sum += double(coe)/double(2*i + 1);
    }
    sum *= 4;

    pthread_mutex_lock(&mutex);
    PI += sum;
    pthread_mutex_unlock(&mutex);
    return NULL;
}