#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <solver.h>
#include "solver_internal.h"

#define NWITEMS 512
/* A simple memset kernel */
const char *source =
  "__kernel void memset( __global uint *dst )                              \n"
  "{                                                                       \n"
  " dst[get_global_id(0)] = get_global_id(0);                              \n"
  "} \n";

const char *sourceLB_BHK =
  "__kernel void lb_bhk(const __global float *u,                             \n"
  "                     __global float *fs,                                  \n"
  "                     const __global float *vectors,                       \n"
  "                     const int nodes_cnt,                                 \n"
  "                     const int vectors_cnt)                               \n"
  "{                                                                         \n"
  "  int i;                                                                  \n"
  "  for (i = 0; i < nodes_cnt; ++i)                                         \n"
  "  {                                                                       \n"
  "  }                                                                       \n"
  "} \n";

cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_program program;
cl_kernel kernel;

cl_program program_lb_bhk;
cl_kernel kernel_lb_bhk;

size_t global_work_size;

int solver_ResolveOpencl(LB_Lattice_p lattice)
{
  int nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  int fs_size = nodes_cnt * lattice->node_type;
  cl_command_queue queue;
  solver_vector_p vector = solver_GetVectors(lattice->node_type);
  
  queue = clCreateCommandQueue(context,
                               device,
                               0, NULL);
  if (NULL == lattice->openCLparams)
  {
    lattice->openCLparams = (LB_OpenCL_p) malloc(sizeof (LB_OpenCL_t));

    lattice->openCLparams->u =     clCreateBuffer(context,
                                                  CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                                  nodes_cnt * sizeof(LB3D_t),
                                                  lattice->velocities,
                                                  NULL);
    lattice->openCLparams->fs =    clCreateBuffer(context,
                                                  CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                                  sizeof(lb_float) * fs_size,
                                                  lattice->fs,
                                                  NULL);
    lattice->openCLparams->fsnew = clCreateBuffer(context,
                                                  CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                                  sizeof(lb_float) * fs_size,
                                                  lattice->fs + fs_size,
                                                  NULL);
    lattice->openCLparams->vectors = clCreateBuffer(context,
                                                  CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                                  sizeof(lb_float) * lattice->node_type,
                                                  vector,
                                                  NULL);
  }
  
  /* 6. Launch the kernel. Let OpenCL pick the local work size */
  global_work_size = NWITEMS;
  clSetKernelArg(kernel, 0, nodes_cnt * sizeof(LB3D_t), (void*)lattice->openCLparams->u);
  clSetKernelArg(kernel, 1, nodes_cnt * sizeof(LB3D_t), (void*)lattice->openCLparams->fs);
  clSetKernelArg(kernel, 2, nodes_cnt * sizeof(LB3D_t), (void*)lattice->openCLparams->vectors);
  clSetKernelArg(kernel, 3, sizeof(nodes_cnt),          (void*)&nodes_cnt);
  clSetKernelArg(kernel, 4, sizeof(lattice->node_type), (void*)&(lattice->node_type));
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

#define solver_breakIfFailed \
    if (CL_SUCCESS != status) \
    { \
      break; \
    }

int solver_initOpencl(void)
{
  cl_int status;

  do
  {
    /* 1. Get a platform */
    status = clGetPlatformIDs(1, &platform, NULL);
    solver_breakIfFailed;

    /* 2. Find a gpu device */
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU,
                            1,
                            &device,
                            NULL);
    solver_breakIfFailed;

    /* 3. Create a context on that device */
    context = clCreateContext(NULL,
                              1,
                              &device,
                              NULL, NULL,
                              &status);
    solver_breakIfFailed;

    /* 4. Perform runtime source compilation, and obtain kernel entry point */
    program = clCreateProgramWithSource(context,
                                        1,
                                        &source,
                                        NULL,
                                        &status);
    solver_breakIfFailed;

    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    solver_breakIfFailed;

    kernel = clCreateKernel(program, "memset",
                            &status);
    solver_breakIfFailed;

    program_lb_bhk = clCreateProgramWithSource(context,
                                               1,
                                               &sourceLB_BHK,
                                               NULL,
                                               &status);
    solver_breakIfFailed;

    status = clBuildProgram(program_lb_bhk, 1, &device, NULL, NULL, NULL);
    solver_breakIfFailed;

    kernel_lb_bhk = clCreateKernel(program_lb_bhk, "lb_bhk",
                                   &status);
    solver_breakIfFailed;
  }
  while (0);

  return (CL_SUCCESS == status) ? 0 : -1;
}
