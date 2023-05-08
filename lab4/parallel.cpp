#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include "mpi.h"


//#define DEBUG_ON

#include "debug.h"


#define SLAVE_NUM (PROC_NUM - 1)  // number of slaves
#define MASTER_ID SLAVE_NUM

/*
 * Buffer Layout:
 *  ▲   +--------------------------------------------------+
 *  |   |                        ...                       |  <-- side_buffer_up            (SIDE_BUFFER_WIDTH)
 *  |   +--------------------------------------------------+ -<-- partition_begin_index
 * buf  |                      .......                     |                                 (BLOCK_SIZE)
 *  |   +--------------------------------------------------+ -<-- partition_end_index
 *  |   |                        ...                       |  <-- side_buffer_down          (SIDE_BUFFER_WIDTH)
 *  ▼   +--------------------------------------------------+
 */

#define BLOCK_SIZE (MAT_DIM / SLAVE_NUM)  // partition size
#define SIDE_BUFFER_WIDTH ((KERNEL_DIM - 1) / 2)  // the width of side buffers, i.e. (total buffer width - partition width) / 2

#define UPPER_BUFFER_BEGIN_INDEX 0  // inclusive
#define UPPER_BUFFER_END_INDEX (SIDE_BUFFER_WIDTH - 1)  // inclusive

#define LOWER_BUFFER_BEGIN_INDEX (SIDE_BUFFER_WIDTH + BLOCK_SIZE)  // inclusive
#define LOWER_BUFFER_END_INDEX (2 * SIDE_BUFFER_WIDTH + BLOCK_SIZE - 1)  // inclusive

#define PARTITION_BEGIN_INDEX (SIDE_BUFFER_WIDTH)  // inclusive
#define PARTITION_END_INDEX (SIDE_BUFFER_WIDTH + BLOCK_SIZE - 1)  // inclusive

#define PARTITION_SIZE (sizeof (double) * MAT_DIM * BLOCK_SIZE)  // size of partition

#define SIDE_BUFFER_SIZE (sizeof (double) * MAT_DIM * SIDE_BUFFER_WIDTH)  // size of side buffer


/*
 * GLOBAL VARIABLES
 */
int NUM_ITER = 10;

int MAT_DIM = 1000;
int KERNEL_DIM = 3;

int PROC_NUM = 4;


// ****************************** ProcData Definition  *********************************************
typedef struct ProcData {
    int id;

    // buffers are 2-D arrays in 1-D format
    double *buffer[2]; // 2 buffers, one for reading, one for writing, switch alternately

    double *RO_buffer_ptr; // rows(i)=buffer_width, cols(j)=MAT_DIM, Read Only, stores previous results
    double *WO_buffer_ptr; // rows(i)=buffer_width, cols(j)=MAT_DIM, Write Only, stores temporary results
} ProcData;

// ****************************** ProcData Methods Begin  *********************************************

/*
 * Description: initialize ProcData, pseudo constructor
 */
ProcData init_proc_data(int id) {
    ProcData proc_data;
    proc_data.id = id;

    // allocate memory for buffers
#pragma omp parallel for num_threads(2) shared(proc_data)
    for (int i = 0; i < 2; i++) {
        proc_data.buffer[i] = (double *) malloc(sizeof(double) * MAT_DIM * (2 * SIDE_BUFFER_WIDTH + BLOCK_SIZE));
        memset(proc_data.buffer[i], 0, sizeof(double) * MAT_DIM * (2 * SIDE_BUFFER_WIDTH + BLOCK_SIZE));
    }

    // assign initial buffer pointers
    proc_data.RO_buffer_ptr = proc_data.buffer[0];
    proc_data.WO_buffer_ptr = proc_data.buffer[1];

    return proc_data;
}

/*
 * Description: get data from the matrix or the buffer, buffer_cols is j max
 */
double read_buffer(double *buffer, int buffer_cols, int i, int j) {
    return buffer[i * buffer_cols + j];
}

