* When just got sources, copy user.mk_example to user.mk, insert your parameters
(path to OpenCL libs and headers and so on).

* Get ATI Stream SDK:
http://developer.amd.com/gpu/AMDAPPSDK/downloads/Pages/default.aspx

Build on Windows:
* Tested on cygwin, opengl, glu, glut components required, read about at
http://www.cs.utoronto.ca/~wongam/d18_cygwin_opengl_setup/cygwin_opengl_setup.html

Build on GNU/Linux:
* Tested on freeglut, make and install it: http://freeglut.sourceforge.net/


* make [-jN]
On multicore systems it's possible to run parallel compilation with make's
parameter -j
N is number of threads, most popular formula: N = number_of_CPU_cores + 2
* ./lbff (lbff.exe)
* make clean

KNOWN ISSUES:
* user.mk: X32_64 - 64-bit support doesn't work yet, described here:
http://forums.amd.com/forum/messageview.cfm?catid=390&threadid=138890

LBFF Authors:
* Mary Fesenko (ChudikSmile@gmail.com) - Simulation, Solver
* Sergii Miroshnychenko (dartzerg@gmail.com) - Architecture, OpenCL, OpenGL

2011