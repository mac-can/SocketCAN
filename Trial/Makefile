#
#	Trial Program
#	PCBUSB-Wrapper
#	Bart Simpson didn´t do it
#
DRV	= ../Sources

PROGRAM	= soc_test

OBJECTS = $(OUTDIR)/main.o $(OUTDIR)/can_io.o

MAIN_DEPS = $(DRV)/can_io.h $(DRV)/can_defs.h

CAN_IO_DEPS = $(DRV)/can_io.h $(DRV)/can_defs.h

CFLAGS	= -O2 -Wall -Wno-parentheses \
	-fno-strict-aliasing \
	-D_CANAPI_EXTERN \
	-I$(DRV)

LIBRARIES = -lpthread

LDFLAGS =

CC = gcc
LD = gcc
RM = rm -f
CP = cp -f

OUTDIR = .objects


.PHONY: outdir


all: outdir $(PROGRAM)

outdir:
	@mkdir -p $(OUTDIR)

clean:
	$(RM) $(PROGRAM) $(OUTDIR)/*.o

install:
	$(CP) $(PROGRAM) ~/bin


$(OUTDIR)/main.o: main.c $(MAIN_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTDIR)/can_io.o: $(DRV)/can_io.c $(CAN_IO_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<


$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
	@echo "Target '"$@"' successfully build"


# #############################################################################
