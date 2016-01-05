/*    search and replace */

#ifdef UNIXV7
#include <ctype.h>
#endif
#include "e.h"
#include "e.tt.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.cm.h"
#include "e.se.h"

Flag rplinteractive, rplshow;
static char *rpls1;
static char *rpls2;
static Ncols rpls1len;
static Ncols rpls2len;

static void doreplace ();
extern void replkey ();
static void rplit ();
extern void dosearch ();
static Nlines rangelimit ();
extern void showrange ();
void aborted ();
extern char *le_comp ();
extern char *index ();

static char *expbuf;
#define EXPSIZE 512

int
regtest (str)
char *str;
{
    Reg1 char *s;
    char *ans;

    for (s = str; *s; s++) {
	if (index ("\\[.^*$", *s) != NULL) {
	    if (!expbuf && !(expbuf = salloc (EXPSIZE, NO))) {
		mesg (ERRALL + 1, ediag ("No memory for regular expression",
					 "Нет памяти для регулярного выражения"));
		return -1;
	    }
	    if ((ans = le_comp (str, expbuf, &expbuf[EXPSIZE])) != NULL) {
/*NOXXSTR*/
		mesg (ERRALL + 2, ediag ("Error in search string: ",
					 "Ошибка в образце поиска: "), ans);
/*YESXSTR*/
		return -1;
	    }
	    return 1;
	}
    }
    return 0;
}

/*
    Do the "replace" command.
    'Delta' is either 1 for forward replace or -1 for backward replace.
*/
Cmdret
replace (delta)
Small delta;
{
/*NOXXSTR*/
    static S_looktbl rpltable[] = {
	"interactive" , 0,
	"show"        , 0,
	0,              0
    };
/*YESXSTR*/
    Reg1 char *cp;
    char *s1;
    char *s2;
    char *fcp;
    char delim;
    int nl;
    Cmdret tmp;
    Ncols s2len;
    Flag moved;
    int i;
    Cmdret rplopts ();

    if (*(fcp = cmdopstr) == '\0')
	return CRNEEDARG;
    moved = NO;
    nl = 1;
    rplshow = NO;
    rplinteractive = NO;
    tmp = scanopts (&fcp, NO, rpltable, rplopts);
/*       3 marked area        \
/*       2 rectangle           \
/*       1 number of lines      > may stopped on an unknown option
/*       0 no area spec        /
/*      -2 ambiguous option     ) see parmlines, parmcols for type of area
/*   <= -3 other error
/**/
    if (tmp <= -2)
	return tmp;
    if (*(cp = fcp) == '\0') {
	mesg (ERRALL + 1, ediag("No search string",
		       "Не задан образец поиска"));
	return CROK;
    }
    switch (tmp) {
    case 0:
	if (delta > 0)
	    nl = max (0, la_lsize (curlas)
			 - (curwksp->wlin + cursorline)) + 1;
	else
	    nl = curwksp->wlin + cursorline + 1;
	break;

    case 1:
	nl = parmlines;
	break;

    case 2:
	return NORECTERR;

    case 3:
	if (markcols)
	    return NORECTERR;
	nl = marklines;
	break;
    }

#ifdef UNIXV6
#define ispunct(c) (isprint(c) && !isalpha(c) && !isdigit(c))
#endif
    i = U(*cp);
    if (!ispunct (i)) {
	mesg (ERRALL + 1, ediag("Invalid search string delimiter",
			"Плохой ограничитель образца поиска"));
	return CROK;
    }
    delim = *cp++;
    s1 = cp;
    for (; *cp && *cp != delim; cp++)
	continue;
    if (*cp == '\0') {
	mesg (ERRALL + 1, ediag("Unterminated search string",
			"Нет ограничителя образца поиска"));
	return CROK;
    }
    if (cp++ == s1) {
	mesg (ERRALL + 1, ediag("Null search string",
			"Пустой образец поиска"));
	return CROK;
    }
    s2 = cp;
    for (; *cp && *cp != delim; cp++)
	continue;
    if (*cp == '\0') {
	mesg (ERRALL + 1, ediag("Unterminated replacement string",
				"Нет ограничителя подстановки"));
	return CROK;
    }
    fcp = cp++;
    for (; *cp && *cp == ' '; cp++)
	continue;
    if (*cp != '\0') {
	mesg (ERRALL + 1, ediag("Extraneous stuff after replacement string",
				"Есть еще что-то после подстановки"));
	return CROK;
    }
    s2[-1] = '\0';
    if (skeylen (s1, YES, NO, NO) == -1) {
	mesg (ERRALL + 1, ediag("Imbedded newline in search string",
				"В образце нельзя использовать символ ^J"));
	if (moved)
	    putupwin ();
	goto r2;
    }
    *fcp = '\0';
    if ((s2len = skeylen (s2, NO, NO, NO)) == -1) {
	mesg (ERRALL + 1, ediag("Newline in replacement string",
			  "В подстановке нельзя использовать символ ^J"));
	if (moved)
	    putupwin ();
	goto r1;
    }

    if ((rex = regtest (s1)) < 0)
	goto r1;

    if (rpls1 != (char *) NULL) {
	sfree (rpls1);
	sfree (rpls2);
    }
    rpls1 = append (s1, "");
    rpls2 = append (s2, "");
    rpls2len = s2len;

    if (curmark) {
	moved = gtumark (YES);
	unmark ();
    }
    if (moved && rplshow || rplinteractive)
	putupwin ();
    if (rplinteractive) {
	if (searchkey != (char *) NULL)
	    sfree (searchkey);
	searchkey = append (rpls1, "");
	starthere = YES;
	dosearch (delta);
	starthere = NO;
    }
    else {
	doreplace (nl, delta, !moved || rplshow);
	if (moved && !rplshow)
	    putupwin ();
    }
r1: *fcp = delim;
r2: s2[-1] = delim;
    return CROK;
}

