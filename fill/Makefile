EDIR = /usr/local/lib/e
SHELL = /bin/sh
E = le
# MEM = -Ml2
CFLAGS = -O $(MEM) $(DEBUG) -DNOEDIAG #-traditional
LDFLAGS = # -i $(MEM) $(DEBUG)
LIBS= #-lc_s
MISC = Makefile make286 make386 makesun
CFILES=fill.c center.c

all: fill just center

fill: fill.o
	$(CC) $(LDFLAGS) fill.o -o fill $(LIBS)

just: fill
	-rm -f just
	ln fill just

center: center.o
	$(CC) $(LDFLAGS) center.o -o center $(LIBS)

install: all
	cp fill $(EDIR)
	strip $(EDIR)/fill
	if [ -x /usr/bin/mcs ]; then /usr/bin/mcs -d $(EDIR)/fill; fi
	-rm -f $(EDIR)/just
	ln $(EDIR)/fill $(EDIR)/just
	cp center $(EDIR)
	strip $(EDIR)/center
	if [ -x /usr/bin/mcs ]; then /usr/bin/mcs -d $(EDIR)/center; fi

clean:
	rm -f *.o
	rm -f fill just center

distribution: clean

echo: $(CFILES) $(MISC)
	@echo $(CFILES) $(MISC)
