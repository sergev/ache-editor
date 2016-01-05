#include "e.h"
#include "e.tt.h"
#include "e.m.h"
#include "e.inf.h"
#ifdef UNIXV7
#include <ctype.h>
#endif

extern Flag needputup; /* because the last one was aborted */
extern Flag    freshputup;      /* ignore previous stuff on these lines */
extern Flag    nodintrup;       /* disallow display interruption */
extern Slines  newcurline;

static Scols  oldccol;
static Slines oldcline;


/*
    Reset terminal if required.
    Put cursor at lower left of screen
    Scroll the screen if scroll is non-0.
*/
void
screenexit (scroll)
Flag scroll;
{
    flushkeys ();
#ifdef CLEARONEXIT
    /*  When a terminal simulator is used that keeps its own duplicate image
    /*  of the screen, screenexit is only used upon exiting, so it doesn't
    /*  need to reinitialize that image.
    /**/
    d_put (VCCICL);
    d_flush ();
    (*kbd.kb_end) ();
    d_put (VCCEND);
    d_flush ();
#else
    d_align ();
    d_put (VCCAAD);
    d_put (0);
    d_put (term.tt_height - 1);
    d_flush ();
    (*kbd.kb_end) ();
    d_put (VCCEND);
    d_flush ();
    if (scroll)
	putchar ('\n');
#endif
    windowsup = NO;
}

/*    Toggle INSERT mode. */
void
tglinsmode ()
{   d_put (VCCMD);
    info (inf_insert, 3, (insmode = !insmode) ? "INS" : "   ", NO);
}