/*
    Get the options to the "replace" command.
    For now the only options are "interactive" and "show".
*/
/* not static */
Cmdret
rplopts (cp, str, tmp, equals)
Reg1 char *cp;
char **str;
Small tmp;
Flag equals;
{
    if (equals)
	return CRBADARG;
    switch (tmp) {
    default:
	return -1;

    case 0:
	rplinteractive = YES;
	break;

    case 1:
	rplshow = YES;
	break;
    }
    for (; *cp && *cp == ' '; cp++)
	continue;
    *str = cp;
    return 1;
}

/*
    Actually do the replace at the current position.
    'Delta' is either 1 for forward replace or -1 for backward replace.
    Any newlines in s1 must be at the beginning and/or end of the string
    s1len is exclusive of newlines
    s2 must not have any newlines in it.
*/
static
void
doreplace (nl, delta, puflg)
Nlines nl;
Small delta;
Flag puflg;
{
    Nlines limit;
    Small srchret;

    srchline = curwksp->wlin + cursorline;

    if (delta > 0) {
	limit = min (la_lsize (curlas) - 1, srchline + nl - 1);
	starthere = YES;
    }
    else
	limit = max (0, srchline - nl - 1);

    if ((limit = rangelimit (srchline, delta, limit)) == -1)
	goto ret;

    srchcol = curwksp->wcol + cursorcol;

    savecurs ();

    for (;;) {
	if (delta > 0) {
	    if (srchline > limit)
		break;
	}
	else {
	    if (srchline < limit)
		break;
	}
	if (sintrup ()) {
	    srchret = ABORTED_SRCH;
	    goto Abort;
	}
	if (rplshow) {
	    if ((srchret = dsplsearch (rpls1, srchline, srchcol,
				       limit, delta, NO, YES))
		!= FOUND_SRCH
	       )
		break;
	    freecurs ();
	    savecurs ();
	    if (sintrup ()) {
		srchret = ABORTED_SRCH;
		goto Abort;
	    }
	    setbul (NO);
	}
	else {
	    srchret = strsearch (rpls1, srchline, srchcol, limit,
				      delta, YES);
	    if (srchret == NOTFOUND_SRCH ||
		srchret == OUTRANGE_SRCH)
		break;
	    else if (srchret == ABORTED_SRCH || srchret == BAD_SRCH) {
 Abort:         aborted (srchret == BAD_SRCH);
		loopflags.flash = NO;
		break;
	    }
	}
	rplit (srchline, srchcol, srchlcol, puflg);
	if (delta > 0)
	    srchcol += rpls2len + (rpls1len == 0);
	else
	    srchcol--;
    }
    restcurs ();
 ret:
    starthere = NO;
}

