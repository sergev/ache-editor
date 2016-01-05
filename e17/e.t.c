#include "e.h"
#include "e.mk.h"
#include "e.cc.h"
#include "e.m.h"
#include "e.tt.h"
#include "e.wi.h"
#ifdef SIGNALS
#include SIG_INCL
#endif

extern Flag NoAttrs;
extern int HelpItems;
extern S_wksp *msowksp;

Flag entfstline = NO;           /*  write out entire first line */
Flag needputup; /* because the last one was aborted */
Flag atoldpos;
Slines  newcurline = -1;
Scols   newcurcol  = -1;
Flag    freshputup;             /* ignore previous stuff on these lines */
Flag    nodintrup = NO;         /* disallow display interruption */
Scols putupdelta;               /* nchars to insert (delete) in putup() */

extern void putupwin ();
extern void putup ();
extern void multchar ();
extern void gotomvwin ();
extern void savecurs ();
extern void restcurs ();
extern void poscursor ();
extern void movecursor ();
extern void updatemark ();
void putch ();
Nlines readjtop ();
extern void param ();
extern void exchgcmds ();
extern void getarg ();
extern void limitcursor ();
extern void info ();
extern void mesg ();
extern void credisplay ();
extern void redisplay ();
extern void screenexit ();
extern void tglinsmode ();
static Flag vinsdel ();

/*    Do a putup() of the whole current window. */
void
putupwin ()
{
    savecurs ();
    putup ((Slines) 0, curwin->btext, (Scols) 0, MAXWIDTH);
    restcurs ();
}

