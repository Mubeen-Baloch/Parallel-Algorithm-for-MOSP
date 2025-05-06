#include <mpi.h>
#include <omp.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();
        std::cout << "Hello from thread " << thread_id 
                  << " of " << total_threads 
                  << " in process " << world_rank 
                  << " of " << world_size << std::endl;
    }

    MPI_Finalize();
    return 0;
}

//mpic++ -fopenmp src/mpi_openmp_test.cpp -o mpi_openmp_test
//mpirun -np 2 ./mpi_openmp_test