/*    Do the REPLACE key. */
void
replkey ()
{
    Nlines slin;
    Ncols  scol;

    if (rpls1 == (char *) NULL) {
	mesg (ERRALL + 1, ediag("No replacement to do",
				"Не задана замена"));
	return;
    }
    slin = curwksp->wlin + cursorline;
    scol = curwksp->wcol + cursorcol;
    starthere = YES;
    switch (strsearch (rpls1, slin, scol, slin, 1, NO)) {
    case FOUND_SRCH:
	savecurs ();
	rplit (slin, srchcol, srchlcol, YES);
	restcurs ();
	break;
    case NOTFOUND_SRCH:
    case ABORTED_SRCH:
    case BAD_SRCH:
	mesg (ERRALL + 1, ediag("Cursor is not on the string to be replaced",
				"Курсор не в начале образца поиска"));
	break;
    case OUTRANGE_SRCH:
	mesg (ERRALL + 1, ediag("Out of range",
				"Выход за диапазон"));
	break;
    }
    starthere = NO;
}

/*
    Replace it.
    That is, do the actual change at 'slin', 'scol',
*/
static
void
rplit (slin, scol, slcol, puflg)
Nlines slin;
Ncols scol;
Ncols slcol;
Flag puflg;
{
    Ncols wdelta;
    Nlines lin;
    Reg1 Echar *pp;
    Reg2 Uchar *rp;
    Short i;
    Uchar len;

    rpls1len = slcol - scol + 1;
    getline (slin);
    numtyp += rpls1len + rpls2len;

    wdelta = rpls2len - rpls1len;
    if (lcline <= (ncline += wdelta))
	excline ((Ncols) ncline - lcline + 1);

    if (wdelta != 0
	&& (len = (ncline - 1 - (scol + rpls2len)) * sizeof (Echar)) > 0) {
	move ((char *) &cline[scol + rpls1len],
	      (char *) &cline[scol + rpls2len],
	      len);
    }
    cline[ncline] = '\n';

    if (rpls2len > 0) {
	pp = &cline[scol];
	rp = (Uchar *) rpls2;
	for (i = 0; i < rpls2len; i++)
	    *pp++ = U(*rp++);
    }
    fcline = YES;
    redisplay (curfile, slin, (Nlines) 1, (Nlines) 0, puflg);
/*
    if (   puflg
	&& (lin = slin - curwksp->wlin) >= 0
	&& lin <= curwin->btext
       )
	putup (-1, lin, (Scols) max (0, scol - curwksp->wcol),
	       (Scols) (wdelta == 0 ? rpls2len : MAXWIDTH));
*/
}

/*
    Search for the next occurrence of searchkey.
    'Delta' is either 1 for forward search or -1 for backward search.
*/
void
dosearch (delta)
Small delta;
{
    (void) dsplsearch (searchkey,
		curwksp->wlin + cursorline,
		curwksp->wcol + cursorcol,
		delta > 0 ? la_lsize (curlas) - 1 : 0,
		delta, YES, YES);
}