/*
    Puts up (displays) lines from 'ls' to 'lf'.
    'ls' and 'lf' are line numbers relative to the top of the current window,
    where 0 would be the first line of the window.
    If 'ls' is negative,
    only puts up line 'lf' and takes it from cline instead of disk.
    Start at column 'strtcol' (rel. to left column of window) on each line.
    If 'ncols' != MAXWIDTH then it is assumed that the line has not gotten
    shorter or longer since last putup, and you know what you are doing,
    and you only want to putup from 'strtcol' to 'strtcol' + 'ncols'.
    Caller must do a poscursor after calling putup.
    If the caller doesn't want the display of the borders to change, he
    clears the global "chgborders" before calling putup (..); If he wants
    dot borders for this putup, he sets chgborders to 2.
    If the caller sets the global `freshputup' flag,
    it means that the line on the screen is blank, and we will
    not need to put out any leading or trailing blanks, and we will have
    to redraw the borders.
*/
void
putup (ls, lf, strtcol, ncols)
Slines  ls;
Slines  lf;
Scols   strtcol;
Scols   ncols;
{
    Reg3 Nlines ln;
    Reg6 Ncols  fc;
    Reg5 Ncols  lcol;   /* = curwksp->wcol */
    Reg8 Scols  rlimit; /* = curwin->rtext + 1 */
    Reg7 Scols  lstcol;
    Reg4 Ncols  col;
    Short tmark;
    Short bmark;
    Short lmark;
    Short rmark;
    Flag  fullmark;
    int     lmc;
    int     rmc;
    Flag    usecline;   /* ls was < 0, so use cline */
    Flag    offendflg;  /* off the end of the file */
    Flag    needmark;   /* current line marked */
    Echar   Ch;

    if (strtcol > (lcol = curwksp->wcol) + (rlimit = curwin->rtext + 1))
	goto ret;
    /* if the caller knows that after the putup he will move the cursor
    /* to someplace other than where it is, then he will set newcurline
    /* and/or newcurcol to the new position
    /* newcurline is always set to -1 on return from putup.
    /**/
    if (newcurline == -1)
	newcurline = cursorline;
    if (newcurcol == -1)
	newcurcol = cursorcol;
    atoldpos = NO;
    if (!nodintrup && dintrup ()) {
	needputup = YES;
	goto ret;
    }
    else
	needputup = NO;

    if (lf > curwin->btext)
	lf = curwin->btext;
    if (usecline = ls < 0)
	ls = lf;
    lmc = lcol == 0 ? LMCH : MLMCH;

    if (curmark) {
	tmark = topmark (newcurline) - curwksp->wlin;
	bmark = botmark (newcurline) - curwksp->wlin;
	lmark = leftmark (newcurcol) - lcol;
	rmark = rightmark (newcurcol) - lcol;
	fullmark = lmark == rmark;
    }

    for (ln = ls;  ln <= lf; ln++) {
	rmc = RMCH;
	offendflg = NO;

	if (!usecline) {        /* get the line into cline */
	    if (   ln != lf
		&& !entfstline
		&& ( replaying || (++intrupnum > DPLYPOL) )
		&& !nodintrup
		&& dintrup ()
	       ) {
		needputup = YES;
		break;
	    }
	    getline (curwksp->wlin + ln);
	    if (xcline) {
		offendflg = YES;
		lmc = ELMCH;
	    }
	    if (ln >= newcurline)
		atoldpos = YES;
	}

	needmark = (curmark && ln >= tmark && ln <= bmark);

	/* left border */
	if (freshputup) {
	    poscursor (-1, ln);
	    Ch = U(chgborders == 1 ? lmc : INVMCH);
	    if (needmark && (fullmark || lmark < 0))
		Ch ^= IA_MR;
	    putch (Ch , NO);
	    curwin->lmchars[ln] = lmc;
	} else Block {
	    Reg1 Slines lt;
	    Slines lb;

	    if (ln == ls && lmc != ELMCH)
		lb = 0;
	    else
		lb = ln;
	    for (lt = ln; lt >= lb; lt--) {
		if ((!curmark || lt < ls) &&
		    U(curwin->lmchars[lt]) == lmc)
		    break;
		if (chgborders) {
		    poscursor (-1, lt);
		    Ch = U(chgborders == 1 ? lmc : INVMCH);
		    if (curmark && lt >= tmark && lt <= bmark &&
			(fullmark || lmark < 0))
			Ch ^= IA_MR;
		    putch (Ch, NO);
		}
		if (lt == newcurline) loopflags.clear = YES;
		curwin->lmchars[lt] = lmc;
	    }
	}

	/*  fc: first non-blank column in cline */
	fc = 0;
	if (offendflg && !needmark || (!needmark || !fullmark) &&
	    (fc = fnbcol (cline, lcol, min (ncline, lcol + rlimit))) < lcol
	   )
	    fc = rlimit;
	else
	    fc -= lcol;

	if (needmark && !fullmark)
	    fc = min (fc, lmark);

	if (fc < 0)
	    fc = 0;

	/* leading blanks ? */
	if (entfstline) {
	    entfstline = NO;
	    col = 0;
	} else {
	    if (freshputup)
		col = fc;
	    else
		col = min (fc, curwin->firstcol[ln]);
	    if (col < strtcol)
		col = strtcol;
	}
	curwin->firstcol[ln] = fc;
	poscursor ((Scols) col, ln);
/*      if (stdout->_cnt < 100) /* smooth any outputting breaks  */
/*          d_put(0);
/**/
	Block {
	    Reg1 Scols tmp;

	    if ((tmp = fc - col) > 0) {
		multchar (' ', tmp);
		col = fc;
	    }
	}

	/* rightmost col */
	if (offendflg && !needmark)
	    lstcol = 0;
	else Block {
	    Reg1 Ncols tmp;

	    tmp = ncline - 1 - lcol;
	    if (needmark && !fullmark)
		tmp = max (tmp, rmark);
	    if (tmp < 0)
		tmp = 0;
	    else if (tmp > rlimit) {
		tmp = rlimit;
		rmc = MRMCH;
	    }
	    lstcol = tmp;
	}
	if (lstcol < col)
	    lstcol = col;
	Block {
	    Reg3 Scols ricol;   /* rightmost col of text to put up */
	    Reg4 Scols tmp;

	    ricol = min (lstcol, min (rlimit, strtcol + ncols));
	    ricol = min (ricol, ncline - 1 - lcol);
	    /* put out the text */
	    if (ricol - col > 0) {
		/*  if (putupdelta && col >= strtcol) {
			d_hmove (putupdelta, (Short) (ricol - col),
				 &cline[lcol + col]);
		} else */ Block {
		    Reg1 Short i;
		    Short lim;

		    d_align ();
		    move ((char *) &cline[col + lcol],
			  (char *) _putscbuf,
			  (Uint) (_putp = ricol - col) * sizeof (Echar));
		    if (needmark && !fullmark) {
			lim = min (_putp, rmark - col);
			for (i = max (0, lmark - col); i < lim; i++)
			    _putscbuf[i] ^= IA_MR;
		    }
		}
		cursorcol += (ricol - col);
	    }

	    /* trailing blanks ? */
	    lstcol = min (lstcol, strtcol + ncols);
	    if (!freshputup && (!needmark || fullmark) &&
		(tmp = lstcol - cursorcol) > 0)
		    multchar (' ', tmp);
	    else if (needmark && !fullmark) {
		Short start;
		Short end;

		start = lmark;
		if (start < cursorcol)
		    start = cursorcol;
		else if (start > lstcol)
		    start = lstcol;
		end = rmark;
		if (end < cursorcol)
		    end = cursorcol;
		else if (end > lstcol)
		    end = lstcol;
		if (!freshputup || end - start > 0)
		    multchar (' ', start - cursorcol);
		multchar (' ' | IA_MR, end - start);
		if (!freshputup)
		    multchar (' ', lstcol - end);
	    }
	}
	if (freshputup || curwin->lastcol[ln] <= strtcol + ncols) {
	    Short tmp;

	    if (!freshputup && (tmp = curwin->lastcol[ln] - lstcol) > 0)
		multchar (' ', tmp);
	    curwin->lastcol[ln] = (lstcol == fc) ? 0 : lstcol;
	}

	/* what to do about the right border */
	if (freshputup) {
	    poscursor (curwin->rmarg - curwin->ltext, ln);
	    Ch = U(chgborders == 1 ? rmc : INVMCH);
	    if (needmark && (fullmark || rmark >= curwin->rtext))
		Ch ^= IA_MR;
	    putch (Ch, NO);
	    curwin->rmchars[ln] = rmc;
	} else if (curmark || U(curwin->rmchars[ln]) != rmc) {
	    if (chgborders) {
		poscursor (curwin->rmarg - curwin->ltext, ln);
		Ch = U(chgborders == 1 ? rmc : INVMCH);
		if (needmark && (fullmark || rmark > curwin->rtext + 1))
		    Ch ^= IA_MR;
		putch (Ch, NO);
	    }
	    curwin->rmchars[ln] = rmc;
	    if (ln == newcurline) loopflags.clear = YES;
	}
    }
ret:
    putupdelta = 0;
    newcurline = -1;
    newcurcol = -1;
}

