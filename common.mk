VERBOSE_COMPILE     := y

OPENCL_INCLUDE_PATH :=
OPENCL_LIBS_PATH    :=

UNAME :=  $(shell uname -o)

ifeq ($(X32_64), 32)
  SUFFIX64=
else ifeq ($(X32_64), 64)
  SUFFIX64=_64
else
  $(error "Unknown arch: $(X32_64)")
endif

ifeq ($(VERBOSE_COMPILE), y)
  ECHO =
else
  ECHO = @
endif

CC         = gcc
CXX        = g++
CFLAGS     = -Wall
CXXFLAGS   = -O2
CMAKEFLAGS = -I$(ROOT)/$(INCLUDE_PATH) -I$(OPENCL_PATH)/include -Wall -Wextra -Werror -pedantic-errors
CLIBSPATHS = -L/lib -L$(OPENCL_PATH)/lib/x86$(SUFFIX64)

ifeq ($(UNAME), GNU/Linux)
  CMAKELIBS = -lGL -lGLU -lglut -lOpenCL
else ifeq ($(UNAME), Cygwin)
  CMAKEFLAGS += -D_WIN32
  CMAKELIBS = -lopengl32 -lglu32 -lglut32 -lOpenCL
else
  $(error "Unknown OS: $(UNAME)")
endif

INCLUDE_PATH = include
OBJECTS_PATH = .obj