/*
    'Display-search'
    searches through curfile for str starting at [ln, stcol].
    Stop short of 'limit' line.
    'Delta' is either 1 for forward search or -1 for backward search.
    If key is not on current page, positions
    window with key on top line.  Leaves cursor under key.
    If 'delay' is non-0 put up a bullet for one second.
*/
static
Small
dsplsearch (str, ln, stcol, limit, delta, delay, puflg)
char   *str;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag    delay;
Flag    puflg;
{
    Small   srchret;
    Flag    newputup;
    Nlines  winlin;
    Ncols   wincol;
    Nlines  lin;
    Ncols   col;
    Ncols   lkey;

    if (str == 0 || *str == 0) {
	mesg (ERRALL + 1, ediag("Nothing to search for.",
				"Не задан образец поиска."));
	return NOTFOUND_SRCH;
    }

    if (puflg) {
	setbul (NO);
/*NOXXSTR*/
	mesg (TELALL + 3, delta > 0 ? "+" : "-",
/*YESXSTR*/
			  ediag("SEARCH: ", "поиск: "), str);
    }

    switch (srchret = strsearch (str, ln, stcol, limit, delta, YES)) {
    case FOUND_SRCH:
	if (puflg) {
	    winlin = curwksp->wlin;
	    wincol = curwksp->wcol;

	    newputup = NO;
	    lin = srchline - winlin;
	    if (lin < 0 || lin > curwin->btext) {
		newputup = YES;
		if (curwin->btext > 1)
		    lin = defplline;
		else
		    lin = 0;
		if ((winlin = srchline - lin) < 0) {
		    lin += winlin;
		    winlin = 0;
		}
	    }
	    /*if ((lkey = skeylen (str, YES, YES, YES)) == 0)*/
	    /*    lkey = 1;*/
	    if ((lkey = srchlcol - srchcol) <= 0)
		lkey = 1;
	    col = srchcol;
	    if (   col < wincol
		|| col > wincol + curwin->rtext + 1 - lkey
	       ) {
		newputup = YES;
		if (col < curwin->rtext + 1 - lkey)
		    wincol = 0;
		else {
		    wincol = col - (curwin->rtext + 1 - lkey);
		    col = curwin->rtext - lkey + 1;
		}
	    }
	    else
		col -= wincol;
	    clrbul ();
	    if (newputup)
		movewin (winlin, wincol, (Slines) lin, (Scols) col, YES);
	    else
		poscursor ((Scols) col, lin);
	    if (delay)
		setbul (YES);
	}
	break;

    case NOTFOUND_SRCH:
    case ABORTED_SRCH:
    case OUTRANGE_SRCH:
    case BAD_SRCH:
	if (puflg) {
	    clrbul ();
	    if (srchret == NOTFOUND_SRCH)
		mesg (ERRALL + 1, ediag(
			"Search key not found.",
			"Образец поиска не найден."));
	    else if (srchret == OUTRANGE_SRCH)
		mesg (ERRALL + 1, ediag(
			"Search key not found. (range is set)",
			"Образец поиска не найден (в заданном диапазоне)."));
	    else
		aborted (srchret == BAD_SRCH);
	    loopflags.flash = NO;
	}
	break;
    }
    return srchret;
}

/*
    Searches through curfile for str starting at [ln, stcol].
    If 'srch' == 0, merely check to see if current position matches str;
    don't look further.
    'Delta' is either 1 for forward search or -1 for backward search.
    Don't go beyond 'limit' line.
    Assumes curwksp->wfile == curfile.
*/
static
Small
strsearch (str, ln, stcol, limit, delta, srch)
char   *str;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag srch;
{
    if (rex > 0)
	return regsrch (ln, stcol, limit, delta, srch);
    else
	return strsrch (str, ln, stcol, limit, delta, srch);
}

