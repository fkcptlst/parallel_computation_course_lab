#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

int mat_dim, kernel_dim;
int N;

int main(int arg, char *argv[])
{
    if (arg != 4)
    {
        printf("Usage: serial.exe <N> <matrix dimension> <kernel dimension>");
        exit(1);
    }

    N = atoi(argv[1]);
    mat_dim = atoi(argv[2]);
    kernel_dim = atoi(argv[3]);

    int matrix[mat_dim + 2][mat_dim + 2];
    int kernel[kernel_dim][kernel_dim];
    int result[mat_dim][mat_dim];
    // int result_tmp[mat_dim][mat_dim];

    // read in the matrix from matrix.txt
    std::ifstream mat_file("matrix.txt");
    for (int i = 0; i < mat_dim + 2; i++)
    {
        for (int j = 0; j < mat_dim + 2; j++)
        {
            matrix[i][j] = 0;
        }
    }
    for (int i = 1; i < mat_dim + 1; i++)
        for (int j = 1; j < mat_dim + 1; j++)
            mat_file >> matrix[i][j];
    mat_file.close();

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

    // initialize result
    for (int i = 0; i < mat_dim; i++)
    {
        for (int j = 0; j < mat_dim; j++)
        {
            result[i][j] = 0;
            // result_tmp[i][j] = 0;
        }
    }

    // convolution
    for (int n = 0; n < N; n++)
    {
        for (int i = 0; i < mat_dim; i++)
        {
            for (int j = 0; j < mat_dim; j++)
            {
                double value = 0;
                for (int i_ = 0; i_ < kernel_dim; i_++)
                {
                    for (int j_ = 0; j_ < kernel_dim; j_++)
                    {
                        value += matrix[i + i_][j + j_] * kernel[i_][j_];
                    }
                }
                result[i][j] = value;
            }
        }

        // copy result to matrix
        for (int i = 0; i < mat_dim; i++)
        {
            for (int j = 0; j < mat_dim; j++)
            {
                matrix[i + 1][j + 1] = result[i][j];
            }
        }
    }

    // write result to result.txt
    std::ofstream result_file("result_serial.txt");
    for (int i = 0; i < mat_dim; i++)
    {
        for (int j = 0; j < mat_dim; j++)
        {
            result_file << result[i][j] << "\t";
        }
        result_file << std::endl;
    }
}