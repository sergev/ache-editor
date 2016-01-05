/*    Window mainpulation code. */

#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.mk.h"
#include "e.inf.h"
#include "e.tt.h"
#include "e.wi.h"

S_window *setupwindow ();

extern Flag entfstline;
extern int curfnum;

void removewindow ();
void chgwindow ();
void drawborders ();
void drawtabs ();
void draw1border ();
void switchwindow ();
void drawsides ();
void draw1side ();
void infotrack ();

/*
    Do the "window" command.
    This routine commits itself to making the new window before it
    calls editfile ().  This is not the way it should be.  It should
    verify that a window can be made here, then it should call some
    routine that is simliar to the present editfile () and if editfile
    says OK, it should draw the window, otherwise return with no new
    window made.
*/
Cmdret
makewindow (file, which)
char   *file;
Small  which;
{
    Reg2 S_window *oldwin;
    Reg3 S_window *newwin;
    Flag    horiz;                /* 1 if margin horiz, 0 if vert */
    Reg4 Scols  nearver;
    Reg5 Slines nearhor;
    Fn ocurfile;

    if (nwinlist >= MAXWINLIST) {
	mesg (ERRALL + 1, ediag("Can't make any more windows.",
				"Слишком много окон."));
	return CROK;
    }
    nearver = min (cursorcol, curwin->rtext - cursorcol);
    nearhor = min (cursorline, curwin->btext - cursorline);
    if (nearver == 0 && nearhor == 0 ||
	nearver == 0 && which == 0 ||
	nearhor == 0 && which == 1) {
	mesg (ERRALL + 1, ediag("Can't put a window there.",
				"Здесь нельзя создать окно."));
	return CROK;
    }
    else if (nearver == 0)
	horiz = YES;
    else if (nearhor == 0)
	horiz = NO;
    else if (which >= 0)
	horiz = which ? YES : NO;
    else Block {            /* Plug optimizer */
	Short b = curwin->btext * 3;
	Short h = nearhor * 3;

	if ((b /= 2) > curwin->rtext)
	    nearver = nearver * b / curwin->rtext;
	else
	    nearhor = nearhor * curwin->rtext / b;
	nearhor /= 2;
	horiz = (nearhor >= nearver) ? YES : NO;
    }

    oldwin = curwin;

    if (curwksp != (S_wksp *) NULL)
	ocurfile = curwksp->wfile;
    else
	ocurfile = NULLFILE;
    savewksp (curwksp);

    if ((newwin = setupwindow ((S_window *) NULL,
				horiz ? oldwin->lmarg
				      : oldwin->lmarg + cursorcol + 1,
				horiz ? oldwin->tmarg + cursorline + 1
				      : oldwin->tmarg,
				oldwin->rmarg,
				oldwin->bmarg, YES)
       ) == (S_window *) NULL)
	return NOMEMERR;

    winlist[nwinlist] = newwin;

    if (horiz) Block {
	/* newwin is below oldwin on the screen  */
	Reg1 Slines i;

	oldwin->bmarg = oldwin->tmarg + cursorline + 1;
	oldwin->bedit = oldwin->btext = cursorline - 1;
	if ((i = (newwin->btext + 1) * sizeof oldwin->firstcol[0]) > 0) {
	    move (&oldwin->firstcol[cursorline + 1],
		   newwin->firstcol, (Uint) i);
	    move (&oldwin->lastcol[cursorline + 1],
		   newwin->lastcol, (Uint) i);
	}
    }
    else Block {
	/* newwin is to the right of oldwin on the screen  */
	Reg1 Slines i;

	oldwin->rmarg = oldwin->lmarg + cursorcol + 1;
	oldwin->rtext = oldwin->redit = cursorcol - 1;
	for (i = newwin->btext; i >= 0; i--) {
	    if (oldwin->lastcol[i] > oldwin->rtext + 1) {
		newwin->firstcol[i] = 0;
		newwin->lastcol[i] = oldwin->lastcol[i] - cursorcol - 1;
		oldwin->lastcol[i] = oldwin->rtext + 1;
		oldwin->rmchars[i] = MRMCH;
	    }
	}
    }

    Block {
	Small winnum;

	for (winnum = 0; winlist[winnum] != curwin; winnum++)
	    continue;
	newwin->prevwin = winnum;
    }
    nwinlist++;

    drawborders (oldwin, WIN_INACTIVE | WIN_DRAWSIDES);

    infotrack (NO);
    inforange (NO);

    switchwindow (newwin);

    /* Дальше создание нового wksp без связи со списком */
    curwksp = (S_wksp *) NULL;

    if (file != (char *) NULL) {
	if (editfile (file, (Ncols) -1, (Nlines) -1, 1, NO, NO) <= 0)
	    (void) eddeffile (NO);
    }
    else if (ocurfile != NULLFILE)
	(void) editfile (names[ocurfile],
		  (Ncols) (horiz || oldwin->wksp == (S_wksp *) NULL
			   ? -1
			   : oldwin->wksp->wcol + oldwin->rtext + 2),
		  (Nlines) (horiz && oldwin->wksp != (S_wksp *) NULL
			   ? oldwin->wksp->wlin + oldwin->btext + 2
			   : -1),
		  0, NO, NO);  /* puflg is YES to initalize border chars */
    else
	(void) eddeffile (NO);

    limitcursor ();

    /* Вставка нового wksp в нужное место списка */
    extractwlist (curwksp);
    insertwlist (curwksp, oldwin->wksp);

    putupwin ();
    drawborders (curwin, WIN_ACTIVE);
    poscursor (0,0);
    curfnum = getnumbywk (curwksp);

    return CROK;
}

