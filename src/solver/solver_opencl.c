#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <solver.h>
#include "solver_internal.h"

/* ------------------------------- Defines --------------------------------- */

#define solver_breakIfFailed(msg, code) \
    if (CL_SUCCESS != status) \
    { \
      printf("[ERROR][%s : %s : %d] %s | error code : (%d)\n", __FILE__, __FUNCTION__, __LINE__, (msg), (code)); \
      break; \
    }

#define SOLVER_OPENCL_DEBUG_BUILD
#undef SOLVER_OPENCL_DEBUG_BUILD

/* -------------------------------- Types ---------------------------------- */
typedef struct
{
  int vectors_cnt;
  int countX;
  int countY;
  int countZ;
  int obj_num;
} solver_clParams_t, *solver_clParams_p;

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

const char *sourceLB_BHK =
  "#define F_COMP(X, Y) (fabs((X) - (Y)) < 0.00001f)                         \n"
  "                                                                          \n"
  "typedef struct                                                            \n"
  "{                                                                         \n"
  "  float  points[3];                                                       \n"
  "  float  vector[3];                                                       \n"
  "  float  force;                                                           \n"
  "} EXTOBJ_force_t;                                                         \n"
  "                                                                          \n"
  "typedef struct                                                            \n"
  "{                                                                         \n"
  "  int vectors_cnt;                                                        \n"
  "  int countX;                                                             \n"
  "  int countY;                                                             \n"
  "  int countZ;                                                             \n"
  "  int obj_num;                                                            \n"
  "} cl_params_t;                                                            \n"
  "                                                                          \n"
  "typedef struct                                                            \n"
  "{                                                                         \n"
  "  int forces_num;                                                         \n"
  "  EXTOBJ_force_t forces[100];                                             \n"
  "} force_pack_t, *force_pack_p;                                            \n"
  "                                                                          \n"
  "void getpos(int *counts, int node, int *pos, float *coord)                \n"
  "{                                                                         \n"
  "  int xy = counts[0] * counts[1];                                         \n"
  "  float step = 1;                                                         \n"
  "                                                                          \n"
  "  pos[2] = node / xy;                                                     \n"
  "  node -= pos[2] * xy;                                                    \n"
  "  pos[1] = node / counts[0];                                              \n"
  "  pos[0] = node - pos[1] * counts[0];                                     \n"
  "  if (0 != coord)                                                         \n"
  "  {                                                                       \n"
  "    coord[0] = pos[0] * step;                                             \n"
  "    coord[1] = pos[1] * step;                                             \n"
  "    coord[2] = pos[2] * step;                                             \n"
  "  }                                                                       \n"
  "}                                                                         \n"
  "                                                                          \n"
  "int nei(int *counts, int node, float *vector)                             \n"
  "{                                                                         \n"
  "  float min1 = 0.577f;                                                    \n"
  "  int dx = fabs(vector[0]) > min1  ? 1 : 0;                               \n"
  "  int dy = fabs(vector[1]) > min1  ? 1 : 0;                               \n"
  "  int dz = fabs(vector[2]) > min1  ? 1 : 0;                               \n"
  "  int pos[3];                                                             \n"
  "                                                                          \n"
  "  getpos(counts, node, pos, 0);                                           \n"
  "                                                                          \n"
  "  dx *= vector[0] > 0 ? 1 : -1;                                           \n"
  "  dy *= vector[1] > 0 ? 1 : -1;                                           \n"
  "  dz *= vector[2] > 0 ? 1 : -1;                                           \n"
  "                                                                          \n"
  "  do                                                                      \n"
  "  {                                                                       \n"
  "    node = -1;                                                            \n"
  "    if (dx < 0 && pos[0] == 0)                                            \n"
  "      break;                                                              \n"
  "    if (dx > 0 && pos[0] == (counts[0] - 1))                              \n"
  "      break;                                                              \n"
  "    if (dy < 0 && pos[1] == 0)                                            \n"
  "      break;                                                              \n"
  "    if (dy > 0 && pos[1] == (counts[1] - 1))                              \n"
  "      break;                                                              \n"
  "    if (dz < 0 && pos[2] == 0)                                            \n"
  "      break;                                                              \n"
  "    if (dz > 0 && pos[2] == (counts[2] - 1))                              \n"
  "      break;                                                              \n"
  "                                                                          \n"
  "    pos[0] += dx;                                                         \n"
  "    pos[1] += dy;                                                         \n"
  "    pos[2] += dz;                                                         \n"
  "                                                                          \n"
  "    node = pos[2] * counts[0] * counts[1] + pos[1] * counts[1] + pos[0];  \n"
  "  } while (0);                                                            \n"
  "                                                                          \n"
  "  return node;                                                            \n"
  "}                                                                         \n"
  "                                                                          \n"
  "float vec_mul(const float *v1, float *v2)                                 \n"
  "{                                                                         \n"
  "  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];                         \n"
  "}                                                                         \n"
  "                                                                          \n"
  "float feqBHK(float density, float *u, float *vector)                      \n"
  "{                                                                         \n"
  "  float t = vec_mul(vector, u);                                           \n"
  "  float fnew = 1.0f + 3.0f * t + 4.5f * t * t - 1.5f * vec_mul(u, u);     \n"
  "  fnew *= vector[3] * density;                                            \n"
  "  return fnew;                                                            \n"
  "}                                                                         \n"
  "                                                                          \n"
  "                                                                          \n"
  "/*----------------------------------------------------------------------*/\n"
  "                                                                          \n"
  "                                                                          \n"
  "__kernel void lb_bhk(__global float *us,                                  \n"
  "                     __global float *fs,                                  \n"
  "                     __global float *fsn,                                 \n"
  "                     const __global float *vectors,                       \n"
  "                     const __global force_pack_t *forces,                 \n"
  "                     const __global cl_params_t *params)                  \n"
  "{                                                                         \n"
  "  float tau = 0.55f;                                                      \n"
  "  int vectors_cnt = params->vectors_cnt;                                  \n"
  "  int counts[3] = { params->countX, params->countY, params->countZ };     \n"
  "  int i = get_global_id(0);                                               \n"
  "  {                                                                       \n"
  "    float density = 0, fe[3] = {0, 0, 0};                                 \n"
  "    int k, obj;                                                           \n"
  "    int pos[3];                                                           \n"
  "    float coord[3];                                                       \n"
  "                                                                          \n"
  "    getpos(counts, i, pos, coord);                                        \n"
  "                                                                          \n"
  "    for (k = 0; k < vectors_cnt; ++k)                                     \n"
  "    {                                                                     \n"
  "      float f = fs[i * vectors_cnt + k];                                  \n"
  "      density += f;                                                       \n"
  "      fe[0] += f * vectors[k*4 + 0];                                      \n"
  "      fe[1] += f * vectors[k*4 + 1];                                      \n"
  "      fe[2] += f * vectors[k*4 + 2];                                      \n"
  "    }                                                                     \n"
  "    us[i * 3 + 0] = fe[0] / density;                                      \n"
  "    us[i * 3 + 1] = fe[1] / density;                                      \n"
  "    us[i * 3 + 2] = fe[2] / density;                                      \n"
  "                                                                          \n"
  "    {                                                                     \n"
  "      float ux = us[i * 3 + 0];                                           \n"
  "      float uy = us[i * 3 + 1];                                           \n"
  "      float uz = us[i * 3 + 2];                                           \n"
  "      if (0.577f < sqrt(ux*ux + uy*uy + uz*uz))                           \n"
  "      {                                                                   \n"
  "        for (k = 0; k < vectors_cnt; ++k)                                 \n"
  "        {                                                                 \n"
  "          us[i * 3 + 0] = 0;                                              \n"
  "          us[i * 3 + 1] = 0;                                              \n"
  "          us[i * 3 + 2] = 0;                                              \n"
  "          density = 1.0f;                                                 \n"
  "          fs[i * vectors_cnt + k] = vectors[k*4 + 3];                     \n"
  "        }                                                                 \n"
  "      }                                                                   \n"
  "    }                                                                     \n"
  "                                                                          \n"
  "    for (k = 0; k < vectors_cnt; ++k)                                     \n"
  "    {                                                                     \n"
  "      float nvec[4] = {                                                   \n"
  "        vectors[k * 4 + 0],                                               \n"
  "        vectors[k * 4 + 1],                                               \n"
  "        vectors[k * 4 + 2],                                               \n"
  "        vectors[k * 4 + 3],                                               \n"
  "      };                                                                  \n"
  "      float nu[3] = {                                                     \n"
  "        us[i * 3 + 0],                                                    \n"
  "        us[i * 3 + 1],                                                    \n"
  "        us[i * 3 + 2],                                                    \n"
  "      };                                                                  \n"
  "      int next_node = nei(counts, i, nvec);                               \n"
  "                                                                          \n"
  "      if (-1 != next_node)                                                \n"
  "      {                                                                   \n"
  "        float fsi = fs[i * vectors_cnt + k];                              \n"
  "        float feq = feqBHK(density, nu, nvec);                            \n"
  "        float delta = (fsi - feq) / tau;                                  \n"
  "        fsn[next_node * vectors_cnt + k] += fsi - delta;                  \n"
  "      }                                                                   \n"
  "      else                                                                \n"
  "      {                                                                   \n"
  "        int opp_k;                                                        \n"
  "        for (opp_k = 0; opp_k < vectors_cnt; ++opp_k)                     \n"
  "        {                                                                 \n"
  "          if (                                                            \n"
  "            F_COMP(nvec[0], -vectors[opp_k * 4 + 0]) &&                   \n"
  "            F_COMP(nvec[1], -vectors[opp_k * 4 + 1]) &&                   \n"
  "            F_COMP(nvec[2], -vectors[opp_k * 4 + 2]))                     \n"
  "          {                                                               \n"
  "            break;                                                        \n"
  "          }                                                               \n"
  "        }                                                                 \n"
  "                                                                          \n"
  "        if (opp_k < vectors_cnt)                                          \n"
  "        {                                                                 \n"
  "          fsn[i * vectors_cnt + opp_k] += fs[i * vectors_cnt + k];        \n"
  "        }                                                                 \n"
  "      }                                                                   \n"
  "    }                                                                     \n"
  "                                                                          \n"
  "    for (obj = 0; obj < params->obj_num; ++obj)                           \n"
  "    {                                                                     \n"
  "      int j;                                                              \n"
  "                                                                          \n"
  "      for (j = 0; j < forces[obj].forces_num; ++j)                        \n"
  "      {                                                                   \n"
  "        float dx = forces[obj].forces[j].points[0] - pos[0];              \n"
  "        float dy = forces[obj].forces[j].points[1] - pos[1];              \n"
  "        float dz = forces[obj].forces[j].points[2] - pos[2];              \n"
  "        float dist = sqrt(dx*dx + dy*dy + dz*dz);                         \n"
  "                                                                          \n"
  "        float d = 3.0f * 90.0f / params->countX;                          \n"
  "                                                                          \n"
  "        if (dist < d)                                                     \n"
  "        {                                                                 \n"
  "          for (k = 0; k < vectors_cnt; ++k)                               \n"
  "          {                                                               \n"
  "            float nvec[3] = {                                             \n"
  "              vectors[k * 4 + 0],                                         \n"
  "              vectors[k * 4 + 1],                                         \n"
  "              vectors[k * 4 + 2] };                                       \n"
  "            float nforce[3] = {                                           \n"
  "              forces[obj].forces[j].vector[0],                            \n"
  "              forces[obj].forces[j].vector[1],                            \n"
  "              forces[obj].forces[j].vector[2] };                          \n"
  "            float delta = (d - dist) / d;                                 \n"
  "            delta *= vec_mul(nvec,                                        \n"
  "                             nforce);                                     \n"
  "            if (fsn[i * vectors_cnt + k] + delta < 0)                     \n"
  "            {                                                             \n"
  "              delta = 0;                                                  \n"
  "            }                                                             \n"
  "            fsn[i * vectors_cnt + k] += delta;                            \n"
  "          }                                                               \n"
  "        }                                                                 \n"
  "      }                                                                   \n"
  "    }                                                                     \n"
  "  }                                                                       \n"
  "                                                                          \n"
  "} \n";

