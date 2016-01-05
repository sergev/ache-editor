#include "e.h"
#include "e.fn.h"
#include "e.hi.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.tt.h"

/*
    Update the state file.
    See State.fmt for a description of the state file format.
*/
Flag
savestate ()
{
    Reg1 Short  i;
    char   *fname;
    Reg2 S_window *window;
    FILE *stfile;
    Reg3 S_wksp *w;

    curwksp->ccol = cursorcol;
    curwksp->clin = cursorline;

    unlink (rfile);

    if ((stfile = fopen (rfile, "w")) == (FILE *) NULL)
	return NO;

    if (!setflock (fileno (stfile), 1, 0)) {
	fclose (stfile);
	unlink (rfile);
	return NO;
    }

    /*  revision number later */
    putshort ((short) 0, stfile);

    /* terminal type name */
    putshort ((short) strlen (tname) + 1, stfile);
    fwrite (tname, sizeof (char), strlen (tname) + 1, stfile);

    putc (term.tt_height, stfile);
    putc (term.tt_width, stfile);

    /* time of start of session */
    putlong (strttime, stfile);

    /* tabstops */
    putshort ((short) ntabs, stfile);
    if (ntabs > 0) Block {
	Reg1 int ind;

	for (ind = 0; ind < ntabs; ind++)
	    putshort ((short) tabs[ind], stfile);
    }

    putshort ((short) linewidth, stfile);

    if (searchkey == 0)
	putshort ((short) 0, stfile);
    else {
	putshort ((short) strlen (searchkey) + 1, stfile);
	fwrite (searchkey, sizeof (char), strlen(searchkey) + 1, stfile);
    }

    putshort ((short) nhistory, stfile);

    for (i = 0; i < nhistory; i++) {
	putshort ((short) hcmd[i].len, stfile);
	if (hcmd[i].len > 0) {
	    hcmd[i].p[hcmd[i].len] = '\0';
	    fwrite (hcmd[i].p, sizeof (char), hcmd[i].len + 1, stfile);
	}
    }
    putshort ((short) ccmd.len, stfile);
    if (ccmd.len > 0) {
	ccmd.p[ccmd.len] = '\0';
	fwrite (ccmd.p, sizeof (char), ccmd.len + 1, stfile);
    }

    putc (insmode, stfile);
    putc (inplace, stfile);
    putc (binary, stfile);
    putc (visualtabs, stfile);
    putc (offsetflg, stfile);

    putc (curmark != 0, stfile);
    if (curmark) {
	putlong  ((long) curmark->mrkwinlin, stfile);
	putshort ((short) curmark->mrkwincol, stfile);
	putc     (curmark->mrklin, stfile);
	putshort ((short) curmark->mrkcol, stfile);
    }

    putc (countwk (), stfile);
    for (w = first_wksp; w != (S_wksp *) NULL; w = w->next_wksp)
	if (   w->wfile != NULLFILE
	    && !(fileflags[w->wfile] & (DELETED|RENAMED))
	   ) {
	    fname = names[w->wfile];
	    putshort ((short) strlen (fname) + 1, stfile);
	    fwrite (fname, sizeof (char), strlen(fname) + 1, stfile);
	    putlong  ((long) w->wlin, stfile);
	    putshort ((short) w->wcol, stfile);
	    putc     (w->clin, stfile);
	    putshort ((short) w->ccol, stfile);
	}

    putc (nwinlist, stfile);
    for (i = Z; i < nwinlist; i++)
	if (winlist[i] == curwin)
	    break;
    putc (i, stfile);
    for (i = Z; i < nwinlist; i++) {
	window = winlist[i];
	putc (window->prevwin, stfile);
	putc     (window->tmarg, stfile);
	putshort ((short) window->lmarg, stfile);
	putc     (window->bmarg, stfile);
	putshort ((short) window->rmarg, stfile);
	putc    (getnumbywk (window->wksp), stfile);
    }
    if (ferror (stfile) || fflush (stfile) == EOF) {
	fclose (stfile);
    Err:
	unlink (rfile);
	return NO;
    }
    rewind (stfile);
    putshort (revision, stfile);   /* state file is OK */
    if (fclose (stfile) == EOF)
	goto Err;
    return  YES;
}
