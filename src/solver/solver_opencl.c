#include <CL/cl.h>
#include <stdio.h>
#define NWITEMS 512
/* A simple memset kernel */
const char *source =
  "__kernel void memset( __global uint *dst )                              \n"
  "{                                                                       \n"
  " dst[get_global_id(0)] = get_global_id(0);                              \n"
  "} \n";

int opencl()
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;
  cl_mem buffer;
  size_t global_work_size;
  cl_uint *ptr;
  int i;

  /* 1. Get a platform */
  clGetPlatformIDs(1, &platform, NULL);

  /* 2. Find a gpu device */
  clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU,
                 1,
                 &device,
                 NULL);

  /* 3. Create a context and command queue on that device */
  context = clCreateContext(NULL,
                            1,
                            &device,
                            NULL, NULL, NULL);
  queue = clCreateCommandQueue(context,
                               device,
                               0, NULL);

  /* 4. Perform runtime source compilation, and obtain kernel entry point */
  program = clCreateProgramWithSource(context,
                                      1,
                                      &source,
                                      NULL, NULL);
  clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  kernel = clCreateKernel(program, "memset", NULL);

  /* 5. Create a data buffer */
  buffer = clCreateBuffer(context,
                          CL_MEM_WRITE_ONLY,
                          NWITEMS * sizeof (cl_uint),
                          NULL, NULL);

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
  ptr = (cl_uint *) clEnqueueMapBuffer(queue,
                                       buffer,
                                       CL_TRUE,
                                       CL_MAP_READ,
                                       0,
                                       NWITEMS * sizeof (cl_uint),
                                       0, NULL, NULL, NULL);

  for (i = 0; i < NWITEMS; i++)
  {
    printf("%d %d\n", i, ptr[i]);
  }
  
  return 0;
}
