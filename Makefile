include user.mk
include common.mk

ROOT = ./

SOURCES_PATH = src
SRC_SUBDIRS  = base solver graph extobj ui

all: .obj subdirs
	$(ECHO) $(CC) -o lbff $(patsubst %, $(OBJECTS_PATH)/%,$(shell ls $(OBJECTS_PATH))) $(CMAKEFLAGS) $(CMAKELIBS) -L$(OPENCL_PATH)/lib/x86 -lOpenCL

.obj:
	mkdir .obj

subdirs:
	$(ECHO) set -e; for d in $(SRC_SUBDIRS); do make -C $(SOURCES_PATH)/$$d ; done


clean:
	$(ECHO) rm -rf $(OBJECTS_PATH)
	$(ECHO) rm -f lbff