Small
regsrch (ln, stcol, limit, delta, srch)
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag    srch;
{
    Reg1    Echar *at;
    Reg2    Echar *back;
    Reg3    Echar sav;
    Flag    retval;
    Echar   *fc, *lc;
    Flag    first, lastcol;
    int     ans;

    retval = NOTFOUND_SRCH;
    intrupnum = 0;

    if ((limit = rangelimit (ln, delta, limit)) == -1) {
	retval = OUTRANGE_SRCH;
	goto ret;
    }
    if (lastcol = (stcol == -1))
	stcol = 0;
    if (ln >= la_lsize (curlas)) {
	if (delta > 0)
	    goto ret;
	ln = la_lsize (curlas) - 1;
	if (ln < 0)
	    goto ret;
	lastcol = YES;
	stcol = 0;
    }
    else if (!starthere)
	stcol += delta;

    getline (ln);

    for (first = YES;;) {
	if (!first) {
	    if (!srch)
		goto ret;
	    ln += delta;
	    if (   (delta < 0 && ln < limit)
		|| (delta > 0 && ln > limit)
		|| (   intrupnum++ > SRCHPOL
		    && (retval = sintrup () ? ABORTED_SRCH : NOTFOUND_SRCH)
		       == ABORTED_SRCH
		   )
	       )
		goto ret;
	    getline (ln);
	    at = cline;
	    stcol = delta < 0 ? ncline - 1 : 0;
	    back = &cline[ncline - 1];
	}
	else {
	    first = NO;
	    if (lastcol)
		stcol += ncline - 1;
	    if (stcol >= ncline && delta < 0)
		stcol = ncline - 1;
	    else if (stcol < 0 && delta > 0)
		stcol = 0;
	    if (stcol >= ncline || stcol < 0)
		continue;
	    if (delta > 0) {
		at = &cline[stcol];
		back = &cline[ncline - 1];
	    }
	    else {
		at = cline;
		back = &cline[stcol];
	    }
	}

	sav = *back;
	*back = '\0';
	ans = le_exec (at, expbuf, back,
		       (!first || delta < 0 || stcol == 0),
		       (!first || delta > 0 || stcol == ncline - 1),
		       delta);
	*back = sav;

	switch (ans) {
	case 1:   /* found it */
	    le_lim (&fc, &lc);
	    srchcol = fc - cline;
	    srchlcol = lc - cline;
	    retval = FOUND_SRCH;
	    goto ret;
	case 0:
	    break;
	case -1:
	    retval = BAD_SRCH;
	    goto ret;
	}
    }
 ret:
    srchline = ln;
    return retval;
}

