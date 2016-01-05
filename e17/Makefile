#+-------------------------------------------------------+
#|                      *                                |
#|                      *                                |
#|                      *       ***                      |
#|                      *      *   *                     |
#|                      *      ****                      |
#|                      *      *                         |
#|                      ******  ***                      |
#+-------------------------------------------------------+
#
#   CFLAGS=-g and LIBC=-lg for debugging on the VAX
#   DEFS=-DTERMCAP and TERMCAP=-ltermcap
#   DEFS=-DPROFILE and PROFILE=-p
E       = $O/le
SHELL	= /bin/sh
RM      = rm -f
EDIR    = /usr/local/lib/e/
LA      = ../la1/
LALIB   = ../la1/$O/libla.a
FF      = ../ff3/
FFLIB   = ../ff3/$O/libff.a
MISC    = ../lib/
MISCLIB = ../lib/$O/libtmp.a
DEFS    = -DUNIXV7 -DTERMCAP -DCHANGECTYPE# -DANSI -DCTYPE -DSECURITY -DNOEDIAG -DDEBUG_KBFILE -DDEBUGALLOC -DPROFILE
LOCALINCL = ../../include/$O/
INCL    = -I. -I$(LA) -I$(FF) -I$(LOCALINCL)
SIG_INCL =<signal.h>
SGTT_INCL =<sgtty.h>
#MEM = -Ml2 -LARGE -SEG 175
#MEM =
DEBUG = -g
CFLAGS  = $(DEBUG) -O $(MEM) $(DEFS) $(INCL) $(PROFILE)
LDFLAGS = $(DEBUG)
TERMCAP = ../termlib/$O/libtermlib.a
LIBC = # -lcompat
MISC= newversion make286 make386 makesun Makefile

HFILES  = \
	e.cc.h \
	e.cm.h \
	e.e.h \
	e.fn.h \
	e.func.h \
	e.h \
	e.help.h \
	e.hi.h \
	e.inf.h \
	e.it.h \
	e.m.h \
	e.mac.h \
	e.mk.h \
	e.ml.h \
	e.ru.h \
	e.se.h \
	e.sg.h \
	e.t.h \
	e.tt.h \
	e.wi.h \
	e.up.h \
	sterm.h

OFILES = \
	e.o \
	e.chk.o \
	e.cm.o \
	e.d.o \
	e.e.o \
	e.f.o \
	e.fix.o \
	e.gett.o \
	e.gets.o \
	e.help.o \
	e.iit.o \
	e.inp.o \
	e.it.o \
	e.la.o \
	e.m.o \
	e.m2.o \
	e.main.o \
	e.mac.o \
	e.m1.o \
	e.mesg.o \
	e.mk.o \
	e.nm.o \
	e.p.o \
	e.pa.o \
	e.par.o \
	e.prot.o \
	e.put.o \
	e.puts.o \
	e.q.o \
	e.r.o \
	e.ru.o \
	e.sb.o \
	e.se.o \
	e.sg.o \
	e.sup.o \
	e.sv.o \
	e.t.o \
	e.t1.o \
	e.tags.o \
	e.tb.o \
	e.tt.o \
	e.u.o \
	e.wi.o \
	e.wk.o \
	e.words.o \
	e.x.o \
	lcmp.o \
	regex.o \
	scroll.o \
	get.o \
	standard.o \
	xlate.o \
	tcap.o \
	xl_tcap.o

CFILES = $(OFILES:.o=.c)

$(E):   newrev load
	@size $(E)

load:   $(OFILES) $(LALIB) $(FFLIB) $(MISCLIB) $(TERMCAP)
	@-mv $(E) $(E).old
	$(CC) $(LDFLAGS) $(OFILES) \
		$(TERMCAP) $(LALIB) $(FFLIB) \
		$(MISCLIB) $(LIBC) -o $(E)

calls: $(CFILES)
	calls $(DEFS) $(INCL) $(CFILES) > calls

tags: $(CFILES) $(HFILES)
	ctags -w $(HFILES) $(CFILES)

Date: $(HFILES) $(CFILES) Make.*
	@echo $?

e.r.c: newversion
	./newversion

