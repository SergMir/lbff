BUILDDIR = build

CFLAGS   = -Iinclude -Wall -Wextra -pedantic --std=gnu99 -O2 -g
LDFLAGS  = -Wl,--no-as-needed -lm -lGL -lglut -lGLU -fopenmp -L$(BUILDDIR) -llbff

SOURCES  = $(shell ls src/*.c)

CL_SRC  += $(shell ls src/*.cl)

OBJECTS  = $(patsubst %.c, $(BUILDDIR)/%.o, $(notdir $(SOURCES)))
OBJECTS += $(patsubst %.cl, $(BUILDDIR)/%.o, $(notdir $(CL_SRC)))


UNAME :=  $(shell uname -o)

ifeq ($(UNAME), GNU/Linux)
  LDFLAGS += -lGL -lGLU -lglut -lOpenCL
else ifeq ($(UNAME), Cygwin)
  CFLAGS  += -D_WIN32
  LDFLAGS += -lopengl32 -lglu32 -lglut32 -lOpenCL
else
  $(error "Unknown OS: $(UNAME)")
endif

$(BUILDDIR)/lbff: all
	$(CC) main.c $(CFLAGS) $(LDFLAGS) -o $@

all: $(BUILDDIR) $(BUILDDIR)/liblbff.a

$(BUILDDIR)/liblbff.a: $(OBJECTS)
	$(AR) rcs $(BUILDDIR)/liblbff.a $(OBJECTS)

$(BUILDDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILDDIR)/%.o: src/%.cl
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 $^ $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)
