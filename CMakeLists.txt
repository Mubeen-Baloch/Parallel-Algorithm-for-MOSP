cmake_minimum_required(VERSION 3.10)
project(PDC_Project)

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(mpi_openmp_test src/mpi_openmp_test.cpp)

target_link_libraries(mpi_openmp_test MPI::MPI_CXX OpenMP::OpenMP_CXX)