static
Small
strsrch (str, ln, stcol, limit, delta, srch)
Uchar   *str;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag srch;
{
    Reg1 Echar  *at;
    Reg2 Uchar  *sk;
    Reg3 Echar *fk;
    Echar   *atcol;
    Nlines  continln;
    Ncols   lkey;
    Flag    nextline;
    Flag    mustmatch;
    Flag    retval;

    retval = NOTFOUND_SRCH;
    intrupnum = 0;
    lkey = skeylen (str, YES, YES, YES);

    if ((limit = rangelimit (ln, delta, limit)) == -1) {
	retval = OUTRANGE_SRCH;
	goto ret;
    }

    if (ln >= la_lsize (curlas)) {
	if (delta > 0)
	    goto ret;
	ln = la_lsize (curlas);
	at = cline;
	getline (ln);
    }
    else {
	getline (ln);
	at = &cline[min (ncline - lkey, stcol)];
    }
    if (starthere)
	at -= delta;

    nextline = NO;
    mustmatch = NO;
    sk = str;
    for (;;) {
#ifdef lint
	continln = 0;
#endif
	for ( at += delta
	    ; nextline || at < cline || at >= &cline[ncline - lkey]
	    ;) {
	    if (!srch)
		goto ret;
	    if (nextline)
		continln++;
	    else
		continln = ln += delta;
	    if (   (delta < 0 && continln < limit)
		|| (delta > 0 && continln > limit && !nextline)
		|| (   intrupnum++ > SRCHPOL
		    && (retval = sintrup () ? ABORTED_SRCH : NOTFOUND_SRCH)
		       == ABORTED_SRCH
		   )
	       )
		goto ret;
	    getline (continln);
	    at = cline;
	    if (nextline)
		break;
	    if (delta < 0)
		at += ncline - 1 - lkey;
	    mustmatch = NO;
	    sk = str;
	}
	fk = at;
	do {
	    if (    U(sk[0]) == ESCCHAR
		&& (sk[1] == 'j' ||
		    sk[1] == 'J')
	       ) {
		if (sk == str && sk[2]) {
		    if (fk == cline) {
			sk += 2;
			mustmatch = NO;
			continue;
		    }
		    else
			break;
		}
		else if (*fk == '\n') {
		    sk += 2;
		    if (*sk == '\0')
			break;
		    else {
			mustmatch = YES;
			if ( !nextline ) {
			    atcol = at;
			    nextline = YES;
			}
			    goto contin_search;
		    }
		}
	    }
	    else if (U(*sk) == U(*fk)) {
		sk++;
		fk++;
		mustmatch = NO;
		continue;
	    }
	    if (mustmatch) {
		if (delta > 0)
		    at = cline + ncline - lkey;
		else
		    at = cline;
	    }
	    break;
	} while (*sk != '\0');

	if (nextline) {
	    nextline = NO;
	    at = atcol;
	}

	if (*sk == '\0') {  /* found it */
	    retval = FOUND_SRCH;
	    break;
	}
	mustmatch = NO;
	sk = str;
 contin_search:
	if (!srch)
	    break;
    }
 ret:
    srchline = ln;
    if (retval == FOUND_SRCH) {
	srchcol = at - cline;
	srchlcol = srchcol + lkey - 1;
    }
    return retval;
}

/*
    Flag nlok;      ok for newlines in string
    Flag imbed;     imbedded newlines are OK
    Flag stopatnl;  stop counting at first newline after other text
    get length of non-newline characters in str
*/
static
Ncols
skeylen (str, nlok, imbed, stopatnl)
Uchar *str;
Flag nlok;
Flag imbed;
Flag stopatnl;
{
    Reg1 Uchar *cp;
    Reg2 Ncols lkey;

    /* get length of searchkey */

    /* skip over initial newlines */
    for (cp = str; ; cp += 2)
	if (    U(cp[0]) == ESCCHAR
	    && (   cp[1] == 'j'
		|| cp[1] == 'J'
	       )
	   ) {
	    if (!nlok)
		return -1;
	}
	else
	    break;

    for (lkey = Z; *cp; cp++, lkey++)
	if (   U(cp[0]) == ESCCHAR
	    && (   cp[1] == 'j'
		|| cp[1] == 'J'
	       )
	   ) {
	    if (!nlok)
		return -1;
	    if (!imbed) {
		if (cp[2] != '\0')
		    return -1;
		else
		    return lkey;
	    }
	    if (stopatnl)
		break;
	    cp++;
	    lkey--;
	}
    return lkey;
}


/*
    If RANGESET, limit the limit to the end of the range.
    If line is less than range, return -1.
*/
static
Nlines
rangelimit (line, delta, limit)
Nlines  line;
Small delta;
Reg1 Nlines  limit;
{
    Reg2 Nlines brange;
    Reg3 Nlines erange;

    if (!(curwksp->wkflags & RANGESET))
	return limit;
    brange = la_lseek (curwksp->brnglas, 0, 1);
    erange = la_lseek (curwksp->ernglas, 0, 1);
    if (delta > 0) {
	if (line < brange)
	    return -1;
	return min (limit, erange);
    }
    else {
	if (line > erange)
	    return -1;
	return max (limit, brange);
    }
}