void write_buffer(double *buffer, int buffer_cols, int i, int j, double value) {
    buffer[i * buffer_cols + j] = value;
}

/*
 * Description: i, j are referring to the partitioned section, side buffer is excluded
 *              i in range [- SIDE_BUFFER_WIDTH, BLOCK_SIZE + SIDE_BUFFER_WIDTH - 1]
 *              j in range [0 - SIDE_BUFFER_WIDTH, MAT_DIM + SIDE_BUFFER_WIDTH - 1]
 */
double get_data(const ProcData &proc_data, int i, int j) {
    if (j < 0 || j >= MAT_DIM) // padding
    {
        return 0;
    }

    int _i = i + PARTITION_BEGIN_INDEX;
    int _j = j;  // the column index remains the same
    return read_buffer(proc_data.RO_buffer_ptr, MAT_DIM, _i, _j);
}

/*
 * Description: i, j are referring to the partitioned section, side buffer is excluded
 *              i in range [- SIDE_BUFFER_WIDTH, BLOCK_SIZE + SIDE_BUFFER_WIDTH - 1]
 *              j in range [0 - SIDE_BUFFER_WIDTH, MAT_DIM + SIDE_BUFFER_WIDTH - 1]
 */
bool set_data(const ProcData &proc_data, int i, int j, double value) {
    if (i >= -SIDE_BUFFER_WIDTH && i < BLOCK_SIZE + SIDE_BUFFER_WIDTH && j >= 0 &&
        j < MAT_DIM) // in the partitioned section{
    {
        int _i = i + PARTITION_BEGIN_INDEX;
        int _j = j;  // the column index remains the same
        write_buffer(proc_data.WO_buffer_ptr, MAT_DIM, _i, _j, value);
        return true;
    }
    return false;
}

/*
 * Description: swap the RO and WO buffer
 * */
void swap_buffer(ProcData &proc_data) {
    double *tmp = proc_data.RO_buffer_ptr;
    proc_data.RO_buffer_ptr = proc_data.WO_buffer_ptr;
    proc_data.WO_buffer_ptr = tmp;
}

double *get_buffer_begin_ptr(double *buffer, int i, int j) {
    return buffer + i * MAT_DIM + j;
}

/*
 * MPI_Sendrecv:
 * Documentation: https://learn.microsoft.com/en-us/message-passing-interface/mpi-sendrecv-function
 * Parameters:
 * int MPIAPI MPI_Sendrecv(
      _In_  void         *sendbuf,   // initial address of send buffer
            int          sendcount,  // number of elements in send buffer
            MPI_Datatype sendtype,
            int          dest,       // rank of destination
            int          sendtag,
      _Out_ void         *recvbuf,   // initial address of receive buffer
            int          recvcount,  // number of elements in receive buffer
            MPI_Datatype recvtype,   // type of elements in receive buffer
            int          source,     // rank of source
            int          recvtag,    // message tag
            MPI_Comm     comm,
      _Out_ MPI_Status   *status
    );
 */

/*
 * Description: Sync buffer using MPI_Sendrecv, i.e. Jacobian method
 */
