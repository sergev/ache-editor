LIB     = $O/libtmp.a
MALLOC  =
SHELL   = /bin/sh
#MEM     = -Ml2
#MEM=
DEBUG   = -g
INC     = ../../include/$O/
CFLAGS  = $(DEBUG) -O $(MEM) -DUNIXV7 -I$(INC)
MISC    = Makefile make286 make386 makesun

# ranlib is a late v7ism. if you don't have it, define it to echo
RANLIB = ranlib

OFILES  =\
	getlong.o \
	getskip.o \
	getshort.o \
	putlong.o \
	putshort.o \
	empty.o \
	inp_flush.o \
	fill.o \
	move.o \
	flock.o
#        $(MALLOC)

CFILES=$(OFILES:.o=.c)

$(LIB): $(OFILES)
	rm -f $(LIB)
	ar r $(LIB) `lorder $(OFILES) | tsort`
	$(RANLIB) $(LIB)

# Malloc is here for testing purposes only.  If it is compiled with 'debug'
# defined, it runs slower but does rigorous checking to insure that the
# heap has not been corrupted.
# It is also modified to call the E 'fatal' routine if an ASSERT fails.
# Has 'veralloc()' to verify the arena
#malloc.o: malloc.c
#        $(CC) -c -Ddebug -O -g malloc.c
#        $(CC) -c -Ddebug -Dlongdebug -O -g malloc.c

distribution:   clean
clean:
	rm -f $(OFILES) $(LIB)

tags: $(CFILES)
	ctags -w $(CFILES)

fill.o: $(INC)c_env.h $(INC)localenv.h
move.o: $(INC)c_env.h $(INC)localenv.h

echo: $(MISC) $(CFILES) test_fill.c test_move.c
	@echo $(MISC) $(CFILES) test_fill.c test_move.c