/*
    Look for a non-blank character in line between line[left] and line[right]
    If right < left, or a newline or non-blank is encountered,
	return left - 1
    else
	return index into array line of the non-blank char.
*/
Short
fnbcol (line, left, right)
Reg4 Echar *line;
Reg5 Ncols left;
Ncols right;
{
    Reg1 Echar *cp;
    Reg2 Echar *cpr;
    Reg3 Echar *cpl;

    if (right < left)
	return left - 1;

    cpl = &line[left];
    cpr = &line[right];
    for (cp = line; ; cp++) {
	if (   *cp == '\n'
	    || cp >= cpr
	   )
	    return left - 1;
	if (   *cp != ' '
	    && cp >= cpl
	   )
	    return cp - line;
    }
    /* NOTREACHED */
}

/*
    Write 'num' 'chr's to d_write.
    Advance cursorcol by 'num' columns.
*/
void
multchar (chr, num)
Reg4 Echar chr;
Reg1 Scols num;
{
    Reg2 Small cnt;
    Echar buf[3];

    if (num <= 0)
	return;
    cursorcol += num;
    while (num > 0) {
	cnt = 0;
	if (num > 3) {
	    buf[cnt++] = VCCARG;
	    buf[cnt++] = num;
	    num = 0;
	}
	else {
	    if (num > 2) {
		buf[cnt++] = chr;
		num--;
	    }
	    if (num > 1) {
		buf[cnt++] = chr;
		num--;
	    }
	    num--;
	}
	buf[cnt++] = chr;
	d_align ();
	d_write (buf, cnt);
    }
}

/*
    All three of the following functions: gotomvwin(), vertmvwin(), and
    horzmvwin() must not return until they have called movewin, because
    movewin looks to see if a putup needs to be done after one was aborted.
*/

/*
    Move the window so that line 'number' is the distance of one +LINE
    down from the top of the window.
*/
void
gotomvwin (line, column)
Reg1 Nlines line;
Reg2 Ncols column;
{
    Nlines ll;
    Ncols cl;
    Small  defpl;
    Reg3 Nlines winlin;
    Reg4 Ncols wincol;

    line = max (0, line);
    winlin = curwksp->wlin;
    if (line >= winlin &&
	line <= winlin + curwin->btext)
	ll = line - winlin;
    else {
	if (curwin->btext > 1)
	    defpl = defplline;
	else
	    defpl = Z;
	ll = min (line, defpl);
	winlin = line - defpl;
    }

    column = max (0, column);
    wincol = curwksp->wcol;
    if (column >= wincol &&
	column <= wincol + curwin->rtext)
	cl = column - wincol;
    else {
	if (curwin->rtext > 1)
	    defpl = defrwin;
	else
	    defpl = Z;
	cl = min (column, defpl);
	wincol = column - defpl;
    }

    movewin (winlin, wincol, ll, cl, YES);
}

/*    Moves window nl lines down.  If nl < 0 then move up. */
Small
vertmvwin (nl)
Reg2 Nlines nl;
{
    Reg3 Nlines winlin;
    Reg1 Nlines cl;

    winlin = curwksp->wlin;
    if (winlin == 0 && nl <= 0)
	nl = 0;
    if ((winlin += nl) < 0) {
	nl -= winlin;
	winlin = Z;
    }
    cl = cursorline - nl;

    if (abs (nl) > curwin->btext)
	cl = cursorline;
    else if (cl < 0)
	cl = Z;
    else if (cl > curwin->btext)
	cl = curwin->btext;

    return movewin (winlin, curwksp->wcol, cl, cursorcol, YES);
}

/*    Moves window nl lines down.  If nl < 0 then move up. */
Small
rollmvwin (nl)
Reg2 Nlines nl;
{
    Reg3 Nlines winlin;

    winlin = curwksp->wlin;
    if (winlin == 0 && nl <= 0)
	nl = 0;
    if ((winlin += nl) < 0) {
	nl -= winlin;
	winlin = Z;
    }

    return movewin (winlin, curwksp->wcol, cursorline, cursorcol, YES);
}

/*    Moves window 'nc' columns right.  If nc < 0 then move left. */
Small
horzmvwin (nc)
Reg2 Ncols nc;
{
    Reg1 Ncols  cc;

    cc = cursorcol;
    if ((curwksp->wcol + nc) < 0)
	nc = -curwksp->wcol;
    cc -= nc;
    cc = max (cc, curwin->ledit);
    cc = min (cc, curwin->redit);
    return movewin (curwksp->wlin, curwksp->wcol + nc, cursorline, (Scols) cc,
		    YES);
}

