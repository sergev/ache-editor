/*    workspace manipulation code. */

#include "e.h"
#include "e.inf.h"
#include "e.tt.h"
#include "e.wi.h"

extern Flag startupflg;
extern int curfnum;

void switchfile ();
void exchgwksp ();
void savewksp ();
void takefile ();
static void swwksp ();
static void releasewk ();
void inforange ();
void infoprange ();
void insertwlist ();
static void appendwlist ();
void extractwlist ();
void freewlist ();
void freewk ();
S_wksp *findwlist ();
S_wksp *getwkbynum ();
int getnumbywk ();
Flag wkused ();

/*    Switch to next/prev wksp if there is one, else do error mesg. */
void
switchfile (nextflg)
Flag nextflg;
{
    getline (-1);

    if (!swfile (nextflg))
	mesg (ERRALL + 1, ediag("No alternate file",
				"Нет альтернативного файла"));
    else if (visualtabs) {
	savecurs ();
	drawtabs (curwin, WIN_ACTIVE);
	restcurs ();
    }
}

void
takefile (file)
Reg1 char *file;
{
    Reg3 Fn newfn;
    Reg2 S_wksp *w;

    if (   (newfn = hvname (file)) != -1
	&& (w = findwlist (newfn)) != (S_wksp *) NULL
       ) {
	if (   (curwin->winflgs & TRACKSET)
	    && curwksp != (S_wksp *) NULL
	   ) {
	    w->ccol = curwksp->ccol;
	    w->clin = curwksp->clin;
	}
	swwksp (w);
	return;
    }
    file = append (file, "\n");
    commedit (file); /* Free in */
}

/*    Return NO if no wksp else switch to wksp and return YES. */
Flag
swfile (nextflg)
Flag nextflg;
{
    Reg1 S_wksp *wk;

    wk = nextflg ? curwksp->next_wksp : curwksp->prev_wksp;
    if (wk == (S_wksp *) NULL)
	wk = nextflg ? first_wksp : last_wksp;
    if (wk == (S_wksp *) NULL || wk == curwksp)
	return NO;
    if (wk->wfile == NULLFILE)
	return NO;

    if (curwin->winflgs & TRACKSET) {
	wk->ccol = cursorcol;
	wk->clin = cursorline;
    }

    swwksp (wk);

    return YES;
}

static
void
swwksp (wk)
Reg1 S_wksp *wk;
{
    curwin->wksp = wk;
    exchgwksp ();
    HelpActive = ((fileflags[curfile] & HELP) != 0);
    putupwin ();
    poscursor (curwksp->ccol, curwksp->clin);
    if (HelpActive)
	HelpGotoMarg (NO);
    curfnum = getnumbywk (curwksp);
}

/*    Exchange wksp and next/prev wksp. */
void
exchgwksp ()
{
    Reg1 S_wksp *cwksp;

    if (curwksp != (S_wksp *) NULL) {
	if (   curfile != deffn
	    || startupflg
	    || wkused (curwksp)
	   )
	    savewksp (cwksp = curwksp);
	else
	    freewk (curwksp);
    }

    curwksp = curwin->wksp;
    curfile = curwksp->wfile;
    curlas = &curwksp->las;
    inforange (curwksp->wkflags & RANGESET);
    limitcursor ();
}

/* Save workspace position in lastlook[pwk->wfile] */
void
savewksp (pwk)
Reg1 S_wksp *pwk;
{
    Reg2 S_wksp *lwksp;

    if (pwk == (S_wksp *) NULL)
	return;
    if (curwksp == pwk) {
	pwk->ccol = cursorcol;
	pwk->clin = cursorline;
	curwksp = (S_wksp *) NULL;
    }
    if (pwk->wfile == NULLFILE)
	return;
    lwksp = &lastlook[pwk->wfile];

    (void) la_align (&pwk->las, &lwksp->las);
    lwksp->wcol = pwk->wcol;
    lwksp->wlin = pwk->wlin;
    lwksp->ccol = pwk->ccol;
    lwksp->clin = pwk->clin;
}

/*
    Release the file in workspace.
    La_close the range streams, if any, for workspace 'wk'.
*/
static
void
releasewk (wk)
Reg1 S_wksp *wk;
{
    if (wk == (S_wksp *) NULL)
	return;
    if (wk->wfile != NULLFILE) {
	(void) la_close (&wk->las);
	wk->wfile = NULLFILE;
    }
    wk->wkflags &= ~RANGESET;
    if (wk->brnglas) {
	(void) la_close (wk->brnglas);
	wk->brnglas = (La_stream *) NULL;
	(void) la_close (wk->ernglas);
    }
}

static char *lastwhere = " ";