void sync_buffer(ProcData &proc_data) {
    int id = proc_data.id;
    int upper_neighbor = (id == 0) ? MPI_PROC_NULL : id - 1;
    int lower_neighbor = (id == SLAVE_NUM - 1) ? MPI_PROC_NULL : id + 1;

    // send [PARTITION_BEGIN_INDEX, PARTITION_BEGIN_INDEX + SIDE_BUFFER_WIDTH - 1] to upper neighbor, receive upper buffer
    MPI_Sendrecv(
            get_buffer_begin_ptr(proc_data.WO_buffer_ptr, PARTITION_BEGIN_INDEX, 0), SIDE_BUFFER_SIZE,
            MPI_UNSIGNED_CHAR,
            upper_neighbor, 0,
            get_buffer_begin_ptr(proc_data.WO_buffer_ptr, PARTITION_BEGIN_INDEX - SIDE_BUFFER_WIDTH, 0),
            SIDE_BUFFER_SIZE, MPI_UNSIGNED_CHAR,
            upper_neighbor, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // send [PARTITION_END_INDEX - SIDE_BUFFER_WIDTH + 1, PARTITION_END_INDEX] to lower neighbor, receive lower buffer
    MPI_Sendrecv(
            get_buffer_begin_ptr(proc_data.WO_buffer_ptr, PARTITION_END_INDEX - SIDE_BUFFER_WIDTH + 1, 0),
            SIDE_BUFFER_SIZE, MPI_UNSIGNED_CHAR,
            lower_neighbor, 0,
            get_buffer_begin_ptr(proc_data.WO_buffer_ptr, PARTITION_END_INDEX + 1, 0), SIDE_BUFFER_SIZE,
            MPI_UNSIGNED_CHAR,
            lower_neighbor, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // swap buffer
    swap_buffer(proc_data);
}

/*
 * Called by Slave
 */
void submit_result(ProcData &proc_data) {
    const double *buf_begin = get_buffer_begin_ptr(proc_data.RO_buffer_ptr, PARTITION_BEGIN_INDEX, 0);
    MPI_Send(buf_begin, PARTITION_SIZE, MPI_UNSIGNED_CHAR, MASTER_ID, 0, MPI_COMM_WORLD);
}

/*
 * Called by Master
 * Input: result buffer, a 2-D array in 1-D format
 * */
void gather_result(double *result) {
    // receive from MPI workers
//#pragma omp parallel for num_threads(SLAVE_NUM)
// TODO: tag_match.c:61   UCX  WARN  unexpected tag-receive descriptor 0x40002d501fc0 was not matched
    for (int i = 0; i < SLAVE_NUM; i++)
    {
        DEBUG(TraceableInfo("Gathering result from slave %d\n", i);)
        // get MPI_Status
        MPI_Status status;
        MPI_Recv(get_buffer_begin_ptr(result, i * BLOCK_SIZE, 0), PARTITION_SIZE, MPI_UNSIGNED_CHAR, i, 0,
                 MPI_COMM_WORLD, &status);
        DEBUG(TraceableInfo("Gathered result from slave %d\n", i);)
    }
}


void print_buffer(const ProcData &proc_data) {
    printf("%d========================================================\n", proc_data.id);
    for (int i = 0; i < BLOCK_SIZE + 2 * SIDE_BUFFER_WIDTH; i++) {
        if (i == PARTITION_BEGIN_INDEX || i == PARTITION_END_INDEX + 1) {
            printf("%d------------------------------------\n", proc_data.id);
        }

        printf("%d-%d: ", proc_data.id, i);
        for (int j = 0; j < MAT_DIM; j++) {
            printf("%lf\t", proc_data.RO_buffer_ptr[i * MAT_DIM + j]);  // directly access the buffer
        }
        printf("\n");
    }
    printf("%d========================================================\n", proc_data.id);
}
// ****************************** ProcData Methods End  *********************************************


// Test MPI_Sendrecv
int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: <N> <matrix dimension> <kernel dimension>");
        exit(1);
    }

    NUM_ITER = atoi(argv[1]);
    MAT_DIM = atoi(argv[2]);
    KERNEL_DIM = atoi(argv[3]);

    /// read inputs
    double matrix[MAT_DIM][MAT_DIM];
    double kernel[KERNEL_DIM][KERNEL_DIM];
    // read in the matrix from matrix.txt
    std::ifstream mat_file("matrix.txt");
    // TODO: do not alter here
//#pragma omp parallel for collapse(2) shared(matrix, mat_file)
    for (int i = 0; i < MAT_DIM; i++)
    {
        for (int j = 0; j < MAT_DIM; j++)
        {
            mat_file >> matrix[i][j];
//            DEBUG(TraceableInfo("matrix[%d][%d]=%lf\n", i, j, matrix[i][j]);)
        }
    }
    mat_file.close();

    DEBUG(Info("matrix read in\n");)

    // read in the kernel from kernel.txt
    std::ifstream kernel_file("kernel.txt");
    // TODO: do not alter here
//#pragma omp parallel for collapse(2)
    for (int i = 0; i < KERNEL_DIM; i++) {
        for (int j = 0; j < KERNEL_DIM; j++) {
            kernel_file >> kernel[i][j];
        }
    }
    kernel_file.close();

    DEBUG(Info("kernel read in\n");)

    // initialize MPI
    MPI_Init(&argc, &argv);

    int my_id;

    MPI_Comm_size(MPI_COMM_WORLD, &PROC_NUM);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    double *result = NULL;
    if (my_id == MASTER_ID) {
        DEBUG(Info("Master process started\n");)
        result = new double[MAT_DIM * MAT_DIM];

    }

    DEBUG(Info("my_id: %d, proc_num: %d\n", my_id, PROC_NUM);)

    if (my_id != MASTER_ID) {
        DEBUG(Info("PARTITION_BEGIN_INDEX: %d, PARTITION_END_INDEX: %d\n", PARTITION_BEGIN_INDEX, PARTITION_END_INDEX);)

        ProcData proc_data = init_proc_data(my_id);

        // copy the matrix to the buffer
        int i_offset = my_id * BLOCK_SIZE;

#pragma omp parallel for collapse(2)
        for (int i = -SIDE_BUFFER_WIDTH; i <= BLOCK_SIZE + SIDE_BUFFER_WIDTH - 1; i++)
        {
            for (int j = 0; j < MAT_DIM; j++)
            {
                // skip side buffer
                if (i < 0 || i >= BLOCK_SIZE)
                {
                    continue;
                }
//                DEBUG(TraceableInfo("thread %d, i=%d, j=%d\n", omp_get_thread_num(), i, j);)
                Assert(set_data(proc_data, i, j, matrix[i_offset + i][j]) == true, "set_data failed, (%d,%d)=%lf\n", i,
                       j, matrix[i_offset + i][j]);
            }
        }

        // convolution
        for (int n = 0; n < NUM_ITER; n++)
        {
            // sync first
            sync_buffer(proc_data);
            // calculation
#pragma omp parallel for collapse(2)
            for (int i = 0; i < BLOCK_SIZE; i++)
            {
                for (int j = 0; j < MAT_DIM; j++)
                {
                    double sum = 0;
#pragma omp parallel for collapse(2) reduction(+:sum)
                    for (int _i = -SIDE_BUFFER_WIDTH; _i <= SIDE_BUFFER_WIDTH; _i++)
                    {
                        for (int _j = -SIDE_BUFFER_WIDTH; _j <= SIDE_BUFFER_WIDTH; _j++)
                        {
                            sum += get_data(proc_data, i + _i, j + _j) *
                                   kernel[_i + SIDE_BUFFER_WIDTH][_j + SIDE_BUFFER_WIDTH];
                        }
                    }
                    set_data(proc_data, i, j, sum);
                }
            }
        }

        swap_buffer(proc_data);  // swap the buffer, so that the result is in the RO_buffer_ptr
        // submit
        DEBUG(Info("slave %d submitting...\n", my_id);)
        submit_result(proc_data);
        DEBUG(Info("slave %d submitted\n", my_id);)
    } // end worker process

    if (my_id == MASTER_ID) {
        DEBUG(Info("Master process gathering...\n");)
        gather_result(result);
        DEBUG(Info("Master process gathered\n");)
        DEBUG(
        // DEBUG print result
                std::ofstream result_file("result.txt");
                for (int i = 0; i < MAT_DIM; i++) {
                    for (int j = 0; j < MAT_DIM; j++) {
                        result_file << result[i * MAT_DIM + j] << "\t";
                        std::cout << result[i * MAT_DIM + j] << "\t";
                    }
                    result_file << "\n";
                    std::cout << "\n";
                }
        )
        DEBUG(Info("Master process ended\n");)
    }

    MPI_Finalize();
    DEBUG(Info("process %d finalized\n", my_id);)
    return 0;
}