/*
    Initialize the window using 'cl', 'cr', 'lt', 'lb' as the left, right,
    top, and bottom.
    If 'editflg' == YES then 'win' is an editing window -- i.e. borders, etc.
    If 'win' == 0, then alloc a new window.
    Return 'win' if no alloc failure, else return 0.
*/
S_window *
setupwindow (win, cl, lt, cr, lb, editflg)
Reg3 S_window *win;
Scols   cl;
Scols   cr;
Slines  lt;
Slines  lb;
Flag    editflg;
{
    if (   win == (S_window *) NULL
	&& (win = (S_window *) okalloc (SWINDOW)) == (S_window *) NULL
       )
	return (S_window *) NULL;

    win->lmarg = cl;
    win->tmarg = lt;
    win->rmarg = cr;
    win->bmarg = lb;
    if (editflg) {
	win->ltext = cl + 1;
	win->ttext = lt + 1;
	win->rtext = cr - cl - 2;
	win->btext = lb - lt - 2;
    }
    else {
	win->ltext = cl;
	win->ttext = lt;
	win->rtext = cr - cl;
	win->btext = lb - lt;
    }
    win->ledit = 0;
    win->tedit = 0;
    win->redit = win->rtext;
    win->bedit = win->btext;

    Block {
	Reg2 Slines size;

	win->wksp = (S_wksp *) NULL;
	if (!editflg)
	    return win;
	size = term.tt_height - NINFOLINES - NENTERLINES - NHORIZBORDERS;
	if ((win->firstcol = (AScols *) okalloc (2 * size * (sizeof *win->firstcol))) != (AScols *) NULL) {
	    win->lastcol = &win->firstcol[size];
	    if ((win->lmchars = okalloc (2 * size * (sizeof *win->lmchars))) != (char *) NULL) Block {
		Reg1 int i;

		win->rmchars = &win->lmchars[size];
		for (i = Z; i < size; i++) {
		    win->firstcol[i] = win->rtext + 1;
		    win->lastcol[i] = 0;
		}
		return win;
	    }
	    sfree ((char *) win->firstcol);
	}
    }
    return (S_window *) NULL;
}

/*    Eliminates the last made window by expanding its ancestor */
void
removewindow ()
{
    Scols   stcol;              /* start col for putup  */
    Slines  stlin;              /* start lin for putup  */
    Small   ppnum;              /* prev window number   */
    Reg1 S_window *thewin;  /* window to be removed */
    Reg2 S_window *pwin;    /* previous window      */

    if (nwinlist == 1) {
	mesg (ERRALL + 1, ediag("Can't remove remaining window.",
				"Нельзя уничтожать последнее окно."));
	return;
    }
    thewin = winlist[--nwinlist];
    ppnum = thewin->prevwin;
    pwin = winlist[ppnum];
    savewksp (thewin->wksp);
    curwksp = thewin->wksp;

    if (pwin->bmarg != thewin->bmarg) Block {
	/* thewin is below pwin on the screen  */
	Slines j;
	Reg3 Slines tmp;

	pwin->firstcol[j = pwin->btext + 1] = 0;
	pwin->lastcol[j++] = pwin->rtext + 1;
	if ((tmp = (thewin->btext + 1) * sizeof *thewin->firstcol) > 0) {
	    move (&thewin->firstcol[0],
		  &pwin->firstcol[j], (Uint) tmp);
	    move (&thewin->lastcol[0],
		  &pwin->lastcol[j], (Uint) tmp);
	}
	stcol = 0;
	stlin = pwin->btext + 1;
	pwin->bmarg = thewin->bmarg;
	pwin->bedit = pwin->btext = pwin->bmarg - pwin->tmarg - 2;
    }
    else Block {
	/* thewin is to the right of pwin on the screen  */
	Reg4 Slines tmp;

	for (tmp = Z; tmp <= pwin->btext; tmp++) {
	    pwin->lastcol[tmp] = thewin->lastcol[tmp] +
		thewin->lmarg - pwin->lmarg;
	    if (pwin->firstcol[tmp] > pwin->rtext)
		pwin->firstcol[tmp] = pwin->rtext;
	}
	stcol = pwin->rtext + 1;
	stlin = 0;
	pwin->rmarg = thewin->rmarg;
	pwin->rtext = pwin->rmarg - pwin->lmarg - 2;
	pwin->redit = pwin->rtext;
    }

    chgwindow (ppnum);
    putup (stlin, curwin->btext, stcol, MAXWIDTH);
    poscursor (pwin->wksp->ccol, pwin->wksp->clin);

    if (!wkused (thewin->wksp))
	freewk (thewin->wksp);
    sfree ((char *) thewin->firstcol);
    sfree (thewin->lmchars);
    (void) la_close (&thewin->wksp->las);
    sfree ((char *) thewin);
}