/*    Do the 'range', '-range', and '?range' commands. */
Cmdret
rangecmd (type)
Small type;
{
    if (   type != CMDRANGE
	&& opstr[0]
       )
	return CRTOOMANYARGS;
    switch (type) {
    case CMDQRANGE:
	if (curwksp->brnglas) {
	    showrange ();
	    return CROK;
	}
	else
	    return NORANGERR;

    case CMDRANGE:
	if (   opstr[0] == '\0'
	    && !curmark
	   ) {
	    if (curwksp->brnglas) {
		curwksp->wkflags |= RANGESET;
		showrange ();
		break;
	    }
	    else
		return NORANGERR;
	}
	Block {
	    Reg1 Nlines nlines;
	    if (opstr[0] != '\0') {
		if (*nxtop)
		    return CRTOOMANYARGS;
		Block {
		    char *str;
		    str = opstr;
		    /* arg string
		    /*  0: was empty
		    /*  1: contained only a number
		    /*  2: contained only a rectangle spec (e.g. "12x16")
		    /*  3: contained a single-word string
		    /*  4: contained a multiple-word string
		    /**/
		    switch (getpartype (&str, YES, NO, curwksp->wlin + cursorline)) {
		    case 1: Block {
			    Reg1 char *cp;
			    for (cp = str; *cp && *cp == ' '; cp++)
				continue;
			    if (*cp != 0) {
		    default:
				mesg (ERRSTRT + 1, opstr);
				return CRUNRECARG;
			    }
			}
			nlines = parmlines;
			break;

		    case 2:
			return NORECTERR;

		    }
		}
		nlines = parmlines;
	    }
	    else {
		if (markcols)
		    return NORECTERR;
		if (gtumark (NO)) {
		    savecurs ();
		    putupwin ();
		    restcurs ();
		}
		nlines = marklines;
		unmark ();
	    }
	    if (!curwksp->brnglas) {
		if (!(curwksp->brnglas = la_clone (curlas, (La_stream *) 0)))
		    return NOMEMERR;
		if (!(curwksp->ernglas = la_clone (curlas, (La_stream *) 0))) {
		    (void) la_close (curwksp->brnglas);
		    return NOMEMERR;
		}
	    }
	    (void) la_lseek (curwksp->brnglas, curwksp->wlin + cursorline, 0);
	    (void) la_align (curwksp->brnglas, curwksp->ernglas);
	    (void) la_lseek (curwksp->ernglas, nlines - 1, 1);
	}
	curwksp->wkflags |= RANGESET;
	break;

    case CMD_RANGE:
	if (curwksp->wkflags & RANGESET)
	    curwksp->wkflags &= ~RANGESET;
	else
	    return NORANGERR;
	break;
    }
    inforange (type == CMDRANGE);
    return CROK;
}

/*    Tell the user what the current range is. */
void
showrange ()
{
    char msgbuf[80];
    Reg1 Nlines begin;
    Reg2 Nlines end;

    begin = la_lseek (curwksp->brnglas, 0, 1) + 1;
    end = la_lseek (curwksp->ernglas, 0, 1) + 1;
    sprintf (msgbuf, ediag(
		"%d through %d = %d lines",
		"от %d до %d = %d строк"),
		begin, end, end - begin + 1);
    mesg (TELALL + 2, (curwksp->wkflags & RANGESET)
		      ? ediag(" Current range is "," Текущий диапазон ")
		      : ediag(" Dormant range is "," Скрытый диапазон "),
		      msgbuf);
    loopflags.hold = YES;
}

/*    Tell the user where he aborted and sleep 1. */
void
aborted (bad)
Flag bad;
{
    char lstr[16];
/*NOXXSTR*/
    sprintf (lstr, "%ld",
/*YESXSTR*/
	     (long) srchline + 1);
    mesg (ERRALL + 4, ediag(
	  "Search aborted at line ",
	  "Поиск прекращен на "),
	  lstr,
/*NOXXSTR*/
	  ediag (".", " строке."),
	  !bad ? "" : ediag (" (search error)", " (ошибка поиска)"));
/*YESXSTR*/
    setbul (YES);
}
