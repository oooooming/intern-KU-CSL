#ifdef __cplusplus
extern "C++" {
#include "glife.h"
}
#include <cuda.h>

// HINT: YOU CAN USE THIS METHOD FOR ERROR CHECKING
// Print error message on CUDA API or kernel launch
#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                    msg, cudaGetErrorString(__err), \
                    __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
        } \
    } while (0)

// TODO: YOU MAY NEED TO USE IT OR CREATE MORE
__device__ int getNeighbors(int* grid, int tot_rows, int tot_cols,
        int row, int col) {
    int numOfNeighbors = 0;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        int newRow = row + dx[i];
        int newCol = col + dy[i];
        
        if (newRow >= 0 && newRow < tot_rows && newCol >= 0 && newCol < tot_cols) {
            numOfNeighbors += grid[newRow * tot_cols + newCol]; // 1D for CUDA
        }
    }
    
    return numOfNeighbors;
}

// TODO: YOU NEED TO IMPLEMENT KERNEL TO RUN ON GPU DEVICE 
__global__ void kernel()
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y & blockDim.y + threadIdx.y;
    
    if (idx < tot_rows && idy < tot_cols) {
      int neighbors = getNeighbors(grid, tot_rows, tot_cols, idx, idy);
      
      // Game of Life rules
      if (grid[idx * tot_cols + idy] == 1) {
          if (neighbors == 2 || neighbors == 3)
              newGrid[idx * tot_cols + idy] = 1;
          else 
              newGrid[idx * tot_cols + idy] =0;
      } else {
          if (neighbors == 3)
              newGrid[idx * tot_cols + idy] = 1;
          else
              newGrid[idx * tot_cols + idy] = 0;
      }
    }
}

void cuda_dump(int *grid, int tot_rows, int tot_cols)
{
    printf("===============================\n");
    for (int i = 0; i < tot_rows; i++) {
        printf("[%d] ", i);
        for (int j = 0; j < tot_cols; j++) {
            if (grid[i * tot_cols + j])
                printf("*");
            else
                printf("o");
        }
        printf("\n");
    }
    printf("===============================\n");
}

void cuda_dump_index(int *grid, int tot_rows, int tot_cols)
{
    printf(":: Dump Row Column indices\n");
    for (int i = 0; i < tot_rows; i++) {
        for (int j = 0; j < tot_cols; j++) {
            if (grid[i * tot_cols + j])
                printf("%d %d\n", i, j);
        }
    }
}

// TODO: YOU NEED TO IMPLEMENT ON CUDA VERSION
uint64_t runCUDA(int rows, int cols, int gen, 
                 GameOfLifeGrid* g_GameOfLifeGrid, int display)
{
    cudaSetDevice(0); // DO NOT CHANGE THIS LINE 

    uint64_t difft;

    // ---------- TODO: CALL CUDA API HERE ----------
    
    // allocate GPU memory
    int* d_grid;
    int* d_newGrid;
    cudaMalloc(&d_grid, rows * cols * sizeof(int));
    cudaMalloc(&d_newGrid, rows * cols * sizeof(int));
    
    // transfer data from CPU to GPU
    cudaMemcpy(d_grid, g_GameOfLifeGrid->getGrid(), rows * cols * sizeof(int), cudaMemcpyHostToDevice);

    // Start timer for CUDA kernel execution
    difft = dtime_usec(0);
    // ----------  TODO: CALL KERNEL HERE  ----------

    // run CUDA kernel
    dim3 blockSize(16, 16);
    dim3 gridSize((cols + blockSize.x - 1) / blockSize.x, (rows + blockSize.y - 1) / blockSize.y);

    for (int i = 0; i < gen; i++) {
        kernel<<<gridSize, blockSize>>>(d_grid, d_newGrid, rows, cols);
        cudaCheckErrors("CUDA kernel launch failed");

        // update new state GPU -> GPU
        cudaMemcpy(d_grid, d_newGrid, rows * cols * sizeof(int), cudaMemcpyDeviceToDevice);
    }

    // Finish timer for CUDA kernel execution
    difft = dtime_usec(difft);

    // transfer data GPU to CPU
    cudaMemcpy(g_GameOfLifeGrid->getGrid(), d_grid, rows * cols * sizeof(int), cudaMemcpyDeviceToHost);

    // deallocate GPU memory
    cudaFree(d_grid);
    cudaFree(d_newGrid);

    // Print the results
    if (display) {
        cuda_dump();
        cuda_dump_index();
    }
    return difft;
}
#endif
