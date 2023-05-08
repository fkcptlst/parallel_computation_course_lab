#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include "assert.h"

using namespace std;

const int MAX_SIZE = 1000;
const int MAX_MAT_DIM = 1000;
const int MAX_KERNEL_DIM = 9;

int N; // number of iterations
int mat_dim, kernel_dim;
int T;              // number of threads
int BLOCK_SIZE = 0; // block size

int matrix[MAX_MAT_DIM][MAX_MAT_DIM];
int kernel[MAX_KERNEL_DIM][MAX_KERNEL_DIM];
int tmp[MAX_MAT_DIM][MAX_MAT_DIM]; // middle result

typedef struct Bstate
{
    pthread_mutex_t barrier_mutex;
    pthread_cond_t barrier_cond;
    int nthread; // Number of threads that have reached this round of the barrier
    Bstate()
    {
        assert(pthread_mutex_init(&barrier_mutex, NULL) == 0);
        assert(pthread_cond_init(&barrier_cond, NULL) == 0);
        nthread = 0;
    }
} Bstate;

Bstate stage_1, stage_2;

void barrier(Bstate &bstate)
{
    pthread_mutex_lock(&bstate.barrier_mutex);

    bstate.nthread++;

    if (bstate.nthread == T) // all reached barrier
    {
        bstate.nthread = 0;
        pthread_cond_broadcast(&bstate.barrier_cond);
    }
    else
    {
        pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
    }
    pthread_mutex_unlock(&bstate.barrier_mutex);
}

typedef struct Thread_data
{
    int thread_id;
    int partition_begin_index; // inclusive, marks the first row of the block, equals to kernel_dim - 1
    int partition_end_index;   // inclusive, marks the last row of the block, equals to partition_begin_index + BLOCK_SIZE - 1
    int partition_size;        // equals to partition_end_index - partition_begin_index + 1
    int buffer_width;          // equals to (kernel_dim - 1) / 2

    vector<vector<double>> side_buffer_up;   // upper side buffer
    vector<vector<double>> side_buffer_down; // lower side buffer

    Thread_data(int thread_id, int partition_begin_index, int partition_end_index) : thread_id(thread_id), partition_begin_index(partition_begin_index), partition_end_index(partition_end_index)
    {
        partition_size = partition_end_index - partition_begin_index + 1;
        // the width of buffers is (kernel_dim - 1) / 2, the height is mat_dim + 2*((kernel_dim - 1) / 2)
        buffer_width = (kernel_dim - 1) / 2;
        side_buffer_up.resize(buffer_width);
        side_buffer_down.resize(buffer_width);
        for (int i = 0; i < buffer_width; i++)
        {
            // side_buffer_up[i].resize(mat_dim + 2 * buffer_width); // buffer_width is also the padding dimension
            // side_buffer_down[i].resize(mat_dim + 2 * buffer_width);
            side_buffer_up[i].resize(mat_dim); // buffer_width is also the padding dimension
            side_buffer_down[i].resize(mat_dim);
        }
    }

    void print_buffer()
    {
        printf("printing upper buffer\n");
        for(int i = 0; i < side_buffer_up.size(); i++)
        {
            for(int j = 0; j < side_buffer_up[i].size(); j++)
            {
                printf("%lf\t", side_buffer_up[i][j]);
            }
            printf("\n");
        }
        
        printf("printing lower buffer\n");
        for(int i = 0; i < side_buffer_down.size(); i++)
        {
            for(int j = 0; j < side_buffer_down[i].size(); j++)
            {
                printf("%lf\t", side_buffer_down[i][j]);
            }
            printf("\n");
        }
    }

    double get_data(int i, int j)
    {
        if (j < 0 || j >= mat_dim) // padding
        {
            return 0;
        }
        else if (i < 0 || i >= partition_size) // use jacobian buffer
        {
            if (i < 0) // upper buffer
            {
                return side_buffer_up[buffer_width + i][j];
            }
            else // lower buffer
            {
                return side_buffer_down[i - partition_size][j];
            }
        }
        else // within the block partition
        {
            return matrix[i + partition_begin_index][j];
        }
    }

    bool set_data(int i, int j, double value)
    {
        if (i >= 0 && i < partition_size && j >= 0 && j < mat_dim) // within the block partition
        {
            tmp[i + partition_begin_index][j] = value;
            return true;
        }
        else
        {
            return false;
        }
    }
} Thread_data;

Thread_data *thread_data_array[MAX_SIZE]; // array pointer to Thread_data

void send_recv(Thread_data &thread_data)
{
    // barrier
    static bool initialized = false;
    static int arrived_threads = 0;

    // printf("thread %d called send_recv\n", thread_data.thread_id);

    barrier(stage_1);

    // printf("thread %d awake 1\n", thread_data.thread_id);

    // send result to the global matrix variable
    for (int i = 0; i < thread_data.partition_size; i++)
    {
        for (int j = 0; j < mat_dim; j++)
        {
            matrix[i + thread_data.partition_begin_index][j] = tmp[i + thread_data.partition_begin_index][j];
        }
    }
    
    // printf("thread %d send complete\n", thread_data.thread_id);

    barrier(stage_2);

    // printf("thread %d awake 2\n", thread_data.thread_id);

    // recv data to the up and down, matrix is now read only
    for (int i = 0; i < thread_data.buffer_width; i++)
    {
        for (int j = 0; j < mat_dim; j++)
        {
            if (thread_data.partition_begin_index - thread_data.buffer_width + i < 0) // uppermost partition
            {
                // side_buffer_up[i][j] = 0;
                thread_data.side_buffer_up[i][j] = 0;
            }
            else
            {
                // side_buffer_up[i][j] = matrix[thread_data.partition_begin_index - thread_data.buffer_width + i][j];
                thread_data.side_buffer_up[i][j] = matrix[thread_data.partition_begin_index - thread_data.buffer_width + i][j];
            }

            if (thread_data.partition_end_index + 1 + i >= mat_dim) // lowermost partition
            {
                // side_buffer_down[i][j] = 0;
                thread_data.side_buffer_down[i][j] = 0;
            }
            else
            {
                // side_buffer_down[i][j] = matrix[thread_data.partition_end_index + 1 + i][j];
                thread_data.side_buffer_down[i][j] = matrix[thread_data.partition_end_index + 1 + i][j];
            }
        }
    }
}

