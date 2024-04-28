#
#	Trial Program
#	SocketCAN-Wrapper
#	Bart Simpson didn´t do it
#
DRV	= ../Sources

PROGRAM	= soc_test

OBJECTS = $(OUTDIR)/main.o $(OUTDIR)/can_api.o $(OUTDIR)/printmsg.o

MAIN_DEPS = $(DRV)/can_api.h $(DRV)/can_defs.h $(DRV)/can_vers.h ./printmsg.h

CAN_API_DEPS = $(DRV)/can_api.h $(DRV)/can_defs.h $(DRV)/can_vers.h

PRINTMSG_DEPS = ./printmsg.h

CFLAGS	= -O2 -Wall -Wno-parentheses \
	-fno-strict-aliasing \
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

$(OUTDIR)/printmsg.o: printmsg.c $(PRINTMSG_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTDIR)/can_api.o: $(DRV)/can_api.c $(CAN_API_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<


$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
	@echo "Target '"$@"' successfully build"


# #############################################################################
