#+------------------------------------------------------+
#|                       ****** ******                  |
#|                       *      *                       |
#|                       *      *                       |
#|                       ****   ****                    |
#|                       *      *                       |
#|                       *      *                       |
#|                       *      *                       |
#+------------------------------------------------------+
A       = # -g
SHELL   = /bin/sh
DEFS    = -DUNIXV7
LOCALINCL = ../../include/$O/
INCL    = -I. -I$(LOCALINCL)
#MEM     = -Ml2
#MEM=
CFLAGS  = $(DEBUG) $(A) -O $(MEM) $(DEFS) $(INCL)
LIB     = $O/libff.a
MISC    = Makefile make286 make386 makesun Doc

RANLIB = ranlib

HFILES = ff.local.h ff.h

OFILES = \
	open.o \
	close.o \
	fd.o \
	free.o \
	getc.o \
	grow.o \
	pos.o \
	putc.o \
	read.o \
	seek.o \
	size.o \
	sync.o \
	flush.o \
	sort.o \
	write.o \
	point.o

CFILES = $(OFILES:.o=.c)

$(LIB): $(OFILES)
	-rm -f $(LIB)
	ar r $(LIB) `lorder $(OFILES) | tsort`
	$(RANLIB) $(LIB)

ranlib:
	$(RANLIB) $(LIB)

lint:
	lint -bxa $(DEFS) $(INCL) $(CFILES) ../lib/llib-ltmp > lint

vlint:
	vlint $(DEFS) $(INCL) $(CFILES) llib-ltmp.c > vlint

calls:  $(CFILES)
	calls $(DEFS) $(INCL) $(CFILES) > calls

tags: $(HFILES) $(CFILES)
	ctags -w $(HFILES) $(CFILES)

Calls:  $(CFILES)
	Makecalls $(CFILES)

clean : 
	rm -f *.o
	rm -f $(LIB)

$(OFILES) : $(HFILES) $(LOCALINCL)/c_env.h

#ff.local.h : ff.h $(LOCALINCL)/c_env.h
#ff.h : /usr/include/stdio.h

distribution:
	rm -f $(OFILES)
	rm -f $(LIB)

echo: $(CFILES) $(HFILES) $(MISC)
	@echo $(CFILES) $(HFILES) $(MISC)
