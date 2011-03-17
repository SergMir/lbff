* When just got sources, copy user.mk_example to user.mk, insert your parameters
(path to OpenCL libs and headers and so on).

* Get ATI Stream SDK:
http://developer.amd.com/gpu/AMDAPPSDK/downloads/Pages/default.aspx

Build on Windows:
* Tested on cygwin, opengl, glu, glut components required, read about at
http://www.cs.utoronto.ca/~wongam/d18_cygwin_opengl_setup/cygwin_opengl_setup.html

Build on GNU/Linux:
* Tested on freeglut, make and install it: http://freeglut.sourceforge.net/


* make
* ./lbff (lbff.exe)
* make clean

KNOWN ISSUES:
* user.mk: X32_64 - 64-bit support doesn't work yet, described here:
http://forums.amd.com/forum/messageview.cfm?catid=390&threadid=138890