/*
    Moves current window to another window.
    if 'winnum' < 0 means go to next higher window in winlist[].
*/
void
chgwindow (winnum)
Small winnum;
{
    Reg1 S_window *newwin;
    Reg2 S_window *oldwin;

    oldwin = curwin;
    if (winnum < 0) {
	winnum = 0;
	while (winnum < nwinlist && oldwin != winlist[winnum++])
	    continue;
    }
    curwksp->ccol = cursorcol;
    curwksp->clin = cursorline;
    newwin = winlist[winnum % nwinlist];
    if (newwin == oldwin)     /* ALWAYS rewrite first line */
	entfstline = YES;
    drawborders (oldwin, WIN_INACTIVE | WIN_DRAWSIDES);
    drawborders (newwin, WIN_ACTIVE | WIN_DRAWSIDES);
    infotrack (newwin->winflgs & TRACKSET);
    inforange (newwin->wksp->wkflags & RANGESET);
    switchwindow (newwin);
    limitcursor ();
    poscursor (curwksp->ccol, curwksp->clin);
    if (HelpActive)
	HelpGotoMarg (NO);
    curfnum = getnumbywk (curwksp);
}

static Nlines top;
static Nlines bot;
static Ncols left;
static Ncols right;
static Flag fullmark;
static Flag needmark;

/*
    Draw borders for active or inactive window with or without drawing sides.
*/
void
drawborders (window, how)
S_window *window;
Small how;
{
    S_window *oldwin;

    if (curmark && curwin == window) {
	top = topmark (cursorline) - curwksp->wlin + window->ttext;
	bot = botmark (cursorline) - curwksp->wlin + window->ttext;
	left = leftmark (cursorcol) - curwksp->wcol + window->ltext;
	right = rightmark (cursorcol) - curwksp->wcol + window->ltext;
	fullmark = left == right;
	needmark = YES;
    }
    else
	needmark = NO;

    oldwin = curwin;
    switchwindow (&wholescreen);
    draw1border (window, how, YES);
    draw1border (window, how, NO);
    drawsides (window, how);
    switchwindow (oldwin);
}

void
drawtabs (window, how)
S_window *window;
Small how;
{
    S_window *oldwin;

    if (curmark && curwin == window) {
	top = topmark (cursorline) - curwksp->wlin + window->ttext;
	bot = botmark (cursorline) - curwksp->wlin + window->ttext;
	left = leftmark (cursorcol) - curwksp->wcol + window->ltext;
	right = rightmark (cursorcol) - curwksp->wcol + window->ltext;
	fullmark = left == right;
	needmark = YES;
    }
    else
	needmark = NO;

    oldwin = curwin;
    switchwindow (&wholescreen);
    draw1border (window, how, NO);
    switchwindow (oldwin);
}

static
void
draw1border (window, how, t)
S_window *window;
Small how;
Flag t;
{
    Reg1 Short i;
    Reg2 Short j;

    j = window->rmarg;
    poscursor (i = window->lmarg, t ? window->tmarg : window->bmarg);
    if (!(how & WIN_ACTIVE))
	for (; i <= j; i++)
	    putch (INHMCH, NO);
    else Block {
	Flag Marked;
	Reg3 Short l;
	Echar Ch;

	if (t)
	    Marked = needmark && top < window->ttext;
	else
	    Marked = needmark && bot > window->ttext + window->btext;

	Ch = (t ? TLCMCH : BLCMCH);
	if (Marked && (fullmark || i >= left && i < right))
	    Ch ^= IA_MR;
	putch (Ch, NO);

	Ch = (t ? TMCH : BMCH);
	d_align ();
	for (i++; i < j; i++)
	    d_put (Ch);

	if (!t && visualtabs) Block {
	    Ncols strtcol;
	    Ncols endcol;

	    if (window->wksp != (S_wksp *) NULL)
		strtcol = window->wksp->wcol;
	    else
		strtcol = 0;
	    endcol = strtcol + window->rtext;
	    for (l = 0; l < ntabs; l++)
		if (tabs[l] >= strtcol) {
		    if (tabs[l] > endcol)
			break;
		    _putscbuf[tabs[l] - strtcol] = BTMCH;
		}
	}

	if (Marked && !fullmark) Block {
	    Short lim;

	    lim = min (right - 1, i - 1);
	    for (l = left - 1; l < lim; l++)
		_putscbuf[l] ^= IA_MR;
	}

	Ch = (t ? TRCMCH : BRCMCH);
	if (Marked && (fullmark || i >= left && i < right))
	    Ch ^= IA_MR;
	putch (Ch, NO);
    }
}

