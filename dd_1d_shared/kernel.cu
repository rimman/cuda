#include "kernel.h"
#define TPB 64
#define RAD 1 // radius of the stencil

///////////////////////////////////////////////////////
// Note: Setting size of the Shared Array
// if you createyour shared array with a fixed size, the array can be created as follows:

// __shared__ float s_in[34];
//
// and not change to the kernel call is needed. Note that teh following will produce a compiler error:
//
// __shared__ float s_in[blockDim.x + 2 * RAD];
//
// if you allocate the array dynamically, the declaration requires the keyword "extern" as follows:
//
// extern __shared__ float s_in[];
//
// And the kernel call requires an optional third argument within the chevrons to specify the size
// of the shared memory allocation in bytes.
// 
// const size_t smemSize = (TPB + 2 * RAD) * sizeof(float);
// ddKernel<<<n+TPB-1)/TPB, TPB, smemSize>>>(d_out, d_in, n, h);

__global__
void ddKernel(float *d_out, const float *d_in, int size, float h) 
{
    // compute "global index" i
    const int i = threadIdx.x + blockDim.x * blockIdx.x;
  
    if (i >= size) return;
    
    const int s_idx = threadIdx.x + RAD;
    
    //declare shared array
    extern __shared__ float s_in[];

    // Regular cells 
    // each thread requests the entry in the input array whose index matches 
    // the thread's global index and stores the value int eh shared array at the entry corresponding to the local index)
    s_in[s_idx] = d_in[i];

    // Halo cells
    if (threadIdx.x < RAD) 
    {
        // careful: the two lines below will also access d_in[-1] and d_in[size+1] which 
        // are undefined! This bug is fixed in heat_2d (cf. idxClip function)
    
        s_in[s_idx - RAD] = d_in[i - RAD];
        s_in[s_idx + blockDim.x] = d_in[i + blockDim.x];
    }
  
    //Kernel launches are async. We cannot assume that all of the input data
    //has been loaded into the shared memory array before threads execute the
    //final statement. To ensure that all the data has been properly stored,
    //call __syncthreads(), which forces all the threads in the block to complete
    //the previous statements ebfore any thread in the block poceeds further.
    __syncthreads();
    d_out[i] = (s_in[s_idx-1] - 2.f*s_in[s_idx] + s_in[s_idx+1])/(h*h);
}

void ddParallel(float *out, const float *in, int n, float h) 
{
  float *d_in = 0, *d_out = 0;
  cudaMalloc(&d_in, n * sizeof(float));
  cudaMalloc(&d_out, n * sizeof(float));
  cudaMemcpy(d_in, in, n * sizeof(float), cudaMemcpyHostToDevice);

  // Set shared memory size in bytes
  const size_t smemSize = (TPB + 2 * RAD) * sizeof(float);

  ddKernel<<<(n + TPB - 1)/TPB, TPB, smemSize>>>(d_out, d_in, n, h);

  cudaMemcpy(out, d_out, n * sizeof(float), cudaMemcpyDeviceToHost);

  cudaFree(d_in);
  cudaFree(d_out);
}