/*
    Move the window so that winlin and wincol are the upper left corner,
    and curslin and curscol are the cursor position.
    If TRACKSET in curwin, move prev wksp too, but don't disturb its
    cursor postion.
*/
Small
movewin (winlin, wincol, curslin, curscol, puflg)
Reg4 Nlines  winlin;
     Ncols   wincol;
Reg5 Slines  curslin;
Reg6 Scols   curscol;
     Flag    puflg;
{
    Reg3 Small newdisplay;
    Reg4 Nlines vdist;      /* vertical distance to move */
    Reg4 Ncols  hdist;      /* horizontal distance to move */
    Reg2 S_wksp *cwksp = curwksp;

    if (winlin < 0)
	winlin = 0;
    if (wincol < 0)
	wincol = 0;
    if (HelpActive && winlin <= HelpItems) {
	winlin = HelpItems + 1;
	curslin = 0;
    }
#ifdef LA_LONGFILES
    if (winlin + defplline < 0)
#else
    if ((long) winlin + defplline > LA_MAXNLINES)
#endif
	winlin = LA_MAXNLINES - defplline;
    curslin = max (0, curslin);
    curslin = min (curslin, curwin->btext);
    curscol = max (0, curscol);
    curscol = min (curscol, curwin->rtext);
    newdisplay = Z;

    if (vdist = winlin - cwksp->wlin) {
	cwksp->wlin = winlin;
	newdisplay |= WLINMOVED;
    }
    if (hdist = wincol - cwksp->wcol) {
	cwksp->wcol = wincol;
	newdisplay |= WCOLMOVED;
    }

    if (newdisplay && (curwin->winflgs & TRACKSET)) Block {
	Reg1 S_wksp *awksp;
	Small win;
	S_wksp *tw;
	S_window *oldwin = curwin;
	S_window *wn;
	Nlines wlin;
	Ncols wcol;
	Flag otherwin = NO;
	Flag savneedp = needputup;

	needputup = NO;
	for (win = Z; win < nwinlist; win++)
	    if ((tw = winlist[win]->wksp) != (S_wksp *) NULL)
		tw->wkflags |= WSDISP;

	for (awksp = first_wksp; awksp != (S_wksp *) NULL; awksp = awksp->next_wksp) {
	    wlin = awksp->wlin;
	    wcol = awksp->wcol;
	    if (vdist) {
		if ((wlin += vdist) < 0)
		    wlin = 0;
#ifdef LA_LONGFILES
		else if (wlin + defplline < 0)
#else
		else if ((long) wlin + defplline > LA_MAXNLINES)
#endif
		    wlin = LA_MAXNLINES - defplline;
	    }
	    if (hdist) {
		if ((wcol += hdist) < 0)
		    wcol = 0;
	    }
	    if (awksp != cwksp) {
		awksp->clin = curslin;
		awksp->ccol = curscol;
	    }
	    if (awksp->wkflags & WSDISP) {
		awksp->wkflags &= ~WSDISP;
		for (win = Z; win < nwinlist; win++)
		    if (   (wn = winlist[win]) != oldwin
			&& wn->wksp == awksp
		       ) Block {
			Flag savtrack = (wn->winflgs & TRACKSET) != 0;

			otherwin = YES;
			wn->winflgs &= ~TRACKSET;
			switchwindow (wn);
			chgborders = 2;
			(void) movewin (wlin, wcol, curslin, curscol, puflg);
			if (savtrack)
			    wn->winflgs |= TRACKSET;
		    }
	    }
	    else {
		awksp->wlin = wlin;
		awksp->wcol = wcol;
	    }
	}
	if (otherwin) {
	    drawborders (oldwin, WIN_ACTIVE | WIN_DRAWSIDES);
	    switchwindow (oldwin);
	    chgborders = 1;
	}
	cwksp->wkflags &= ~WSDISP;
	needputup = savneedp;
    }

    if (newdisplay || needputup) {
	newcurline = curslin;
	newcurcol  = curscol;
	if (puflg) {
	    if (needputup)
		putupwin ();
	    else {
		if (   hdist == 0
		    && abs(vdist) <= curwin->btext
		    && curwksp->wlin < la_lsize (curlas)
		    && curwksp->wlin - vdist < la_lsize (curlas)
		   )
		    (void) vinsdel (0, -vdist, YES);
		else
		    putupwin ();
	    }
	    if (!needputup) {
		if (curmark && !NoAttrs && (newdisplay & WLINMOVED))
		    drawborders (curwin, WIN_ACTIVE | WIN_DRAWSIDES);
		else if (visualtabs && (newdisplay & WCOLMOVED))
		    drawtabs (curwin, WIN_ACTIVE);
	    }
	}
    }
    if (cursorline != curslin)
	newdisplay |= LINMOVED;
    if (cursorcol != curscol)
	newdisplay |= COLMOVED;
    poscursor (curscol, curslin);

    return newdisplay;
}

static
struct svcs {
    struct svcs *sv_lastsv;
    AScols  sv_curscol;
    ASlines sv_cursline;
} *sv_curs;

/*
    Save current cursor position.
    See restcurs().
*/
void
savecurs ()
{
    Reg1 struct svcs *lastsv;

    lastsv = sv_curs;
    sv_curs = (struct svcs *) salloc (sizeof *sv_curs, YES);
    sv_curs->sv_lastsv = lastsv;
    sv_curs->sv_curscol = cursorcol;
    sv_curs->sv_cursline = cursorline;
}

