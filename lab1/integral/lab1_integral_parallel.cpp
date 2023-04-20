#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <algorithm>
#include <time.h>

const int MAX_SIZE = 1000;

long long N;
int T;
double PI=0.0;
pthread_mutex_t mutex;

clock_t start_time[MAX_SIZE], end_time[MAX_SIZE];

void* thread_function(void* arg);

// input arg: N, num of thread
int main(int arg, char* argv[]) {
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    pthread_t thread[T];
    int ids[T];

    clock_t proc_start_time, proc_end_time;
    proc_start_time = clock();
    for (int i = 0; i < T; i++) {
        ids[i] = i;
        // start_time[i] = clock();
        pthread_create(&thread[i], NULL, thread_function, &ids[i]);
    }

    // wait
    for (int i = 0;i < T;i++) {
        pthread_join(thread[i], NULL);
        // end_time[i] = clock();
    }
    
    proc_end_time = clock();

    printf("proc time=%lf\n",double(proc_end_time - proc_start_time) / (double)CLOCKS_PER_SEC);
    for(int i=0; i < T; i++)
    {
        printf("thread %d time=%lf\n", i, double(end_time[i] - start_time[i]) / (double)CLOCKS_PER_SEC);
    }

    return 0;
}

void* thread_function(void* arg) {
    int id = *(int*)arg;
    
    start_time[id] = clock();

    long long piece = (N - 1) / T + 1;  // round up
    long long start = id * piece;
    long long end = std::min(start + piece, N);
    double sum = 0.0;
    // printf("start=%lld,end=%lld\n",start,end);
     for (int i = start;i < end;i++) {
        sum += (4.0 / (1.0 + ((i + 0.5) / N) * ((i + 0.5) / N))) / N;
    }
    pthread_mutex_lock(&mutex);
    PI += sum;
    pthread_mutex_unlock(&mutex);
    end_time[id] = clock();
    return NULL;
}