#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <solver.h>

#define NWITEMS 512
/* A simple memset kernel */
const char *source =
  "__kernel void memset( __global uint *dst )                              \n"
  "{                                                                       \n"
  " dst[get_global_id(0)] = get_global_id(0);                              \n"
  "} \n";

cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_mem buffer;
size_t global_work_size;

int solver_ResolveOpencl(LB_Lattice_p lattice)
{
  int nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  cl_command_queue queue;
  
  queue = clCreateCommandQueue(context,
                               device,
                               0, NULL);
  if (NULL == lattice->openCLparams)
  {
    lattice->openCLparams = (LB_OpenCL_p)malloc(sizeof(LB_OpenCL_t));

    lattice->openCLparams->u =  clCreateBuffer(context,
                                               CL_MEM_WRITE_ONLY,
                                               nodes_cnt * sizeof(LB3D_t),
                                               NULL, NULL);
    lattice->openCLparams->fs = clCreateBuffer(context,
                                               CL_MEM_WRITE_ONLY,
                                               2 * sizeof(double) * nodes_cnt * lattice->node_type,
                                               NULL, NULL);
  }
  
  /* 6. Launch the kernel. Let OpenCL pick the local work size */
  global_work_size = NWITEMS;
  clSetKernelArg(kernel, 0, sizeof (buffer), (void*) &buffer);
  clEnqueueNDRangeKernel(queue,
                         kernel,
                         1,
                         NULL,
                         &global_work_size,
                         NULL, 0, NULL, NULL);
  clFinish(queue);
  
  /* 7. Look at the results via synchronous buffer map */
  /*ptr = (cl_uint *) clEnqueueMapBuffer(queue,
                                       buffer,
                                       CL_TRUE,
                                       CL_MAP_READ,
                                       0,
                                       NWITEMS * sizeof (cl_uint),
                                       0, NULL, NULL, NULL);*/
  
  return 0;
}

int solver_initOpencl(void)
{
  /* 1. Get a platform */
  clGetPlatformIDs(1, &platform, NULL);

  /* 2. Find a gpu device */
  clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU,
                 1,
                 &device,
                 NULL);

  /* 3. Create a context on that device */
  context = clCreateContext(NULL,
                            1,
                            &device,
                            NULL, NULL, NULL);
  
  /* 4. Perform runtime source compilation, and obtain kernel entry point */
  program = clCreateProgramWithSource(context,
                                      1,
                                      &source,
                                      NULL, NULL);
  clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  kernel = clCreateKernel(program, "memset", NULL);

  return 0;
}