void
freecurs ()
{
    Reg1 struct svcs *lastsv;

    lastsv = sv_curs->sv_lastsv;
    sfree ((char *) sv_curs);
    sv_curs = lastsv;
}

/*
    Restore current cursor position.
    See savecurs().
*/
void
restcurs ()
{
    poscursor (sv_curs->sv_curscol, sv_curs->sv_cursline);
    freecurs ();
}

#ifdef NOTYET
static
struct svps {
    struct svps *pv_lastpv;
    ANcols  pv_wcol;
    ANlines pv_wline;
} *pv_pos;

void
savepos ()
{
    Reg1 struct svps *lastpv;

    savecurs ();
    lastpv = pv_pos;
    pv_pos = (struct svps *) salloc (sizeof *pv_pos, YES);
    pv_pos->pv_lastpv = lastpv;
    pv_pos->pv_wcol = curwksp->wcol;
    pv_pos->pv_wlin = curwksp->wlin;
}

void
freepos ()
{
    Reg1 struct svps *lastpv;

    freecurs ();
    lastpv = pv_pos->pv_lastpv;
    sfree ((char *) pv_pos);
    pv_pos = lastpv;
}

void
restpos ()
{
    (void) movewin (pv_pos->pv_wlin, pv_pos->pv_wcol,
		    sv_curs->sv_cursline,
		    sv_curs->sv_curscol, YES);
    freepos ();
}
#endif NOTYET

/*
    Position cursor.
    col is relative to curwin->ltext
    lin is relative to curwin->ttext
*/
void
poscursor (col, lin)
Reg1 Scols  col;
Reg2 Slines lin;
{
    if (cursorline == lin && cursorcol == col)
	return;
    d_align ();
    d_put (VCCAAD);
    d_put (curwin->ltext + col);
    d_put (curwin->ttext + lin);
    cursorcol = col;
    cursorline = lin;
}

/*
    Move cursor within boundaries of current window.
    Type of motion is specified by 'func'.
*/
void
movecursor (func, cnt)
Small func;
Reg4 Nlines cnt;
{
    Reg3 Slines lin;
    Reg2 Ncols col;
    Reg5 Nlines ldif;
    Reg6 Ncols  cdif;
    Flag enter;
    Short i;
    S_wksp *cwksp;

    if (enter = (curwin == &enterwin))
	cwksp = msowksp;
    else
	cwksp = curwksp;
    lin = cursorline;
    col = cursorcol;
    ldif = cdif = 0;
    switch (func) {
    case 0:                     /* noop                             */
	break;
    case HO:                    /* upper left-hand corner of screen */
	col = Z;
	lin = Z;
	break;
    case UP:                    /* up one line                      */
	if (!enter && cnt <= 1 && lin <= curwin->tedit + defmiline / 3 &&
	    cwksp->wlin + curwin->tedit > 0) {
	    ldif = -cnt - ((curwin->tedit + defmiline / 3) - lin);
	    lin -= ldif + cnt;
	}
	else
	    lin -= cnt;
	break;
    case RN:                    /* left column and down one */
    case C1:
	col = - (cwksp->wcol + col);
	if (func != RN)
	    break;
	/* fall into */
    case DN:                    /* down one line */
	if (!enter && cnt <= 1 && lin >= curwin->bedit - defplline / 3 &&
	    cwksp->wlin + curwin->bedit < la_lsize (curlas)) {
	    ldif = cnt + (lin - (curwin->bedit - defplline / 3));
	    lin -= ldif - cnt;
	    }
	else
	    lin += cnt;
	break;
    case RT:                    /* forward one space */
	if (!enter && cnt <= 1 && col >= curwin->redit - defrwin / 3) {
	    cdif = cnt + (col - (curwin->redit - defrwin / 3));
	    col -= cdif - cnt;
	}
	else
	    col += cnt;
	break;
    case LT:                    /* back space (non-destructive) */
	if (!enter && cnt <= 1 && col <= curwin->ledit + deflwin / 3 &&
	    cwksp->wcol + curwin->ledit > 0) {
	    cdif = - cnt - ((curwin->ledit + deflwin / 3) - col);
	    col -= cdif + cnt;
	}
	else
	    col -= cnt;
	break;
    case TB:                    /* tab forward to next stop */
	if (enter)
	    col--;
	for (i = Z, col += cwksp->wcol; i < ntabs; i++)
	    if (tabs[i] > col) {
		/*if (tabs[i] <= cwksp->wcol + curwin->rtext)*/
		    col = tabs[min (ntabs - 1, i + cnt - 1)];
		break;
	    }
	col -= cwksp->wcol;
	if (enter)
	    col++;
	break;
    case BT:                    /* tab back to previous stop        */
	if (enter)
	    col--;
	for (i = ntabs - 1, col += cwksp->wcol; i >= 0; i--)
	    if (tabs[i] < col) {
		/*if (tabs[i] >= cwksp->wcol)*/
		    col = tabs[max (0, i - cnt + 1)];
		break;
	    }
	col -= cwksp->wcol;
	if (enter)
	    col++;
	break;
    }

    if (enter) {
	if (col > curwin->redit)
	    col %= curwin->redit + 1;
	if (col < curwin->ledit + 5) /* CMD: */
	    col = curwin->ledit + 5;
	func = 0;
    }

    if (cdif == 0) {
	if ((cdif = col - curwin->ledit) < 0)
	    col = curwin->ledit;
	else if ((cdif = col - curwin->redit) > 0)
	    col = curwin->redit;
	else
	    cdif = 0;
    }

    if (ldif == 0) {
	if ((ldif = lin - curwin->tedit) < 0)
	    lin = curwin->tedit;
	else if ((ldif = lin - curwin->bedit) > 0)
	    lin = curwin->bedit;
	else
	    ldif = 0;
    }

    if ((ldif || cdif) && func && !enter) {
	if (cdif < 0) {
	    cdif -= deflwin / 3;
	    col += deflwin / 3;
	    if ((i = cwksp->wcol + cdif) < 0) {
		col += i;
		cdif -= i;
	    }
	}
	else if (cdif > 0) {
	    col -= defrwin / 3;
	    cdif += defrwin / 3;
	}
	movewin (cwksp->wlin + ldif,
		 cwksp->wcol + cdif,
		 lin, (Scols) col, YES);
    }
    else
	poscursor ((Scols) col, lin);
}

