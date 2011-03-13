ROOT = ../..

include $(ROOT)/user.mk
include $(ROOT)/common.mk

_OBJ = $(shell ls *c)
OBJ  = $(patsubst %.c,%.o,$(_OBJ))

all: ${OBJ}

%.o: %.c
	$(ECHO) $(CC) $(CFLAGS) -c -o $(ROOT)/$(OBJECTS_PATH)/$@ $< $(CMAKEFLAGS)