void *thread_function(void *arg);

int main(int arg, char *argv[])
{
    if (arg != 5)
    {
        printf("Usage: parallel.exe <N> <matrix dimension> <kernel dimension> <number of threads>");
        exit(1);
    }

    N = atoi(argv[1]);
    mat_dim = atoi(argv[2]);
    kernel_dim = atoi(argv[3]);
    T = atoi(argv[4]);

    BLOCK_SIZE = mat_dim / T;

    // printf("row_block_num = %d, col_block_num = %d\n", row_block_num, col_block_num);

    pthread_t threads[T];

    // read in the matrix from matrix.txt
    std::ifstream mat_file("matrix.txt");
    for (int i = 0; i < mat_dim; i++)
    {
        for (int j = 0; j < mat_dim; j++)
        {
            mat_file >> matrix[i][j];
        }
    }
    mat_file.close();

    // printf("matrix read in\n");

    // read in the kernel from kernel.txt
    std::ifstream kernel_file("kernel.txt");
    for (int i = 0; i < kernel_dim; i++)
    {
        for (int j = 0; j < kernel_dim; j++)
        {
            kernel_file >> kernel[i][j];
        }
    }
    kernel_file.close();

    // printf("kernel read in\n");

    int thread_id[T];
    for (int t = 0; t < T; t++)
    {
        thread_id[t] = t;

        thread_data_array[t] = new Thread_data(t, t * BLOCK_SIZE, (t + 1) * BLOCK_SIZE - 1);

        // printf("thread %d, begin_row = %d, end_row = %d \n", t, thread_data_array[t]->partition_begin_index, thread_data_array[t]->partition_end_index);
        pthread_create(&threads[t], NULL, thread_function, &thread_id[t]);
    }

    // wait for all threads to finish
    for (int t = 0; t < T; t++)
    {
        pthread_join(threads[t], NULL);
    }

    // printf("done!\n");

//     // write result to result_parallel.txt
//     std::ofstream result_file("result_parallel.txt");
//     for (int i = 0; i < mat_dim; i++)
//     {
//         for (int j = 0; j < mat_dim; j++)
//         {
//             result_file << matrix[i][j] << "\t";
//         }
//         result_file << std::endl;
//     }
}

void *thread_function(void *arg)
{
    int id = *(int *)arg;


    Thread_data &thread_data = *thread_data_array[id];

    // initialize side buffer first
    for (int i = 0; i < thread_data.buffer_width; i++)
    {
        for (int j = 0; j < mat_dim; j++)
        {
            if (thread_data.partition_begin_index - thread_data.buffer_width + i < 0) // uppermost partition
            {
                // side_buffer_up[i][j] = 0;
                thread_data.side_buffer_up[i][j] = 0;
            }
            else
            {
                // side_buffer_up[i][j] = matrix[thread_data.partition_begin_index - thread_data.buffer_width + i][j];
                thread_data.side_buffer_up[i][j] = matrix[thread_data.partition_begin_index - thread_data.buffer_width + i][j];
            }

            if (thread_data.partition_end_index + 1 + i >= mat_dim) // lowermost partition
            {
                // side_buffer_down[i][j] = 0;
                thread_data.side_buffer_down[i][j] = 0;
            }
            else
            {
                // side_buffer_down[i][j] = matrix[thread_data.partition_end_index + 1 + i][j];
                thread_data.side_buffer_down[i][j] = matrix[thread_data.partition_end_index + 1 + i][j];
            }
        }
    }

    int offset = (kernel_dim - 1) / 2;
    for (int n = 0; n < N; n++)
    {
        // printf("round %d ========================================\n", n);
        // print margin
        // printf("thread %d, iteration %d, get_data(0,-1) %lf\n", id, n, thread_data.get_data(0, -1));
        // printf("thread %d, iteration %d, get_data(-1,0) %lf\n", id, n, thread_data.get_data(-1, 0));
        // thread_data.print_buffer();
        // convolution
        for (int i = 0; i < thread_data.partition_size; i++)
        {
            for (int j = 0; j < mat_dim; j++)
            {
                double value = 0;
                for (int i_ = 0; i_ < kernel_dim; i_++)
                {
                    for (int j_ = 0; j_ < kernel_dim; j_++)
                    {
                        value += kernel[i_][j_] * thread_data.get_data(i + i_ - offset, j + j_ - offset);
                    }
                }
                // update
                thread_data_array[id]->set_data(i, j, value);
            }
        }

        // iteration finished, send data to other threads
        send_recv(thread_data); // barrier
    }

    return NULL;
}