void
updatemark ()
{
    Reg1 Nlines minl;
    Reg2 Nlines maxl;
    Reg3 Ncols  minc;
    Reg4 Ncols  maxc;
    Nlines top;
    Nlines bot;
    Ncols left;
    Ncols right;
    Flag fullmark;
    Flag needbord;

    if (curmark) {
	left = leftmark (cursorcol);
	right = rightmark (cursorcol);
	top = topmark (cursorline);
	bot = botmark (cursorline);
	if (cmarkleft == -1 && cmarkright == -1 &&
	    cmarktop == -1 && cmarkbot == -1) {
	    needbord = YES;
	    fullmark = NO;
	    cmarkleft = left;
	    cmarkright = right;
	    cmarktop = top;
	    cmarkbot = bot;
	}
	else {
	    needbord = left != cmarkleft || right != cmarkright;
	    fullmark = cmarkleft == cmarkright;
	}
    }
    else {
	needbord = YES;
	fullmark = NO;
	left = cmarkleft;
	right = cmarkright;
	top = cmarktop;
	bot = cmarkbot;
    }
    if (!needbord && top == cmarktop && bot == cmarkbot)
	return;
    fullmark = fullmark || left == right;
    minl = min (cmarktop, top) - curwksp->wlin;
    maxl = max (cmarkbot, bot) - curwksp->wlin;
    minc = min (cmarkleft, left) - curwksp->wcol;
    maxc = max (cmarkright, right) - curwksp->wcol;
    needbord = needbord && (minl < 0 || maxl > curwin->btext);
    if (minl < 0)
	minl = 0;
    if (minc < 0)
	minc = 0;
    if (maxc > curwin->rtext)
	maxc = curwin->rtext;
    newcurline = cursorline;
    newcurcol = cursorcol;
    savecurs ();
    if (needbord || !curmark)
	drawborders (curwin, WIN_ACTIVE | (!curmark ? WIN_DRAWSIDES : 0));
    chgborders = 1;
    putup (minl, maxl, minc, maxc - minc + 1 - fullmark);
    restcurs ();
    cmarkleft = -1;
    cmarkright = -1;
    cmarktop = -1;
    cmarkbot = -1;
}

/*
    Put a character up at current cursor position.
    The character has to be a space, a printing char, a bell or an 0177.
    Can't be negative.
    If 'flg' is non-0, then 'chr' is being put into a display window,
    and its position may be before or after existing printing characters
    on the line.  This must be noted for putup ().
    'flg' is is only YES in 3 places: 2 in printchar and 1 in setbul ().
*/
void
putch (chr, flg)
Echar   chr;
Flag    flg;
{
    if (U(chr) < ' ') {
	d_put (chr);
	return;
    }

    if (flg && chr != ' ') {
	/* Позиция символа */
	if (curwin->firstcol[cursorline] > cursorcol)
	    curwin->firstcol[cursorline] = cursorcol;
	if (curwin->lastcol[cursorline] <= cursorcol)
	    curwin->lastcol[cursorline] = cursorcol + 1;
    }
    /* Следущая позиция */
    /*  Adjust cursorcol, cursorline for edge effects of screen.
	Sets cursorcol, cursorline to correct values if they were
	positioned past right margin.
	If cursor was incremented from bottom right corner of screen do not
	put out a character since screen would scroll, but home cursor.
    /**/
    if (curwin->ltext + ++cursorcol >= term.tt_width) {
	cursorcol = -curwin->ltext;  /* left of screen */
	if (curwin->ttext + ++cursorline >= term.tt_height) {
	    cursorline = -curwin->ttext; /* top of screen */
	    d_put (VCCHOM);
	}
	else
	    d_put (chr);
    }
    else
	d_put (chr);
}

