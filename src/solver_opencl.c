#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <solver.h>
#include "solver_internal.h"

#define SOLVER_OPENCL_DEBUG_BUILD
#undef SOLVER_OPENCL_DEBUG_BUILD

typedef struct {
	int vectors_cnt;
	int countX;
	int countY;
	int countZ;
	int obj_num;
} solver_clParams_t, *solver_clParams_p;

cl_platform_id platform;
cl_device_id device;
cl_context context;

cl_program program_lb_bhk;
cl_kernel kernel_lb_bhk;

size_t global_work_size;

extern char _binary_src_kernel_cl_start;
extern char _binary_src_kernel_cl_end;
extern char _binary_src_kernel_cl_size;

/*
 * Run OpenCL kernel function
 */
int solver_ResolveOpencl(LB_Lattice_p lattice, force_pack_p forces,
			 int forces_num)
{
	int nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
	int fs_size = nodes_cnt * lattice->node_type;
	cl_command_queue queue;
	solver_vector_p vector = solver_GetVectors(lattice->node_type);
	cl_int status;
	long time_start, time_beforeCalc, time_afterCalc, time_stop;
	solver_clParams_t cl_params = {
		lattice->node_type,
		(int)lattice->countX,
		(int)lattice->countY,
		(int)lattice->countZ,
		forces_num
	};

	time_start = BASE_GetTimeNs();
	queue = clCreateCommandQueue(context, device, 0, &status);
	COND_OUT(CL_SUCCESS == status);

	if (NULL == lattice->openCLparams) {
		lattice->openCLparams =
			(LB_OpenCL_p) malloc(sizeof(LB_OpenCL_t));

		lattice->openCLparams->u =
			clCreateBuffer(context, CL_MEM_READ_WRITE,
				       nodes_cnt * sizeof(LB3D_t),
				       NULL, &status);
		COND_OUT(CL_SUCCESS == status);

		lattice->openCLparams->fs =
			clCreateBuffer(context, CL_MEM_READ_WRITE,
				       sizeof(lb_float) * fs_size,
				       NULL, &status);
		COND_OUT(CL_SUCCESS == status);

		lattice->openCLparams->fsnew =
			clCreateBuffer(context, CL_MEM_READ_WRITE,
				       sizeof(lb_float) * fs_size,
				       NULL, &status);
		COND_OUT(CL_SUCCESS == status);

		lattice->openCLparams->vectors =
			clCreateBuffer(context,
				       CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				       sizeof(solver_vector_t) * lattice->node_type,
				       vector, &status);
		COND_OUT(CL_SUCCESS == status);

		lattice->openCLparams->forces =
			clCreateBuffer(context, CL_MEM_READ_ONLY,
				       sizeof(force_pack_t) * forces_num,
				       NULL, &status);
		COND_OUT(CL_SUCCESS == status);

		lattice->openCLparams->params =
			clCreateBuffer(context, CL_MEM_READ_ONLY,
				       sizeof(cl_params),
				       NULL, &status);
		COND_OUT(CL_SUCCESS == status);
	}

	status = clEnqueueWriteBuffer(queue, (cl_mem) lattice->openCLparams->fs,
				      CL_FALSE, 0, sizeof(lb_float) * fs_size,
				      lattice->fs, 0, NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	status = clEnqueueWriteBuffer(queue, (cl_mem) lattice->openCLparams->fsnew,
				      CL_FALSE, 0, sizeof(lb_float) * fs_size,
				      lattice->fs + fs_size, 0, NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	status = clEnqueueWriteBuffer(queue, (cl_mem) lattice->openCLparams->forces,
				      CL_FALSE, 0, sizeof(force_pack_t) * forces_num,
				      forces, 0, NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	status = clEnqueueWriteBuffer(queue, (cl_mem) lattice->openCLparams->params,
				      CL_FALSE, 0, sizeof(cl_params), &cl_params, 0,
				      NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	global_work_size = nodes_cnt;
	clSetKernelArg(kernel_lb_bhk, 0, sizeof(cl_mem),
		       (void *)&(lattice->openCLparams->u));
	clSetKernelArg(kernel_lb_bhk, 1, sizeof(cl_mem),
		       (void *)&(lattice->openCLparams->fs));
	clSetKernelArg(kernel_lb_bhk, 2, sizeof(cl_mem),
		       (void *)&(lattice->openCLparams->fsnew));
	clSetKernelArg(kernel_lb_bhk, 3, sizeof(cl_mem),
		       (void *)&(lattice->openCLparams->vectors));
	clSetKernelArg(kernel_lb_bhk, 4, sizeof(cl_mem),
		       (void *)&(lattice->openCLparams->forces));
	clSetKernelArg(kernel_lb_bhk, 5, sizeof(cl_mem),
		       (void *)&(lattice->openCLparams->params));

	time_beforeCalc = BASE_GetTimeNs();
	status = clEnqueueNDRangeKernel(queue, kernel_lb_bhk,
					1, NULL, &global_work_size,
					NULL, 0, NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	clFinish(queue);
	time_afterCalc = BASE_GetTimeNs();

	status = clEnqueueReadBuffer(queue, (cl_mem) lattice->openCLparams->fsnew,
				     CL_TRUE, 0, sizeof(lb_float) * fs_size,
				     lattice->fs + fs_size, 0, NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	status = clEnqueueReadBuffer(queue, (cl_mem) lattice->openCLparams->u,
				     CL_TRUE, 0, nodes_cnt * sizeof(LB3D_t),
				     lattice->velocities, 0, NULL, NULL);
	COND_OUT(CL_SUCCESS == status);

	clReleaseCommandQueue(queue);
	time_stop = BASE_GetTimeNs();
	printf("Solver: pre_calc %8.3f ms; calc %8.3f ms; post_calc %8.3f ms\n",
	       BASE_GetTimeMs(time_start, time_beforeCalc),
	       BASE_GetTimeMs(time_beforeCalc, time_afterCalc),
	       BASE_GetTimeMs(time_afterCalc, time_stop));

out:
	return (CL_SUCCESS == status) ? 0 : -1;
}


int solver_initOpencl(void)
{
	cl_int status;
	const char * build_options = NULL;
	const char *kernel_src = &_binary_src_kernel_cl_start;

#if defined(SOLVER_OPENCL_DEBUG_BUILD)
	build_options = "-g -O0";
#endif

	status = clGetPlatformIDs(1, &platform, NULL);
	COND_OUT(CL_SUCCESS == status);

	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL,
				1, &device, NULL);
	COND_OUT(CL_SUCCESS == status);

	context = clCreateContext(NULL, 1, &device, NULL, NULL,
				  &status);
	COND_OUT(CL_SUCCESS == status);

	program_lb_bhk =
		clCreateProgramWithSource(context, 1, &kernel_src,
					  NULL, &status);
	COND_OUT(CL_SUCCESS == status);

	status = clBuildProgram(program_lb_bhk, 1, &device,
				build_options, NULL, NULL);
	if (CL_SUCCESS != status) {
		size_t build_log_size = 16 * 1024;
		char *build_log = malloc(build_log_size);

		clGetProgramBuildInfo(program_lb_bhk, device,
				      CL_PROGRAM_BUILD_LOG,
				      build_log_size, build_log,
				      NULL);
		fprintf(stderr, "Kernel program build failed:\n%s",
			build_log);
	}
	COND_OUT(CL_SUCCESS == status);

	kernel_lb_bhk = clCreateKernel(program_lb_bhk, "lb_bhk",
				       &status);
	COND_OUT(CL_SUCCESS == status);

out:
	return (CL_SUCCESS == status) ? 0 : -1;
}
