#
#       MAKEFILE FOR RAND "LA" LINE-ACCESS I/O PACKAGE
#
#
A       = # -g
SHELL   = /bin/sh
DEFS    = -DUNIXV7 #-DVAX
LIB     = $O/libla.a
#
FF      = ../ff3/
FFLIB   = ../ff3/$O/libff.a
LOCALINCL = ../../include/$O
INCL    = -I. -I$(FF) -I$(LOCALINCL)
#MEM     = -Ml2
#MEM=
MISC    = Makefile make386 make286 makesun doc
DEBUG   = -g
CFLAGS  = $(DEBUG) $(A) -O $(MEM) $(DEFS) $(INCL)

# ranlib is a late v7ism. if you don't have it, define it to echo
RANLIB = ranlib

HFILES = la.h la.local.h
# laopen.o can be first because it always HAS to be loaded anyway,
# and it might speed up the linker to get it out of the way.
OFILES =\
	laopen.o \
	laalign.o \
	lablank.o \
	labsize.o \
	ladeb.o \
	laerr.o \
	lafrpl.o \
	lalcnt.o \
	lalcopy.o \
	lalflush.o \
	lalget.o \
	lalpnt.o \
	laadv.o \
	lalrpl.o \
	laldel.o \
	lalcol.o \
	lalins.o \
	labreak.o \
	laclose.o \
	lalink.o \
	lalrsize.o \
	lalseek.o \
	lalwsize.o \
	lacfile.o \
	laint.o \
	laother.o \
	laabort.o

CFILES = $(OFILES:.o=.c)

$(LIB): $(OFILES)
	-rm -f $(LIB)
	ar r $(LIB) `lorder $(OFILES) | tsort`
	$(RANLIB) $(LIB)

ranlib:
	$(RANLIB) $(LIB)

lint:;
	lint -bax $(DEFS) $(INCL) $(CFILES) \
	$(FF)llib-lff ../lib/llib-ltmp > lint

vlint:;
	vlint $(DEFS) $(INCL) $(CFILES) llib-lff.c $(FF)llib-ltmp.c > vlint

Calls:  $(CFILES)
	Makecalls $(CFILES)

calls:;
	calls $(DEFS) $(INCL) $(CFILES) > calls

test:   test.o $(LIB)
	$(CC) $(CFLAGS) test.o $(LIB) $(FFLIB) $(LIBC)
	-mv test ,test
	mv a.out test

$(FFLIB): $(.PRECIOUS)
	cd $(FF); make

clean:
	rm -f $(OFILES) $(LIB) test

distribution:
	rm -f $(OFILES) $(LIB) test

tags: $(CFILES) $(HFILES)
	ctags -w $(CFILES) $(HFILES)

$(OFILES) : la.local.h la.h $(LOCALINCL)/c_env.h $(FF)ff.h

#la.local.h: la.h $(LOCALINCL)/c_env.h
#la.h: $(FF)ff.h /usr/include/stdio.h


echo: $(HFILES) $(CFILES) $(MISC)
	@echo $(HFILES) $(CFILES) $(MISC)