/*
    Limit cursor to be within current window.
    Used after a new window has been made to be sure that the cursor
    in the parent window stays within that window.
*/
void
limitcursor ()
{
    curwksp->ccol = min (curwksp->ccol, curwin->rtext);
    curwksp->clin = min (curwksp->clin, curwin->btext);
}

/*
    Redisplay the current line if it has changed since that last time
    credisplay was called.
    Only called from one place: mainloop ().
*/
void
credisplay (cwkspflg)
Flag cwkspflg;
{
    /* this is a little tricky.  Everywhere else in the editor, fcline
    /* is treated as if it is either 0 or non-0, thus it is given YES
    /* or NO values.  Here, if it is YES, we set it to 2, so that we can
    /* tell if it has been set to YES since the last time we were called.
    /* Only if it is YES do we need to do a redisplay.
    /**/
    if (fcline == YES) {
	redisplay (curfile, curwksp->wlin + cursorline, (Nlines) 1, (Nlines) 0, cwkspflg);
	fcline = 2;
    }
}

/*
    Redisplay is called after a change has been made in file 'fn',
    starting at line 'from',
    with a total change of 'delta' in the length of the file.
    If `delta' is negative, `num' lines must be redisplayed after
    deleting `delta' lines.
    If `delta' is positive, `delta' + `num' lines must be redisplayed.
    If `delta' is 0, dislplay `num' lines.
    We are supposed to:
    1. Redisplay any workspaces which are actually changed by this
	tampering, including curwksp if 'cwkspflg' is non-0
    2. Adjust the current line number of any workspace which may be pointing
	further down in the file than the change, and doesn't want
	to suffer any apparent motion.
*/
void
redisplay (fn, from, num, delta, cwkspflg)
Fn      fn;
Reg5 Nlines  from;
Reg8 Nlines  num;
Reg9 Nlines  delta;
Flag    cwkspflg;
{
    Reg1 S_wksp *tw;
    Reg4 Small  win;
    Reg3 Slines winfirst;   /* from - wksp->wlin */
    Reg6 Slines first;
    Reg2 Slines endwin;     /* height of window -1 */

    for (win = Z; win < nwinlist; win++)
	if (   (tw = winlist[win]->wksp) != (S_wksp *) NULL
	    && tw->wfile == fn
	   ) Block {
	    Nlines wmove;
	    Reg7 S_window *oldwin;

	    tw->wkflags |= WSDISP;
	    wmove = readjtop (tw->wlin, from, num, delta
			      /*,winlist[win]->btext + 1*/);
	    winfirst = from - (tw->wlin += wmove);
	    if (tw == curwksp && !cwkspflg)
		continue;
	    first = winfirst > 0 ? winfirst : 0;
	    endwin = winlist[win]->btext;
	    /* changes below the bottom of the window? */
	    if (first > endwin)
		continue;
	    /* changes above the top of the window? */
	    if (delta == 0 && winfirst + num <= 0)
		continue;
	    if (   delta > 0
		&& winfirst < 0         /* for insert on first line */
		&& winfirst + num <= 0
	       )
		continue;
	    if (delta < 0 && winfirst - delta + num <= 0)
		continue;
	    oldwin = curwin;
	    savecurs ();
	    switchwindow (winlist[win]);
	    if (curwin != oldwin)
		chgborders = 0;
	    if (delta == 0) {
		putup (first, winfirst + num - 1, (Scols) 0, MAXWIDTH);
	    } else if (delta > 0) {
		if (winfirst + delta + num > endwin)
		    putup (first, endwin, (Scols) 0, MAXWIDTH);
		else if (from >= la_lsize (curlas) - delta)
		    putup (first, winfirst + delta + num - 1, (Scols) 0, MAXWIDTH);
		else if (  !vinsdel (first, delta, curwin == oldwin)
			 && num > 0
			)
		    putup (winfirst + delta, winfirst + delta + num - 1,
			   (Scols) 0, MAXWIDTH);
	    } else { /* delta < 0 */
		if (   first - (delta - wmove) > endwin
		    || (   num > 0
			&& winfirst - delta + num > endwin
		       )
		   )
		    putup (first, endwin, (Scols) 0, MAXWIDTH);
		else if (from >= la_lsize (curlas))
		    putup (first, winfirst - delta - 1, (Scols) 0, MAXWIDTH);
		else if (   !vinsdel (first, delta - wmove, curwin == oldwin)
			 && num > 0
			)
		    putup (first, winfirst + num - 1, (Scols) 0, MAXWIDTH);
	    }
	    chgborders = 1;
	    switchwindow (oldwin);
	    restcurs ();
	}

    for (tw = first_wksp; tw != (S_wksp *) NULL; tw = tw->next_wksp)
	if (tw->wfile == fn) {
	    if (tw->wkflags & WSDISP)
		tw->wkflags &= ~WSDISP;
	    else
		tw->wlin += readjtop (tw->wlin, from, num, delta
				  /*,winlist[win]->btext + 1*/);
	}
}