/*    Draw the side borders of window. */
static
void
drawsides (window, how)
S_window *window;
Small how;
{
    Echar Ch;

    if (!(how & WIN_DRAWSIDES))
	return;

    if (window->rmarg - window->lmarg < term.tt_width - 1) {
	draw1side (window, window->lmarg, how);
	draw1side (window, window->rmarg, how);
    }
    else {
	Reg3 Slines line;
	Reg4 Slines bottom;

	bottom = window->bmarg - 1;
	line = window->tmarg + 1;
	if (how & WIN_ACTIVE) Block {
	    Reg1 char *lmcp;
	    Reg2 char *rmcp;
	    Flag Marked;

	    lmcp = window->lmchars;
	    rmcp = window->rmchars;
	    for (; line <= bottom; line++) {
		Marked = (needmark && line >= top && line <= bot);
		poscursor (window->lmarg, line);
		Ch = U(*lmcp++);
		if (   Marked
		    && (fullmark || left < window->ltext))
		    Ch ^= IA_MR;
		putch (Ch, NO);
		poscursor (window->rmarg, line);
		Ch = U(*rmcp++);
		if (   Marked
		    && (fullmark || right > window->ltext + window->rtext + 1))
		    Ch ^= IA_MR;
		putch (Ch, NO);
	    }
	}
	else {
	    for (; line <= bottom; line++) {
		poscursor (window->lmarg, line);
		putch (INVMCH, NO);
		poscursor (window->rmarg, line);
		putch (INVMCH, NO);
	    }
	}
    }
}

/*    Draw each side border of window. */
static
void
draw1side (window, border, how)
S_window *window;
Reg1 Scols border;
Small how;
{
    Reg3 Slines line;
    Reg4 Slines bottom;
    Flag lmargflg;
    Echar Ch;

    bottom = window->bmarg - 1;
    line = window->tmarg + 1;
    lmargflg = border == window->lmarg;

    if (how & WIN_ACTIVE) Block {
	Reg2 char *bchrp;

	bchrp = lmargflg ? window->lmchars : window->rmchars;
	for (; line <= bottom; line++) {
	    poscursor (border, line);
	    Ch = U(*bchrp++);
	    if (needmark && line >= top && line <= bot &&
		(fullmark || lmargflg && left < window->ltext ||
		 !lmargflg && right > window->ltext + window->rtext + 1))
		Ch ^= IA_MR;
	    putch (Ch, NO);
	}
    }
    else {
	for (; line <= bottom; line++) {
	    poscursor (border, line);
	    putch (INVMCH, NO);
	}
    }
}

/*
    Adjust all the 'cur' globals like curfile, etc.
    Changes cursorline, cursorcol to be relative to new upper lefthand
    corner.
    You must do a poscursor after switchwindow.
*/
void
switchwindow (win)
Reg1 S_window *win;
{
    Reg2 S_window *cwin;

    cwin = curwin;
    cursorcol  += cwin->ltext - win->ltext;
    cursorline += cwin->ttext - win->ttext;
    if ((curwksp = (curwin = win)->wksp) != (S_wksp *) NULL) {
	curfile = curwksp->wfile;
	curlas = &curwksp->las;
    }
    if (curwin == (S_window *) NULL/* || curwksp == (S_wksp *) NULL*/)
	fatal (FATALBUG, "switchwindow %x %x", curwin, curwksp);
    defplline = defmiline = win->btext / 4 + 1;
    deflwin = defrwin = win->rtext / 5 + 1;
}

/*    Update the TRACK display on the info line */
void
infotrack (onoff)
Reg1 Flag onoff;
{
    static Flag wason;

    if ((onoff = onoff ? YES : NO) ^ wason) {
	d_put (VCCMD);
	info (inf_track, 3, (wason = onoff) ? "TRK" : "", NO);
    }
}
