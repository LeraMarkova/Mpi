
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "mpi.h"



enum TAG { value, index, first_index };

void findMax(int* vector, int vector_size, int first, int &max, int &position) {
    max = vector[first]; position = first;
    for (int elemCounter = first + 1; elemCounter < first + vector_size; elemCounter++) {
        if (max < vector[elemCounter]) {
            max = vector[elemCounter];
            position = elemCounter;
        }
    }
}

int main(int argc, char* argv[])
{
    int procRank, procSize;
    const int n = 1000000;
    int *vec,
        maxValue = -1, maxPosition = -1,
        maxValueLocal = -1, maxPosLocal = -1;
    MPI_Status stat;
    double startTime, endTime;

    srand(time(0));
    vec = (int*)malloc(n*sizeof(int));
    for (int i = 0; i < n; i++) {
        vec[i] = rand();
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procSize);

    int step = n / procSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    printf("%d\n", procRank);

    if (procRank == 0)
    {

        /*startTime = MPI_Wtime();
        findMax(vec, n, 0, maxValue, maxPosition);
        endTime = MPI_Wtime();
        printf( "ONE PROCESS RESULT: max value is %d at %d position. Found in %f\n\n",
            maxValue, maxPosition, endTime - startTime);
            */
        startTime = MPI_Wtime();
        for (int i = 0; i < procSize - 1; i++)
        {
            MPI_Send(vec + i*step, step, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
        }

        findMax(vec, n - (procSize - 1)*step, (procSize - 1)*step, maxValueLocal, maxPosLocal);
        maxValue = maxValueLocal;
        maxPosition = maxPosLocal;
        //endTime = MPI_Wtime();
#ifdef _DEBUG
     //  fprintf(stdout, "proc %d reporting: max value is %d at %d position. Found in %f\n", procRank, maxValueLocal, maxPosLocal, endTime - startTime);

#endif

        for (int i = 0; i < procSize - 1; i++)
        {
            MPI_Recv(&maxValueLocal, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (maxValueLocal > maxValue)
            {
                maxValue = maxValueLocal;
                maxPosition = stat.MPI_TAG + (stat.MPI_SOURCE - 1)*step;

            }
        }
        /*  for (int i = 0; i < n; i++) {
              if (i == maxPosition) fprintf(stdout, "!!!");
              fprintf(stdout, "%d ", vec[i]);
          }
          fprintf(stdout, "\n");*/
          //endTime = MPI_Wtime();

        endTime = MPI_Wtime();
    }
       
        
    
    else {
        int* buf, maxLocal = -1, maxPosLocal = -1;
        buf = (int*)malloc(sizeof(int)*(step));
        double startTime = MPI_Wtime(), endTime;
        //fprintf(stdout, "got memory size %d\n", step + 1);
        MPI_Recv(buf, step, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
        findMax(buf, step, 0, maxLocal, maxPosLocal);
        //fprintf(stdout, "recieved buf %d\n", step + 1);
        for (int i = 0; i < step + 1; i++) {
           // fprintf(stdout, "%d ", buf[i]);
        }
        //fprintf(stdout, "\n");
        endTime = MPI_Wtime();
        //fprintf(stdout, "proc %d reporting: max value is %d at %d position. Found in %f\n", procRank, maxLocal, maxPosLocal, endTime - startTime);

        MPI_Send(&maxLocal, 1, MPI_INT, 0, maxPosLocal, MPI_COMM_WORLD);
        free(buf);
    }

    printf("%d PROCESSES RESULT: max value is %d at %d position. Found in %f\n",
        procRank, maxValue, maxPosition, endTime - startTime);
    MPI_Finalize();
    free(vec);
    return 0;
}