newrev: e.r.o

$(LALIB):

$(FFLIB):

$(MISCLIB):

depend:
	/bin/sh Makedep $(HFILES) $(CFILES)

lint:;  lint -bxa $(DEFS) $(INCL) $(CFILES) \
	../lib/llib-ltmp \
	$(FF)llib-lff \
	$(LA)llib-lla \
	> lint

vlint:;  lint -LARGE $(DEFS) $(INCL) $(CFILES) \
	$(FF)llib-ltmp.c \
	$(LA)llib-lff.c \
	llib-lla.c \
	> vlint

$O/pres: pres.o $(MISCLIB)
	-mv $@ $@.old
	$(CC) pres.o $(MISCLIB) -o $@

$O/run:    run.c
	$(CC) -O -o $@ run.c

distribution: clean
	$(RM) $(E) $(E).old $O/run $O/run.old $O/pres $O/pres.old

clean:
	$(RM) $(OFILES) $O/pres.o

echo: $(MISC) run.c pres.c install.c
	@echo $(HFILES) $(CFILES) $(MISC) run.c pres.c install.c

e.r.o: e.r.c
	$(CC) $(CFLAGS) -c $<

install: install.c
	$(CC) $(CFLAGS) install.c -o install

#
#
#e.h:    $(LOCALINCL)/c_env.h $(.PRECIOUS)
#        touch e.h
#
#e.sg.h: $(.PRECIOUS)
#        touch e.sg.h


# DO NOT DELETE THIS LINE

e.h: $(LOCALINCL)c_env.h
e.h: $(LOCALINCL)localenv.h
e.h: $(LA)la.h
e.h: e.t.h
e.h: e.up.h
	touch $@
e.m.h: e.ml.h
	touch $@
sterm.h: e.h
sterm.h: e.tt.h
	touch $@
