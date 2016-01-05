#include "e.h"
#include "e.fn.h"
#include "e.tt.h"
#include "e.wi.h"
#include "e.hi.h"
#include <varargs.h>

extern void makestate ();
extern void getout ();
extern void getskip ();
extern S_wksp *getwkbynum ();
extern S_wksp *findwlist ();
extern Flag startupflg;
extern int curfnum;
/*  After the two-byte revision number in the keystroke file
/*  comes a character telling what to use from the state file.
/**/
#define NO_WINDOWS  ' ' /* don't use any files from state file */
#define ALL_WINDOWS '!' /* use all windows and files */

/*
    Set up things as per the state file.
    See State.fmt for a description of the state file format.
    The 'ichar' argument is still not totally cleaned up from the way
    this was originally written.
    If ichar ==
      ALL_WINDOWS use all windows and files from state file.
      ONE_WINDOW  set up one full-size window and don't put up any files.
*/
void
getstate (ichar)
char ichar;
{
    Small winnum;
    Reg1 FILE *gbuf;
    Reg2 Short i;
    Reg3 Short  n;
    Reg4 S_wksp *w;
    Reg5 Short lim;

    if (   ichar == NO_WINDOWS
	|| notracks
	|| (gbuf = fopen (rfile, "r")) == (FILE *) NULL
       ) {
    OpErr:
	makestate (ichar != NO_WINDOWS);
	return;
    }
    if (!setflock (fileno (gbuf), 0, 0)) {
	fclose (gbuf);
	goto OpErr;
    }

    if ( (i = getshort (gbuf)) != revision) {
	if (i >= 0 || feoferr (gbuf))
	    goto badstart;
	if (i != -1) {
	    fclose (gbuf);
	    unlink (rfile);
	    getout (YES, ediag(
	       "Startup file %s was made by other revision of LE (%d).\n\
File deleted, try again.",
	       "Файл состояния %s сделан другой версией LE (%d).\n\
Файл удален, попробуйте снова."),
		    rfile, -i);
	}
    }

    /* terminal type */
    Block {
	Reg2 Small nletters;

	nletters = getshort (gbuf);
	if (feoferr (gbuf))
	    goto badstart;
	getskip (nletters, gbuf);
    }

    Block {
	Reg2 Slines nlin;
	Reg3 Scols ncol;

	nlin = getc (gbuf);
	ncol = getc (gbuf);
	if (feoferr (gbuf))
	    goto badstart;
	if (   nlin != term.tt_height
	    || ncol != term.tt_width
	   )
	    ichar = NO_WINDOWS;
/***********************
	    getout (YES, ediag(
"Startup file: %s was made for a terminal with a different screen size.\n\
(%d X %d).  Delete it or give a filename in call arguments.",
"Файл состояния %s сделан для терминала с другими размерами экрана.\n\
(%d X %d).  Удалите его или укажите имя файла в аргументах вызова."),
		    rfile, nlin, ncol);
************************/
    }

    /* sttime     = */ getlong (gbuf);

    ntabs = getshort (gbuf);
    if (feoferr (gbuf))
	goto badstart;
    stabs = max (ntabs, NTABS);
    tabs = (ANcols *) salloc (stabs * sizeof *tabs, YES);

    if (ntabs > 0) Block {
	Reg2 int ind;

	for (ind = 0; ind < ntabs; ind++) {
	    tabs[ind] = (ANcols) getshort (gbuf);
	    if (feoferr (gbuf))
		goto badstart;
	}
    }

    linewidth = getshort (gbuf);

    if (i = getshort (gbuf)) {
	if (feoferr (gbuf))
	    goto badstart;
	searchkey = salloc (i, YES);
	if (fread (searchkey, sizeof (char), i, gbuf) != i)
	    goto badstart;
    }

    if (i = getshort (gbuf)) {
	if (feoferr (gbuf))
	    goto badstart;
	if (nhistory != i && hcmd != (S_cmd *) NULL) {
	    sfree ((char *) hcmd);
	    hcmd = (S_cmd *) NULL;
	}
	nhistory = i;
	if (hcmd == (S_cmd *) NULL)
	    hcmd = (S_cmd *) salloc (nhistory * sizeof (S_cmd), YES);
    }

    for (i = 0; i < nhistory; i++) Block {
	Reg3 Short j;

	if (j = getshort (gbuf)) Block {
	    Reg4 Short k;

	    if (feoferr (gbuf))
		goto badstart;
	    hcmd[i].len = j;
	    k = j / LPARAM + 1;
	    hcmd[i].p = salloc (hcmd[i].l = k * LPARAM, YES);
	    if (fread (hcmd[i].p, sizeof (char), j + 1, gbuf) != j + 1)
		goto badstart;
	}
    }
    if (i = getshort (gbuf)) Block {
	Reg3 Short j;

	if (feoferr (gbuf))
	    goto badstart;
	ccmd.len = i;
	j = i / LPARAM + 1;
	ccmd.p = salloc (ccmd.l = j * LPARAM, YES);
	if (fread (ccmd.p, sizeof (char), i + 1, gbuf) != i + 1)
	    goto badstart;
    }

    insmode = getc (gbuf);
    inplace = getc (gbuf);
    binary = getc (gbuf);
    visualtabs = getc (gbuf);
    offsetflg = getc (gbuf);

    if (getc (gbuf)) {  /* curmark */
	getskip (sizeof (long)
		 + sizeof (short)
		 + sizeof (char)
		 + sizeof (short),
		 gbuf);
    }

    startupflg = YES;
    freewlist ();
    n = getc (gbuf);
    if (feoferr (gbuf))
	goto badstart;
    for (i = 0; i < n; i++) Block {
	Reg4 char *fname;

	Block {
	    Reg2 Small nletters;

	    nletters = getshort (gbuf);
	    if (feoferr (gbuf))
		goto badstart;
	    fname = salloc (nletters, YES);
	    if (fread (fname, sizeof (char), nletters, gbuf) != nletters)
		goto badstart;
	}
	Block {
	    Reg2 Nlines  lin;
	    Reg3 Ncols   col;
	    Slines tmplin;
	    Scols tmpcol;

	    lin = (Nlines) getlong  (gbuf);
	    col = getshort (gbuf);
	    tmplin = getc (gbuf);
	    tmpcol = getshort (gbuf);
	    if (feoferr (gbuf))
		goto badstart;
	    if (editfile (fname, col, lin, 0, NO, NO) != 1) {
		if (eddeffile (NO) != 1) {
		    startupflg = NO;
		    makestate (NO);
		    return;
		}
		else
		    curwksp->ccol = curwksp->clin = 0;
	    }
	    else {
		curwksp->ccol = tmpcol;
		curwksp->clin = tmplin;
	    }
	    cursorcol = curwksp->ccol;
	    cursorline = curwksp->clin;
	}
    }
    savewksp (curwksp);
    lim = countwk ();

    nwinlist = getc (gbuf);
    if (feoferr (gbuf) || nwinlist > MAXWINLIST)
	goto badstart;
    winnum = getc (gbuf);
    if (ichar != ALL_WINDOWS) {
	for (n = Z; n < winnum; n++) {
	    getskip (sizeof (char)
		     + sizeof (char)
		     + sizeof (short)
		     + sizeof (char)
		     + sizeof (short)
		     + sizeof (char),
		     gbuf);
	    if (feoferr (gbuf))
		goto badstart;
	}
	winnum = 0;
	nwinlist = 1;
    }

    /* set up the windows */
    Block {
	Reg7 Small n;

	for (n = Z; n < nwinlist; n++) Block {
	    Reg5 S_window *window;
	    char wnum;
	    S_wksp *w;

	    window = winlist[n] = (S_window *) salloc (SWINDOW, YES);
	    window->prevwin = getc (gbuf);
	    if (ichar != ALL_WINDOWS)
		window->prevwin = 0;
	    Block {
		Reg2 Scols   lmarg;
		Reg3 Scols   rmarg;
		Reg4 Slines  tmarg;
		Slines  bmarg;

		tmarg = getc (gbuf);
		lmarg = getshort (gbuf);
		bmarg = getc (gbuf);
		rmarg = getshort (gbuf);
		if (feoferr (gbuf))
		    goto badstart;
		if (ichar != ALL_WINDOWS) {
		    tmarg = 0;
		    lmarg = 0;
		    bmarg = term.tt_height - 1 - NPARAMLINES;
		    rmarg = term.tt_width - 1;
		}
		setupwindow (window, lmarg, tmarg, rmarg, bmarg, 1);
	    }
	    if (n != winnum)
		drawborders (winlist[n], WIN_INACTIVE);
	    wnum = getc (gbuf);
	    if (feoferr (gbuf))
		goto badstart;
	    if (wnum < 0 || wnum >= lim) {
		if (eddeffile (NO) != 1)
		    goto badstart;
		window->wksp = curwksp;
	    }
	    else
		window->wksp = getwkbynum (wnum);
	    switchwindow (window);
	    if (n != winnum)
		chgborders = 2;
	    putupwin ();
	    chgborders = 1;
	}
    }
    startupflg = NO;

    if (feoferr (gbuf) || getc (gbuf) != EOF) {
badstart:
	fclose (gbuf);
	unlink (rfile);
	getout (YES, ediag(
"Bad state file: %s -- deleted.\n\
Try again.",
"Испорченный файл состояния: %s -- удален.\n\
Попробуйте снова."),
		 rfile);
    }
    else
	fclose (gbuf);

    switchwindow (winlist[winnum]);
    drawborders (curwin, WIN_ACTIVE);
    limitcursor ();
    poscursor (curwksp->ccol, curwksp->clin);
    if (HelpActive)
	HelpGotoMarg (NO);
    curfnum = getnumbywk (curwksp);
}

extern Flag crashed;       /* prev session crashed */

/*
    This is called to exit from the editor for conditions that arise before
    entering mainloop().
*/
/* VARARGS2 */
void
getout (filclean, va_alist)
Flag filclean;
va_dcl
{
    extern char verstr[];
    Reg1 va_list fmt;
    Reg2 char *format;

    if (windowsup)
	screenexit (YES);
    fixtty ();

    if (filclean && !crashed)
	cleanup (YES, NO);
    if (keysmoved)
	mv (bkeytmp, keytmp);

    va_start (fmt);
    format = va_arg (fmt, char *);
    (void) vprintf (format, fmt);
    va_end (fmt);

    printf (ediag(
"\nThis is %s revision %d.%d\n%s",
"\nЭто %s версии %d.%d\n%s"),
progname, -revision, subrev, verstr);
#ifdef PROFILE
    monexit (-1);
#else
    exit (-1);
#endif
    /* NOTREACHED */
}
