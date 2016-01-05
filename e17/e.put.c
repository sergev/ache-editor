/*    put qbuffers into the file */

#include "e.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.cm.h"
#include "e.e.h"

/*NOXXSTR*/
static
S_looktbl qbuftable[] = {
    "adjust"  , QADJUST ,
    "box"     , QBOX    ,
    "close"   , QCLOSE  ,
    "erase"   , QERASE  ,
    "pick"    , QPICK   ,
    "run"     , QRUN    ,
    0         , 0
};
/*YESXSTR*/
Small insqbuf;
Small nbufargs;

/*
    Do the cover, overlay, underlay, blot, and insert commands
    'cmd':
    0 COVER
    1 OVERLAY
    2 UNDERLAY
    3 BLOT
    4 -BLOT
    5 INSERT
*/
Cmdret
insert (cmd)
int cmd;
{
    char *cp;
    Cmdret tmp;
    Cmdret qbufopts ();

    if (opstr[0] == '\0')
	return CRNEEDARG;

    nbufargs = 0;
    cp = cmdopstr;
    if ((tmp = scanopts (&cp, NO, qbuftable, qbufopts)) < 0)
	return tmp;
/*       3 marked area        \
/*       2 rectangle           \
/*       1 number of lines      > may stopped on an unknown option
/*       0 no area spec        /
/*      -2 ambiguous option     ) see parmlines, parmcols for type of area
/*   <= -3 other error
/**/
    if (*cp != '\0')
	return CRBADARG;
    if (nbufargs < 1)
	return CRNEEDARG;
    if (nbufargs > 1)
	return CRTOOMANYARGS;

    switch (tmp) {
    case 0:
	return ed (OPCOVER << cmd,
		   insqbuf,
		   curwksp->wlin + cursorline,
		   curwksp->wcol + cursorcol, (Nlines) 0, (Ncols) 0, YES);

    case 1:
    case 2:
	if (insqbuf != QBOX)
	    return CRBADARG;
	return ed (OPCOVER << cmd,
		   insqbuf,
		   curwksp->wlin + cursorline,
		   curwksp->wcol + cursorcol, parmlines, parmcols, YES);

    case 3:
	if (insqbuf != QBOX)
	    return NOMARKERR;
	return edmark (OPCOVER << cmd, insqbuf);
    }
    return tmp;
}

/*
    Get the buffer name for the insert command.
    Called only by insert().
*/
/* not static */
Cmdret
qbufopts (cp, str, tmp, equals)
Reg1 char *cp;
char **str;
Small tmp;
Flag equals;
{
    nbufargs++;
    if (equals)
	return CRBADARG;
    insqbuf = qbuftable[tmp].val;
    for (; *cp && *cp == ' '; cp++)
	continue;
    *str = cp;
    return 1;
}


/*
    Do the -pick, -close, and -erase commands.
    'buf':
    0 -close
    1 -pick
    2 -erase
*/
Cmdret
insbuf (buf)
Small buf;
{
    if (curmark)
	return NOMARKERR;
    if (*cmdopstr)
	return CRTOOMANYARGS;

    return ed (OPINSERT,
	       buf,
	       curwksp->wlin + cursorline,
	       curwksp->wcol + cursorcol, (Nlines) 0, (Ncols) 0, YES);
}
