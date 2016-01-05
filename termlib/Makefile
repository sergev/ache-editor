#
# $Header: Makefile,v 1.2 89/08/01 14:58:15 ache Exp $
#
#MEM     = -Ml2
#MEM=
DEFS    = -DUNIXV7 -DCM_N -DCM_GT -DCM_B -DCM_D
LOCALINCL = ../../include/$O
INCL    = -I. -I$(LOCALINCL) -I../ff3/
CFLAGS= $(DEBUG) -O $(MEM) $(DEFS) $(INCL)
OBJS = termcap.o tgoto.o tputs.o
LIB=$O/libtermlib.a
CFILES=$(OBJS:.o=.c)
MISC=Makefile make286 make386 makesun

all: $(LIB)

install cp: $(LIB)
	cp $(LIB) ${DESTDIR}/usr/lib/libtermcap.a
	-rm -f ${DESTDIR}/usr/lib/libtermlib.a
	ranlib ${DESTDIR}/usr/lib/libtermcap.a
	ln ${DESTDIR}/usr/lib/libtermcap.a ${DESTDIR}/usr/lib/libtermlib.a

$(LIB): $(OBJS)
	-rm -f $(LIB)
	ar rcv $(LIB) `lorder ${OBJS} | tsort`
	ranlib $(LIB)

termcap.o: ../ff3/ff.local.h

clean:
	-rm -f $(OBJS) *.b $(LIB)

VGRIND= csh /usr/local/bin/vgrind
vgrind:
	cp /dev/null index
	${VGRIND} -h "Termcap library" termcap.c tputs.c tgoto.c tparm.c
	${VGRIND} -h "Termcap library" -x index

echo: $(CFILES) $(MISC)
	@echo $(CFILES) $(MISC)
