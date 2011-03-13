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
CMAKEFLAGS = -I$(ROOT)/$(INCLUDE_PATH) -L$(ROOT)/$(INCLUDE_PATH) -L/lib -Wall

ifeq ($(UNAME), Linux)
  CMAKELIBS = -lGL -lGLU -lglut
else ifeq ($(UNAME), Cygwin)
  CMAKELIBS = -lopengl32 -lglu32 -lglut32
else
  $(error "Unknown OS: $(UNAME)")
endif

INCLUDE_PATH = include
OBJECTS_PATH = .obj