e.o: e.h
e.o: e.e.h
e.o: e.tt.h
e.o: e.wi.h
e.o: e.fn.h
e.o: e.inf.h
e.chk.o: e.h
e.chk.o: e.tt.h
e.chk.o: e.fn.h
e.cm.o: e.h
e.cm.o: e.e.h
e.cm.o: e.m.h
e.cm.o: e.ru.h
e.cm.o: e.mk.h
e.cm.o: e.cm.h
e.cm.o: e.wi.h
e.d.o: e.h
e.d.o: e.tt.h
e.e.o: e.h
e.e.o: e.m.h
e.e.o: e.mk.h
e.e.o: e.cm.h
e.e.o: e.e.h
e.f.o: e.h
e.fix.o: e.h
e.fix.o: e.sg.h
e.gett.o: e.h
e.gett.o: e.tt.h
e.gets.o: e.h
e.gets.o: e.fn.h
e.gets.o: e.tt.h
e.gets.o: e.wi.h
e.gets.o: e.hi.h
e.help.o: e.h
e.help.o: e.mk.h
e.help.o: e.cm.h
e.help.o: e.cc.h
e.help.o: e.ml.h
e.help.o: e.help.h
e.iit.o: e.h
e.iit.o: e.sg.h
e.iit.o: e.cc.h
e.iit.o: e.it.h
e.inp.o: e.h
e.inp.o: e.inf.h
e.inp.o: e.cc.h
e.inp.o: e.mk.h
e.inp.o: e.m.h
e.inp.o: e.tt.h
e.inp.o: e.mac.h
e.it.o: e.h
e.it.o: e.it.h
e.la.o: e.h
e.la.o: e.m.h
e.la.o: e.help.h
e.m.o: e.h
e.m.o: e.cc.h
e.m.o: e.e.h
e.m.o: e.cm.h
e.m.o: e.mk.h
e.m.o: e.inf.h
e.m.o: e.m.h
e.m.o: e.tt.h
e.m.o: e.se.h
e.m2.o: e.h
e.m2.o: e.cc.h
e.m2.o: e.e.h
e.m2.o: e.cm.h
e.m2.o: e.mk.h
e.m2.o: e.m.h
e.main.o: e.h
e.mac.o: e.h
e.mac.o: e.inf.h
e.mac.o: e.tt.h
e.mac.o: e.m.h
e.mac.o: e.cm.h
e.mac.o: e.mac.h
e.m1.o: e.h
e.m1.o: e.cc.h
e.m1.o: e.fn.h
e.m1.o: e.tt.h
e.m1.o: e.se.h
e.mesg.o: e.h
e.mesg.o: e.tt.h
e.mk.o: e.h
e.mk.o: e.inf.h
e.mk.o: e.m.h
e.mk.o: e.tt.h
e.nm.o: e.h
e.nm.o: e.inf.h
e.nm.o: e.fn.h
e.nm.o: e.tt.h
e.nm.o: e.m.h
e.nm.o: e.cm.h
e.p.o: e.h
e.p.o: e.cc.h
e.p.o: e.cm.h
e.pa.o: e.h
e.pa.o: e.m.h
e.pa.o: e.mk.h
e.pa.o: e.cm.h
e.par.o: e.h
e.par.o: e.cc.h
e.par.o: e.tt.h
e.par.o: e.m.h
e.par.o: e.hi.h
e.prot.o: e.h
e.put.o: e.h
e.put.o: e.m.h
e.put.o: e.mk.h
e.put.o: e.cm.h
e.put.o: e.e.h
e.puts.o: e.h
e.puts.o: e.fn.h
e.puts.o: e.hi.h
e.puts.o: e.m.h
e.puts.o: e.tt.h
e.q.o: e.h
e.q.o: e.cc.h
e.q.o: e.m.h
e.q.o: e.cm.h
e.ru.o: e.h
e.ru.o: e.e.h
e.ru.o: e.m.h
e.ru.o: e.cm.h
e.ru.o: e.ru.h
e.ru.o: e.mk.h
e.sb.o: e.h
e.sb.o: e.fn.h
e.sb.o: e.sg.h
e.se.o: e.h
e.se.o: e.m.h
e.se.o: e.cm.h
e.se.o: e.mk.h
e.se.o: e.se.h
e.sg.o: e.h
e.sg.o: e.tt.h
e.sg.o: e.sg.h
e.sup.o: e.h
e.sup.o: e.tt.h
e.sup.o: e.fn.h
e.sv.o: e.h
e.sv.o: e.m.h
e.sv.o: e.mk.h
e.sv.o: e.cm.h
e.t.o: e.h
e.t.o: e.cc.h
e.t.o: e.m.h
e.t.o: e.tt.h
e.t.o: e.wi.h
e.t1.o: e.h
e.t1.o: e.tt.h
e.t1.o: e.m.h
e.t1.o: e.inf.h
e.tags.o: e.h
e.tags.o: e.m.h
e.tags.o: e.cm.h
e.tags.o: e.se.h
e.tb.o: e.h
e.tb.o: e.m.h
e.tb.o: e.mk.h
e.tb.o: e.cm.h
e.tb.o: e.tt.h
e.tb.o: e.wi.h
e.tt.o: e.h
e.tt.o: e.tt.h
e.tt.o: standard.c
e.tt.o: tcap.c
e.u.o: e.h
e.u.o: e.cc.h
e.u.o: e.m.h
e.u.o: e.cm.h
e.u.o: e.tt.h
e.u.o: e.wi.h
e.wi.o: e.h
e.wi.o: e.m.h
e.wi.o: e.cm.h
e.wi.o: e.mk.h
e.wi.o: e.inf.h
e.wi.o: e.tt.h
e.wi.o: e.wi.h
e.wk.o: e.h
e.wk.o: e.inf.h
e.wk.o: e.tt.h
e.wk.o: e.wi.h
e.words.o: e.h
e.words.o: e.cm.h
e.words.o: e.cc.h
e.words.o: e.e.h
e.x.o: e.h
e.x.o: e.hi.h
e.x.o: e.e.h
e.x.o: e.cc.h
e.x.o: e.inf.h
e.x.o: e.m.h
e.x.o: e.sg.h
scroll.o: sterm.h
get.o: sterm.h
standard.o: sterm.h
standard.o: e.cc.h
xlate.o: sterm.h
xlate.o: e.sg.h
tcap.o: sterm.h
xl_tcap.o: sterm.h
lcmp.o: e.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