cl_platform_id platform;
cl_device_id device;
cl_context context;

cl_program program_lb_bhk;
cl_kernel kernel_lb_bhk;

size_t global_work_size;

/* --------------------------- Implementation ------------------------------ */

/*
 * Run OpenCL kernel function
 */
int solver_ResolveOpencl(LB_Lattice_p lattice, force_pack_p forces, int forces_num)
{
  int nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  int fs_size = nodes_cnt * lattice->node_type;
  cl_command_queue queue;
  solver_vector_p vector = solver_GetVectors(lattice->node_type);
  cl_int status;
  long time_start, time_beforeCalc, time_afterCalc, time_stop;
  solver_clParams_t cl_params = {lattice->node_type,
      lattice->countX, lattice->countY, lattice->countZ, forces_num};

  do
  {
    time_start = BASE_GetTimeNs();
    queue = clCreateCommandQueue(context,
                                 device,
                                 0, &status);
    solver_breakIfFailed("Create queue", status);

    if (NULL == lattice->openCLparams)
    {
      lattice->openCLparams = (LB_OpenCL_p) malloc(sizeof (LB_OpenCL_t));

      lattice->openCLparams->u = clCreateBuffer(context,
                                                CL_MEM_READ_WRITE,
                                                nodes_cnt * sizeof (LB3D_t),
                                                NULL, &status);
      solver_breakIfFailed("Create velocities buffer", status);

      lattice->openCLparams->fs = clCreateBuffer(context,
                                                 CL_MEM_READ_WRITE,
                                                 sizeof (lb_float) * fs_size,
                                                 NULL, &status);
      solver_breakIfFailed("Create fs buffer", status);

      lattice->openCLparams->fsnew = clCreateBuffer(context,
                                                    CL_MEM_READ_WRITE,
                                                    sizeof (lb_float) * fs_size,
                                                    NULL, &status);
      solver_breakIfFailed("Create fs* buffer", status);

      lattice->openCLparams->vectors = clCreateBuffer(context,
                                                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                      sizeof (solver_vector_t) * lattice->node_type,
                                                      vector, &status);
      solver_breakIfFailed("Create vectors buffer", status);

      lattice->openCLparams->forces = clCreateBuffer(context,
                                                     CL_MEM_READ_ONLY,
                                                     sizeof (force_pack_t) * forces_num,
                                                     NULL, &status);
      solver_breakIfFailed("Create forces buffer", status);

      lattice->openCLparams->params = clCreateBuffer(context,
                                                     CL_MEM_READ_ONLY,
                                                     sizeof (cl_params),
                                                     NULL, &status);
      solver_breakIfFailed("Create forces buffer", status);
    }
    
    status = clEnqueueWriteBuffer(queue,
                                  (cl_mem) lattice->openCLparams->fs,
                                  CL_FALSE, 0,
                                  sizeof (lb_float) * fs_size,
                                  lattice->fs,
                                  0, NULL, NULL);
    solver_breakIfFailed("clEnqueueWriteBuffer(fs)", status);
    
    status = clEnqueueWriteBuffer(queue,
                                  (cl_mem) lattice->openCLparams->fsnew,
                                  CL_FALSE, 0,
                                  sizeof (lb_float) * fs_size,
                                  lattice->fs + fs_size,
                                  0, NULL, NULL);
    solver_breakIfFailed("clEnqueueWriteBuffer(fsnew)", status);
    
    status = clEnqueueWriteBuffer(queue,
                                  (cl_mem) lattice->openCLparams->forces,
                                  CL_FALSE, 0,
                                  sizeof (force_pack_t) * forces_num,
                                  forces,
                                  0, NULL, NULL);
    solver_breakIfFailed("clEnqueueWriteBuffer(forces)", status);
    
    status = clEnqueueWriteBuffer(queue,
                                  (cl_mem) lattice->openCLparams->params,
                                  CL_FALSE, 0,
                                  sizeof (cl_params),
                                  &cl_params,
                                  0, NULL, NULL);
    solver_breakIfFailed("clEnqueueWriteBuffer(params)", status);

    global_work_size = nodes_cnt;
    clSetKernelArg(kernel_lb_bhk, 0, sizeof (cl_mem), (void*) &(lattice->openCLparams->u));
    clSetKernelArg(kernel_lb_bhk, 1, sizeof (cl_mem), (void*) &(lattice->openCLparams->fs));
    clSetKernelArg(kernel_lb_bhk, 2, sizeof (cl_mem), (void*) &(lattice->openCLparams->fsnew));
    clSetKernelArg(kernel_lb_bhk, 3, sizeof (cl_mem), (void*) &(lattice->openCLparams->vectors));
    clSetKernelArg(kernel_lb_bhk, 4, sizeof (cl_mem), (void*) &(lattice->openCLparams->forces));
    clSetKernelArg(kernel_lb_bhk, 5, sizeof (cl_mem), (void*) &(lattice->openCLparams->params));

    time_beforeCalc = BASE_GetTimeNs();
    //__OpenCL_lb_bhk_kernel
    status = clEnqueueNDRangeKernel(queue, kernel_lb_bhk,
                                    1, NULL, &global_work_size,
                                    NULL, 0, NULL, NULL);
    solver_breakIfFailed("clEnqueueNDRangeKernel", status);

    clFinish(queue);
    time_afterCalc = BASE_GetTimeNs();

    status = clEnqueueReadBuffer(queue,
                                 (cl_mem) lattice->openCLparams->fsnew,
                                 CL_TRUE, 0,
                                 sizeof (lb_float) * fs_size,
                                 lattice->fs + fs_size,
                                 0, NULL, NULL);
    solver_breakIfFailed("clEnqueueReadBuffer(fsnew)", status);

    status = clEnqueueReadBuffer(queue,
                         (cl_mem) lattice->openCLparams->u,
                         CL_TRUE, 0,
                         nodes_cnt * sizeof (LB3D_t),
                         lattice->velocities,
                         0, NULL, NULL);
    solver_breakIfFailed("clEnqueueReadBuffer(u)", status);

    clReleaseCommandQueue(queue);
    time_stop = BASE_GetTimeNs();
    printf("Solver: pre_calc %8.3f ms; calc %8.3f ms; post_calc %8.3f ms\n",
           BASE_GetTimeMs(time_start, time_beforeCalc),
           BASE_GetTimeMs(time_beforeCalc, time_afterCalc),
           BASE_GetTimeMs(time_afterCalc, time_stop));
  } while (0);

  return (CL_SUCCESS == status) ? 0 : -1;
}