/*
    Called by redisplay () to determine how far to move the top of 'wksp'.
    Returns the distance to move the window.
.
    The logic in redisplay () and the algorithm
    implemented here are closely interdependent.
*/
Nlines
readjtop (wlin, from, num, delta/*, height*/)
Reg1 Nlines wlin;
Reg3 Nlines from;
Reg4 Nlines num;
Reg2 Nlines delta;
{
    if (delta == 0)
	return 0;
    if (delta > 0) {
	if (from >= wlin)
	    return 0;
	if (from + num <= wlin)
	    return delta;
	return 0;
    }
    /* delta < 0 */
	if (from > wlin)
	    return 0;
	if (from - delta + num <= wlin)
	    return delta;
	if (num == 0)
	    return from - wlin;
	return 0;
}

/*
    Insert `delta' lines at line `start' in the window.
    This means delete if `delta' is negative, of course.
    Return YES if we had to do a putup for the whole thing,
    i.e. couldn't use ins/del terminal capability, else NO.
*/
static
Flag
vinsdel (start, delta, mainwin)
Reg1 Slines start;
Reg3 Slines delta;
Flag mainwin;
{
    Reg5 S_wksp *cwksp = curwksp;

    if (needputup || silent) goto doputup;

    if (mainwin) Block {
	extern Flag needrest;
	Flag savneedrest = needrest;

	needrest = NO;
	clrbul ();
	needrest = savneedrest;
	loopflags.clear = YES;
    }

    if (delta > 0) Block {
	Reg2 int num;

	/* insert lines */
	do Block {
	    Reg4 int stplnum;   /* start + num */
	    Reg5 Uint nmove;

	    num = d_vmove (curwin->ttext + start,
			   curwin->lmarg,
			   curwin->btext + 1 - (start + delta),
			   curwin->rmarg + 1 - curwin->lmarg,
			   delta,
			   YES);
	    if (num <= 0) {
 doputup:
		savecurs ();
		putup (start, curwin->btext, (Scols) 0, MAXWIDTH);
		restcurs ();
		return YES;
	    }
	    nmove = curwin->btext + 1 - (stplnum = start + num);
	    (void) move (&curwin->firstcol[start], &curwin->firstcol[stplnum],
		  nmove * sizeof curwin->firstcol[0]);
	    (void) fill (&curwin->firstcol[start],
		  num * sizeof curwin->firstcol[0], 0);
	    (void) move (&curwin->lastcol[start], &curwin->lastcol[stplnum],
		  nmove * sizeof curwin->lastcol[0]);
	    (void) fill (&curwin->lastcol[start],
		  num * sizeof curwin->lastcol[0], 0);
	    (void) move (&curwin->lmchars[start], &curwin->lmchars[stplnum], nmove);
	    (void) move (&curwin->rmchars[start], &curwin->rmchars[stplnum], nmove);
	    savecurs ();
	    freshputup = YES;
	    nodintrup = YES;
	    cwksp->wlin += delta - num;
	    putup (start, curmark && !NoAttrs ?
		   curwin->btext : start + num - 1, (Scols) 0, MAXWIDTH);
	    cwksp->wlin -= delta - num;
	    nodintrup = NO;
	    freshputup = NO;
	    restcurs ();
	    newcurline = cursorline;
	    if ((replaying || (term.tt_vscset || term.tt_delline) &&
		(++intrupnum > DPLYPOL*5)) && dintrup()) needputup = YES;
	} while ((delta -= num) > 0);
    } else Block { /* delta < 0 */
	Reg2 int num;

	delta = -delta;
	/* delete lines */
	do Block {
	    Reg4 int stplnum;   /* start + num */
	    Reg5 Uint nmove;

	    num = d_vmove (curwin->ttext + start + delta,
			   curwin->lmarg,
			   curwin->btext + 1 - start - delta,
			   curwin->rmarg - curwin->lmarg + 1,
			   -delta,
			   YES);
	    if (num <= 0)
		goto doputup;
	    nmove = curwin->btext + 1 - (stplnum = start + num);
	    (void) move (&curwin->firstcol[stplnum], &curwin->firstcol[start],
		  nmove * sizeof curwin->firstcol[0]);
	    (void) fill (&curwin->firstcol[curwin->btext + 1 - num],
		  num * sizeof curwin->firstcol[0], 0);
	    (void) move (&curwin->lastcol[stplnum], &curwin->lastcol[start],
		  nmove * sizeof curwin->lastcol[0]);
	    (void) fill (&curwin->lastcol[curwin->btext + 1 - num],
		  num * sizeof curwin->firstcol[0], 0);
	    (void) move (&curwin->lmchars[stplnum], &curwin->lmchars[start], nmove);
	    (void) move (&curwin->rmchars[stplnum], &curwin->rmchars[start], nmove);
	    savecurs ();
	    freshputup = YES;
	    nodintrup = YES;
	    cwksp->wlin -= delta - num;
	    putup (curmark && !NoAttrs ? start : curwin->btext + 1 - num,
			curwin->btext, (Scols) 0, MAXWIDTH);
	    cwksp->wlin += delta - num;
	    nodintrup = NO;
	    freshputup = NO;
	    restcurs ();
	    newcurline = cursorline;
	    if ((replaying || (term.tt_vscset || term.tt_delline) &&
		(++intrupnum > DPLYPOL*5)) && dintrup()) needputup = YES;
	} while ((delta -= num) > 0);
    }
    return NO;
}
