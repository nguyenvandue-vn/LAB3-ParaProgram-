#include<iostream>
#include <vector>
#include<stdlib.h>
#include<mpi.h>
#include<time.h>

using namespace std;

void writeMatrix(const char* path, int size) {
    FILE* f;
    fopen_s(&f, path, "w");
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            fprintf(f, "%5d", rand() % 5);
    fclose(f);
}

int main(int argc, char* argv[]){
    srand(time(NULL));
    double start, stop;
    int rows;
    int N = 100;
    int rank, numprocs;

    MPI_Init(NULL, NULL);//MPI Initialize
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);// Получить текущий номер процесса
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);// Получить количество процессов
    rows = N/ numprocs;
    
    int* matrixA = (int*)malloc(sizeof(int) * N * N);
    int* matrixB = (int*)malloc(sizeof(int) * N * N);
    int* matrixC = (int*)malloc(sizeof(int) * N * N);
    int* buffer = (int*)malloc(sizeof(int) * N * rows);
    int* result = (int*)malloc(sizeof(int) * N * N);

    writeMatrix("C:/Users/vuong/Desktop/inputA.txt", N);

    writeMatrix("C:/Users/vuong/Desktop/inputB.txt", N);

    // Основной процесс присваивает матрице начальное значение и передает матрицу N каждому процессу, а матрицу M передает каждому процессу в группах.
    if (rank == 0) {
        start = MPI_Wtime();
        FILE* f;
        //read and print matrix A
        fopen_s(&f, "C:/Users/vuong/Desktop/inputA.txt", "r");
        if (f != NULL) {
            printf("Matrix A in file:\n");
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    fscanf_s(f, "%d", &matrixA[i * N + j]);
                    printf("%3d", matrixA[i * N + j]);
                }
                printf("\n");
            }
            fclose(f);
        }
        //read and print matrix B
        fopen_s(&f, "C:/Users/vuong/Desktop/inputB.txt", "r");
        if (f != NULL) {
            printf("Matrix B in file:\n");
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    fscanf_s(f, "%d", &matrixB[i * N + j]);
                    printf("%3d", matrixB[i * N + j]);
                }
                printf("\n");
            }
            fclose(f);
        }

        // Отправить матрицу B другим подчиненным процессам
        for (int i = 1; i < numprocs; i++)         
            MPI_Send(matrixB, N * N, MPI_INT, i, 0, MPI_COMM_WORLD);
        for (int i = 1; i < numprocs; i++)
            MPI_Send(matrixA+(i-1)*rows*N, N * N, MPI_INT, i, 1, MPI_COMM_WORLD);
        for (int k = 1; k < numprocs; k++) {          
            MPI_Recv(matrixC,rows * N, MPI_INT, k, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    result[((k - 1) * N + i) * N + j] = matrixC[i * N + j];
        }
        for (int i = (numprocs - 1) * rows; i < N; i++){
            for (int j = 0; j < N; j++) {
                result[i * N + j] = 0;
                for (int k = 0; k < N; k++)
                    result[i * N + j] += matrixA[i * N + k] * matrixB[k * N + j];
            }
        }

        //write result
        fopen_s(&f, "C:/Users/vuong/Desktop/output.txt", "w");
        for (int i = 0; i < N; ++i){
            for (int j = 0; j < N; ++j)
                fprintf(f, "%5d", result[i * N + j]);
        }
        fclose(f);

        //print result
        cout << "Matrix AxB:\n";
        for (int i = 0; i < N; ++i){
            for (int j = 0; j < N; ++j)
                printf("%5d", result[i * N + j]);  
            printf("\n");
        }

        stop = MPI_Wtime();

        cout <<"\nTime:"<< stop - start;

        free(matrixA);
        free(matrixB);
        free(matrixC);
        free(buffer);
        free(result);
    }

    // Другие процессы получают данные и после вычисления результата отправляют их в основной процесс
    else {
        // Получаем широковещательные данные (матрица b)
        MPI_Recv(matrixB, N * N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(buffer, rows * N, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //mulMatrix
        for (int i = 0; i <rows; ++i){
            for (int j = 0; j < N; ++j){
                matrixC[i * N + j] = 0;
                for (int k = 0; k < N; k++) 
                    matrixC[i * N + j] += (buffer[i * N + k] * matrixB[k * N + j]);
            }
        }
        // Отправить результат расчета в основной процесс
        MPI_Send(matrixC, rows * N, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();//Конец

    return 0;
}
