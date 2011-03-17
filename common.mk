VERBOSE_COMPILE     := y

OPENCL_INCLUDE_PATH :=
OPENCL_LIBS_PATH    :=

UNAME := $(shell uname -o)

ifeq ($(VERBOSE_COMPILE), y)
  ECHO =
else
  ECHO = @
endif

CC         = gcc
CXX        = g++
CFLAGS     = -Wall
CXXFLAGS   = -O2
CMAKEFLAGS = -I$(ROOT)/$(INCLUDE_PATH) -L/lib -Wall -I$(OPENCL_PATH)/include

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
