#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <solver.h>
#include "solver_internal.h"

#define solver_breakIfFailed \
    if (CL_SUCCESS != status) \
    { \
      break; \
    }

const char *sourceLB_BHK =
  "void getpos(int *sizes, int node, int *pos)                               \n"
  "{                                                                         \n"
  "}                                                                         \n"
  "                                                                          \n"
  "int nei(int *sizes, int node, float *vector)                              \n"
  "{                                                                         \n"
  "  int dx = fabs(vector[0]) > 0.577 ? 1 : 0;                               \n"
  "  int dy = fabs(vector[1]) > 0.577 ? 1 : 0;                               \n"
  "  int dz = fabs(vector[2]) > 0.577 ? 1 : 0;                               \n"
  "  return 0;                                                               \n"
  "}                                                                         \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "                                                                          \n"
  "float vec_mul(float *v1, float *v2)                                       \n"
  "{                                                                         \n"
  "  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];                         \n"
  "}                                                                         \n"
  "                                                                          \n"
  "float feqBHK(float density, float *u, float *vector, int vec_cnt)         \n"
  "{                                                                         \n"
  "  float t = vec_mul(vector, u);                                           \n"
  "  float fnew = 1 + 3 * t + 4.5 * t * t - 1.5 * vec_mul(u, u);             \n"
  "  fnew *= vector[3] * density;                                            \n"
  "  return fnew;                                                            \n"
  "}                                                                         \n"
  "                                                                          \n"
  "                                                                          \n"
  "__kernel void lb_bhk(__global float *us,                                  \n"
  "                     const __global float *fs,                            \n"
  "                     __global float *fsn,                                 \n"
  "                     const __global float *vectors,                       \n"
  "                     const  int nodes_cnt,                                \n"
  "                     const  int vectors_cnt)                              \n"
  "{                                                                         \n"
  "  int i = get_global_id(0);                                               \n"
  "  {                                                                       \n"
  "    int j;                                                                \n"
  "    for (j = 0; j < vectors_cnt; ++j)                                     \n"
  "    {                                                                     \n"
  "      fsn[i * vectors_cnt + j] = vectors[j*4 + 3];                        \n"
  "    }                                                                     \n"
  "  }                                                                       \n"
  "} \n";
  //"                                                                          \n";

cl_platform_id platform;
cl_device_id device;
cl_context context;

cl_program program_lb_bhk;
cl_kernel kernel_lb_bhk;

size_t global_work_size;

int solver_ResolveOpencl(LB_Lattice_p lattice)
{
  int nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  int fs_size = nodes_cnt * lattice->node_type;
  cl_command_queue queue;
  solver_vector_p vector = solver_GetVectors(lattice->node_type);
  cl_int status = CL_SUCCESS;
  
  queue = clCreateCommandQueue(context,
                               device,
                               0, &status);
  if (CL_SUCCESS != status) { exit(-1); }
  
  if (NULL == lattice->openCLparams)
  {
    memset(lattice->fs + fs_size, 0, sizeof (lb_float) * fs_size);
    do
    {
      lattice->openCLparams = (LB_OpenCL_p) malloc(sizeof (LB_OpenCL_t));

      lattice->openCLparams->u = clCreateBuffer(context,
                                                CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                                nodes_cnt * sizeof (LB3D_t),
                                                lattice->velocities,
                                                &status);
      solver_breakIfFailed;
      
      lattice->openCLparams->fs = clCreateBuffer(context,
                                                 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                                 sizeof (lb_float) * fs_size,
                                                 lattice->fs,
                                                 &status);
      solver_breakIfFailed;

      lattice->openCLparams->fsnew = clCreateBuffer(context,
                                                    CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                                    sizeof (lb_float) * fs_size,
                                                    lattice->fs + fs_size,
                                                    &status);
      solver_breakIfFailed;
      
      lattice->openCLparams->vectors = clCreateBuffer(context,
                                                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                                      sizeof (lb_float) * lattice->node_type,
                                                      vector,
                                                      &status);
      solver_breakIfFailed;
      
      
    } while (0);
  }
  
  
  if (CL_SUCCESS == status)
  {
    global_work_size = nodes_cnt;
    clSetKernelArg(kernel_lb_bhk, 0, sizeof(cl_mem), (void*) &(lattice->openCLparams->u));
    clSetKernelArg(kernel_lb_bhk, 1, sizeof(cl_mem), (void*) &(lattice->openCLparams->fs));
    clSetKernelArg(kernel_lb_bhk, 2, sizeof(cl_mem), (void*) &(lattice->openCLparams->fsnew));
    clSetKernelArg(kernel_lb_bhk, 3, sizeof(cl_mem), (void*) &(lattice->openCLparams->vectors));
    clSetKernelArg(kernel_lb_bhk, 4, sizeof(int),    (void*) &nodes_cnt);
    clSetKernelArg(kernel_lb_bhk, 5, sizeof(int),    (void*) &(lattice->node_type));
    status = clEnqueueNDRangeKernel(queue,
                           kernel_lb_bhk,
                           1,
                           NULL,
                           &global_work_size,
                           NULL, 0, NULL,
                           NULL);
    clFinish(queue);

    status = clEnqueueReadBuffer(queue,
                                (cl_mem)lattice->openCLparams->fsnew,
                                 CL_TRUE,
                                 0,
                                 sizeof (lb_float) * fs_size,
                                 lattice->fs + fs_size,
                                 0, NULL, NULL);
  }
  
  if (CL_SUCCESS != status)
  {
    exit(-1);
  }
  return (CL_SUCCESS == status) ? 0 : -1;
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

    program_lb_bhk = clCreateProgramWithSource(context,
                                               1,
                                               &sourceLB_BHK,
                                               NULL,
                                               &status);
    solver_breakIfFailed;

    status = clBuildProgram(program_lb_bhk, 1, &device, NULL, NULL, NULL);
    if (CL_SUCCESS != status)
    {
      char *build_log;
      size_t ret_val_size;
      clGetProgramBuildInfo(program_lb_bhk, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
      build_log = new char[ret_val_size + 1];
      clGetProgramBuildInfo(program_lb_bhk, device, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
      printf("%s", build_log);
    }
    solver_breakIfFailed;

    kernel_lb_bhk = clCreateKernel(program_lb_bhk, "lb_bhk",
                                   &status);
    solver_breakIfFailed;
  }
  while (0);
  
  return (CL_SUCCESS == status) ? 0 : -1;
}
