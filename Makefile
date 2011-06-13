include user.mk
include common.mk

ROOT = ./

SOURCES_PATH = src
SRC_SUBDIRS  = base solver graph extobj ui

all: .obj subdirs
	$(ECHO) $(CC) -o lbff $(patsubst %, $(OBJECTS_PATH)/%,$(shell ls $(OBJECTS_PATH))) $(CMAKEFLAGS) $(CLIBSPATHS) $(CMAKELIBS)

.obj:
	mkdir .obj

subdirs: $(SRC_SUBDIRS)

$(SRC_SUBDIRS):
	$(MAKE) -C $(ROOT)/$(SOURCES_PATH)/$@

clean:
	$(ECHO) rm -rf $(OBJECTS_PATH)
	$(ECHO) rm -f lbff
	$(ECHO) rm -f *.tmp
	$(ECHO) rm -f *.tmp.dll
	$(ECHO) rm -f *.tmp.cl

