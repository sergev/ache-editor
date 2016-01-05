SHELL = /bin/sh
MAKE = make CC=gcc
CP = cp
FF = ff3
E = le
VER = e17
EDIR = /usr/local/lib/e# see also e17/e.h fill/Makefile

all: test $(VER)/le #$(VER)/pres #

test:
	@[ "$(MAKE)" = make ] && ( echo "You must NOT run this directly"; exit 1 ); exit 0

install: mkdirs etc_e_files installroot

installroot:
	@echo 'you should be root when you do this'
	cd fill; $(MAKE) install
	-mv /usr/local/bin/le /usr/local/bin/le.old
	$(CP) $(VER)/le /usr/local/bin
	strip /usr/local/bin/le
	if [ -x /usr/bin/mcs ]; then /usr/bin/mcs -d /usr/local/bin/le; fi
	chmod 5755 /usr/local/bin/le
#       $(CP) $(VER)/run $(EDIR)
#       chmod a-w $(EDIR)

mkdirs:
	-mkdir $(EDIR)
	chown root $(EDIR)
	chmod o-w $(EDIR)

$(VER)/le: fill/fill fill/just fill/center \
	   lib/libtmp.a la1/libla.a $(FF)/libff.a termlib/libtermlib.a
	cd $(VER); $(MAKE) $X

lib/libtmp.a:
	cd lib; $(MAKE) $X

la1/libla.a: $(FF)/libff.a
	cd la1; $(MAKE) $X

$(FF)/libff.a:
	cd $(FF); $(MAKE) $X

termlib/libtermlib.a:
	cd termlib; $(MAKE) $X

$(VER)/run:
	cd $(VER); $(MAKE) run

etc_e_files:
	$(CP) e/errmsg* $(EDIR)
#       If the editor crashes, this doc is typed out at the user's terminal.
#       You should edit this file and $(MAKE) it says what you want for your
#       site.
	$(CP) e/Crashdoc* $(EDIR)
	$(CP) e/recovermsg* $(EDIR)
	$(CP) e/*help* $(EDIR)
	chmod 444 $(EDIR)/*

fill/fill fill/just fill/center:
	cd fill; $(MAKE) $X

$(VER)/pres: lib/libtmp.a
	cd $(VER); $(MAKE) pres

distribution:
	cd lib; $(MAKE) distribution
	cd $(FF); $(MAKE) distribution
	cd la1; $(MAKE) distribution
	cd $(VER); $(MAKE) distribution
	cd fill; $(MAKE) distribution

clean:
	cd lib; $(MAKE) clean
	cd $(FF); $(MAKE) clean
	cd la1; $(MAKE) clean
	cd $(VER); $(MAKE) clean
	cd fill; $(MAKE) clean

echo:
	@cd e17; $(MAKE) echo | sed 's/^/e17\//;s/ / e17\//g'
	@cd ff3; $(MAKE) echo | sed 's/^/ff3\//;s/ / ff3\//g'
	@cd la1; $(MAKE) echo | sed 's/^/la1\//;s/ / la1\//g'
	@cd lib; $(MAKE) echo | sed 's/^/lib\//;s/ / lib\//g'
	@cd termlib; $(MAKE) echo | sed 's/^/termlib\//;s/ / termlib\//g'
	@cd fill; $(MAKE) echo | sed 's/^/fill\//;s/ / fill\//g'
	@echo ../include e Makefile make286 make386 makesun