/*
 * Get supported devices and build OpenCL kernel
 */
int solver_initOpencl(void)
{
  cl_int status;

  do
  {
    /* 1. Get a platform */
    status = clGetPlatformIDs(1, &platform, NULL);
    solver_breakIfFailed("clGetPlatformIDs", status);

    /* 2. Find a gpu device */
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU,
                            1,
                            &device,
                            NULL);
    solver_breakIfFailed("clGetDeviceIDs", status);

    /* 3. Create a context on that device */
    context = clCreateContext(NULL,
                              1,
                              &device,
                              NULL, NULL,
                              &status);
    solver_breakIfFailed("clCreateContext", status);

    program_lb_bhk = clCreateProgramWithSource(context,
                                               1,
                                               &sourceLB_BHK,
                                               NULL,
                                               &status);
    solver_breakIfFailed("clCreateProgramWithSource", status);

    status = clBuildProgram(program_lb_bhk, 1, &device,
#if defined(SOLVER_OPENCL_DEBUG_BUILD)
                            "-g -O0",
#else
                            //"-Werror",
                            NULL,
#endif
                            NULL, NULL);
    if (CL_SUCCESS != status)
    {
      char *build_log;
      size_t ret_val_size;
      clGetProgramBuildInfo(program_lb_bhk, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
      build_log = new char[ret_val_size + 1];
      clGetProgramBuildInfo(program_lb_bhk, device, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
      printf("%s", build_log);
    }
    solver_breakIfFailed("clBuildProgram", status);

    kernel_lb_bhk = clCreateKernel(program_lb_bhk, "lb_bhk",
                                   &status);
    solver_breakIfFailed("clCreateKernel", status);
  }
  while (0);
  
  return (CL_SUCCESS == status) ? 0 : -1;
}