/*
    Update the RANGE display on the info line
*/
void
inforange (onoff)
Reg1 Flag onoff;
{
    static Flag wason;

    if ((onoff = onoff ? YES : NO) ^ wason) {
	if (wason = onoff) {
	    d_put (VCCMD);
	    info (inf_range + 1, 3, "RNG", NO);
	}
	else {
	    info (inf_range, 4, "", NO);
	    lastwhere = " ";
	}
    }
}

/*
    Update the position portion of the RANGE display on the info line
*/
void
infoprange (line)
Reg2 Nlines line;
{
    Reg1 char *where;

    if (line < la_lseek (curwksp->brnglas, 0, 1))
	where = "<";
    else if (line > la_lseek (curwksp->ernglas, 0, 1))
	where = ">";
    else
	where = "=";
    if (   *lastwhere == ' '
	|| *where != *lastwhere
       ) {
	info (inf_range, 1, where, NO);
	lastwhere = where;
    }
}

static
void
appendwlist (w)
Reg1 S_wksp *w;
{
    if ((w->prev_wksp = last_wksp) != (S_wksp *) NULL)
	last_wksp->next_wksp = w;
    else
	first_wksp = w; /* Начало списка */
    w->next_wksp = (S_wksp *) NULL;
    last_wksp = w;
}

void
insertwlist (w, where)
Reg1 S_wksp *w;
Reg2 S_wksp *where;
{
    if (where == (S_wksp *) NULL) {
	appendwlist (w);
	return;
    }
    if ((w->next_wksp = where->next_wksp) == (S_wksp *) NULL)
	last_wksp = w;
    else
	where->next_wksp->prev_wksp = w;
    w->prev_wksp = where;
    where->next_wksp = w;
}

void
extractwlist (w)
Reg1 S_wksp *w;
{
    if (w->prev_wksp == (S_wksp *) NULL)
	first_wksp = w->next_wksp;
    else
	w->prev_wksp->next_wksp = w->next_wksp;
    if (w->next_wksp == (S_wksp *) NULL)
	last_wksp = w->prev_wksp;
    else
	w->next_wksp->prev_wksp = w->prev_wksp;
}

S_wksp *
findwlist (fn)
Reg2 Fn fn;
{
    Reg1 S_wksp *w;

    if ((w = curwksp) == (S_wksp *) NULL)
	return (S_wksp *) NULL;

    for (;;) {
	w = w->next_wksp;
	if (w == (S_wksp *) NULL)
	    w = first_wksp;
	if (w == curwksp)
	    break;
	if (w->wfile == fn)
	    return w;
    }

    return (S_wksp *) NULL;
}

void
freewk (w)
Reg1 S_wksp *w;
{
    releasewk (w);
    extractwlist (w);
    sfree ((char *) w);
}

void
freewlist ()
{
    while (last_wksp != (S_wksp *) NULL)
	freewk (last_wksp);
}

S_wksp *
getwkbynum (n)
Reg3 int n;
{
    Reg1 S_wksp *w;
    Reg2 int i;

    for (i = 0, w = first_wksp; w != (S_wksp *) NULL; w = w->next_wksp)
	if (   w->wfile != NULLFILE
	    && !(fileflags[w->wfile] & (DELETED|RENAMED))
	   ) {
	    if (i == n)
		return w;
	    else
		i++;
	}
    return (S_wksp *) NULL;
}

int
getnumbywk (tw)
Reg3 S_wksp *tw;
{
    Reg1 S_wksp *w;
    Reg2 int i;

    for (i = 0, w = first_wksp; w != (S_wksp *) NULL; w = w->next_wksp)
	if (   w->wfile != NULLFILE
	    && !(fileflags[w->wfile] & (HELP|DELETED|RENAMED))
	   ) {
	    if (w == tw)
		return i;
	    else
		i++;
	}
    return -1;
}

int
countwk ()
{
    Reg1 S_wksp *w;
    Reg2 int i;

    for (i = 0, w = first_wksp; w != (S_wksp *) NULL; w = w->next_wksp)
	if (   w->wfile != NULLFILE
	    && !(fileflags[w->wfile] & (DELETED|RENAMED))
	   )
	    i++;
    return i;
}

Flag
wkused (wk)
Reg2 S_wksp *wk;
{
    Reg1 Small win;

    if (wk == (S_wksp *) NULL)
	return NO;
    for (win = 0; win < nwinlist; win++)
	if (winlist[win]->wksp == wk)
	    return YES;
    return NO;
}

#ifdef  DEBUG_WKSP
dumpwksp()
{
	Reg1 S_wksp *w;

	for (w = first_wksp; w; w = w->next_wksp) {
		dbgpr ("%x: next %x, prev %x, file %s, c %d l %d, wc %d wl %d\n",
			w, w->next_wksp, w->prev_wksp,
			names[w->wfile], w->ccol, w->clin,
			w->wcol, w->wlin);
	}
	dbgpr("********\n");
}
#endif
