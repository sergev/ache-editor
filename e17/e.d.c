/*
    Display manipulation code.
    Here is where the internal screen image is maintained and output
    minimizing takes place.
*/

#include "e.h"
#include "e.tt.h"
#ifdef UNIXV7
#include <ctype.h>
#endif
#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
#define ISARG(ch) (!ISCNTRL(ch) && ch <= 0177)
#define ISCNTRL(ch) (ch < 040)

/* these are the functions of the physical terminal */
#define MINI0()         (*term.tt_ini0)  ()
#define MINI1()         (*term.tt_ini1)  ()
#define MEND()          (*term.tt_end)   ()
#define MVSCEND()       (*term.tt_vscend)()
#define MVSCSET(b,t)    (*term.tt_vscset)(b, t)
#define MSCRDN(n)       (*term.tt_scrdn) (n)
#define MSCRUP(n)       (*term.tt_scrup) (n)
#define MDELLINE(n)     (*term.tt_delline)(n)
#define MINSLINE(n)     (*term.tt_insline)(n)
#define MCLREOS()       (*term.tt_clres) ()
#define MLEFT()         (*term.tt_left)  ()
#define MRIGHT()        (*term.tt_right) ()
#define MDN()           (*term.tt_dn)    ()
#define MUP()           (*term.tt_up)    ()
#define MCRET()         (*term.tt_cret)  ()
#define MNL()           (*term.tt_nl)    ()
#define MCLEAR()        (*term.tt_clear) ()
#define MHOME()         (*term.tt_home)  ()
#define MBSP()          (*term.tt_bsp)   ()
#define MADDR(l,c)      (*term.tt_addr)  (l,c)
#define MLAD(l)         (*term.tt_lad)   (l)
#define MCAD(c)         (*term.tt_cad)   (c)
#define MXLATE(c)       (*term.tt_xlate) (c)
#define MWL()           term.tt_wl
#define MCWR()          term.tt_cwr
#define MPWR()          term.tt_pwr
#define MAXIS()         term.tt_axis
#define MPRTOK()        term.tt_prtok

static Echar  *image;           /* alloced memory array - screen image */
static Short   icursor;         /* used as index into image */
static Short   ocursor;         /* where the terminal cursor is */

static Short   lincurs;         /* pos of beginning of current line*/

static Scols   winl;            /* left column of window */
static Scols   winr;            /* right column of window */
static Slines  wint;            /* top line of window */
static Slines  winb;            /* bottom line of window */
static Short   winul;           /* upper-left corner of window */
static Scols   width;           /* width of current window */
static Slines  height;          /* height of current window */
static Scols   argcol;          /* marked column */
static Slines  argline;         /* marked line */

static Flag    redraw;          /* we're redrawing screen */
static Flag    noimage;         /* do not use image */
static Small   arg;             /* repeat count for repeatable commands */
static Small   state;           /* state the char is to be analyzed in */
static Flag    wrapflg;         /* VT100 end of line glitch */
static Short obottom;
static Short otop;

Slines  ilin;                   /* current internal image line */
Scols   icol;                   /* current internal image column */
Scols   ocol;                   /* column position of the terminal */
Slines  olin;                   /* line   position of the terminal */
Flag    psgraph = NO;
Echar   attributes = IA_NORMAL; /* current attr of the screen */
Echar   iattrs = IA_NORMAL;
Flag NoAttrs = YES;

static void d_init ();
extern void d_write ();
extern void putmult ();
extern void dbfill ();
extern void dbmove ();
static void putscr ();
extern void fresh ();
static void freshlines ();
static void endreg ();
void gmexit ();
void psexit ();
void set_attributes ();
void restborders ();

/*
    Initialize the screen manager.
    NOTE: The first character in the first call to d_write must be VCCINI.
*/
static
void
d_init (clearmem, clearscr)
Flag clearmem;
Flag clearscr;
{
    if (image == (Echar *) NULL) {
	screensize = term.tt_width * term.tt_height;
	image = (Echar *) salloc ((Uint) screensize * sizeof (Echar), YES);
    }
    if (_putscbuf == (Echar *) NULL) {
	_sizebuf = term.tt_width;
	_putscbuf = (Echar *) salloc ((Uint) (_sizebuf + 1) * sizeof (Echar), YES);
    }

    if (clearmem)
	dbfill (' ', 0, screensize, NO, YES);

    if (clearscr) {
	ocursor = 0;
	ocol = 0;
	olin = 0;
	gmexit ();
	otop = 0;
	obottom = term.tt_height - 1;
	MCLEAR ();
	wrapflg = NO;
    }

    icursor = 0;
    icol = 0;
    ilin = 0;
    lincurs = 0;
    iattrs = IA_NORMAL;

    argcol = argline = 0;
    arg = 1;
    state = 0;

    if (redraw)
	return;

    NoAttrs = (!term.tt_so && !term.tt_mr && !term.tt_mh && !term.tt_md &&
	       !term.tt_mb && !term.tt_as && !term.tt_us);
    width = term.tt_width;
    height = term.tt_height;
    winl = 0;
    winr = width - 1;
    wint = 0;
    winb = height - 1;
    winul = 0;
}


/*
    Write characters to the display.
    Chp points to a mixture of data and control characters.
    The control characters are VCC*** and are defined in e.tt.h.
*/
void
d_write (chp, count)
Reg1 Echar *chp;
Short count;
{
    Reg2 int chr;

    for (; count > 0; chp++, count--) {
	chr = U(*chp);
	for (;;) {
	    switch (state) {
	    case 0:
		if (!ISCNTRL(chr)) {
		    if (arg == 1) {
			if (redraw || (*chp|iattrs) != image[icursor])
			{
			    if ((redraw || !NoAttrs || U(*chp) != U(image[icursor])) &&
				(!redraw || *chp != ' ' || iattrs != IA_NORMAL))
				putscr (*chp|iattrs);
			    image[icursor] = (*chp|iattrs);
			}
			icursor++;
			icol++;
			if (icol >= winr + 1) {
			    if (ilin < winb) {
				icol = 0;
				lincurs = (++ilin) * width;
			    }
			    else {
				icursor--;
				icol--;
			    }
			}
		    }
		    else  putmult (*chp|iattrs);
		}
		else switch (chr) {
		case VCCNUL:
		    putscr (0);
		    iattrs = IA_NORMAL;
		    fflush (stdout);
		    goto nextchar;

		case VCCARG:
		    state = 2;
		    goto nextchar;

		case VCCBEL:
		    if (!silent) {
			gmexit ();
			putchar (7);
		    }
		    goto nextchar;

		case VCCAAD:
		    state = 3;
		    goto nextchar;

		case VCCINI:
		    MINI0 ();
		    d_init (YES, NO);
		    goto nextchar;

		case VCCEND:
		    gmexit ();
		    if (term.tt_vscend)
			endreg (olin, ocol);
		    MEND ();
		    goto nextchar;

		case VCCICL:
		    MINI1 ();
		    d_init (NO, YES);
		    goto nextchar;

		case VCCHOM:
		    icursor = winul;
		    lincurs = icursor - winl;
		    ilin = wint;
		    icol = winl;
		    goto nextchar;

		case VCCNRM:
		    iattrs = 0;
		    goto nextchar;

		case VCCMR:
		    if (term.tt_mr)
			iattrs |= IA_MR;
		    else
			goto so;
		    goto nextchar;

		case VCCMRE:
		    if (term.tt_mr)
			iattrs &= ~IA_MR;
		    else
			goto se;
		    goto nextchar;

		case VCCMH:
		    if (term.tt_mh)
			iattrs |= IA_MH;
		    else
			goto md;
		    goto nextchar;

		case VCCMHE:
		    if (term.tt_mh)
			iattrs &= ~IA_MH;
		    else
			goto mde;
		    goto nextchar;

		case VCCMD:
	md:
		    if (term.tt_md)
			iattrs |= IA_MD;
		    else
			goto so;
		    goto nextchar;

		case VCCMDE:
mde:
		    if (term.tt_md)
			iattrs &= ~IA_MD;
		    else
			goto se;
		    goto nextchar;

		case VCCMB:
		    if (term.tt_mb)
			iattrs |= IA_MB;
		    else
			goto so;
		    goto nextchar;

		case VCCMBE:
		    if (term.tt_mb)
			iattrs &= ~IA_MB;
		    else
			goto se;
		    goto nextchar;

		case VCCUS:
us:
		    if (term.tt_us)
			iattrs |= IA_US;
		    else
			goto so;
		    goto nextchar;

		case VCCUE:
ue:
		    if (term.tt_us)
			iattrs &= ~IA_US;
		    else
			goto se;
		    goto nextchar;

		case VCCAS:
		    if (term.tt_as)
			iattrs |= IA_AS;
		    else
			goto us;
		    goto nextchar;

		case VCCAE:
		    if (term.tt_as)
			iattrs &= ~IA_AS;
		    else
			goto ue;
		    goto nextchar;

		case VCCSO:
so:
		    if (term.tt_so)
			iattrs |= IA_SO;
		    goto nextchar;

		case VCCSE:
se:
		    iattrs &= ~IA_SO;
		    goto nextchar;

		default:
/*NOXXSTR*/
		    fatal (FATALBUG, "Illegal command to terminal simulator");
/*YESXSTR*/
		}

		arg = 1;
		goto nextchar;

	    case 2:                         /* get argument */
		state = 0;
		arg = *chp;
		goto nextchar;

	    case 3:                         /* get column addr */
		state = 5;
		icol = *chp;
		goto nextchar;

	    case 5:                         /* get line addr */
		state = 0;
		ilin = *chp;
		lincurs = (Short) ilin * (Short) width;
		icursor = lincurs + icol;
		goto nextchar;

	    default:
/*NOXXSTR*/
		fatal (FATALBUG, "Illegal state in terminal simulator");
/*YESXSTR*/
	    }
	}
 nextchar: ;
    }
}

#ifdef  NOT_IMPLEMENTED
/*
  move characters horizontally within a line at the current position.
  If `delta' > 0, then insert characters, else delete them.
  The character positions vacated by the move are filled from buf.
*/
int
d_hmove (delta, num, buf)
Scols delta;
Scols num;
char *buf;
{
    if (term.tt_inschar && term.tt_delchar)
	; /* how nice */
    Block {
	Reg2 Echar *rto;

	rto = &image[icursor];
	do {
	    if (*rto != *buf)
		putscr (*rto++ = *buf++);
	    else {
		++rto;
		++buf;
	    }
	    icursor++;
	    icol++;
	} while (--num);
    }
}
#endif NOT_IMPLEMENTED

static
void
toline (lin)
Slines lin;
{
    if (lin < 0 || lin > height - 1) return;
    icol = 0;
    ilin = lin;
    icursor = ilin * width;
    putscr (0);
}

static
void
setreg (top, bottom, totop)
Short bottom;
Short top;
Flag totop;
{
    if (top != otop || bottom != obottom) {
	MVSCSET (top, bottom);
	otop = top;
	obottom = bottom;
    }
    toline (totop ? top : bottom);
}

static
void
endreg (lin, col)
Short lin;
Short col;
{
    if (otop != 0 || obottom != height - 1) {
	ilin = lin;
	icol = col;
	MVSCEND ();
	otop = 0;
	obottom = height - 1;
    }
}

Flag needrest = NO;
Flag usescroll = NO;

/*
  move a rectangle of text vertically
  If `clearok' == YES, then it is ok to use ins/del line which will
  clear the vacated lines.
  If can't do insert/delete, return -1.
  Else if multiple lines can't be moved in one operation, return 1.
  Else return number of lines moved.
*/
int
d_vmove (line, col, aheight, awidth, num, clearok)
Slines line;
Scols col;
Slines aheight;
register Scols awidth;
int num;
Flag clearok;
{
    register Short i;
    register Short nch;
    register Short to;
    Flag isinsdel = term.tt_insline && term.tt_delline;
    Flag useinsdel;
    Short savicursor;
    Scols savicol;
    Slines savilin;
    Short beg, temp;
    Flag savneedrest;

    /* out of heap space */
    useinsdel = (num < 0 && term.tt_scrup && term.tt_vscset
	    || num > 0 && term.tt_scrdn && term.tt_vscset || isinsdel);
    useinsdel = useinsdel
	    || ((num < 0 && term.tt_scrup || num > 0 && term.tt_scrdn)
		 && aheight > height - aheight + abs(num) &&
		    (term.tt_clrel || term.tt_erase));
    useinsdel = clearok && awidth == width && useinsdel;

    if (!useinsdel || silent)
	return -1;
    if (num == 0)
	return 0;

    if (col + awidth > width)
	fatal (FATALBUG, "vmove width to big");
    if (line + aheight > height)
	fatal (FATALBUG, "vmove move area too high");
    if (num < 0 && line < num)
	fatal (FATALBUG, "vmove move too far up");
    if (num > 0 && line + aheight + num > height)
	fatal (FATALBUG, "vmove move too far down");

    savicol = icol;
    savilin = ilin;
    savicursor = icursor;
    savneedrest = needrest;
    noimage = YES;

    if (num < 0) {
	num = -num;

	if (smoothscroll) {
	    line -= num - 1;
	    aheight += num - 1;
	    num = 1;
	}

	nch = num * width;
	to = winul + col + (Short) (line - num) * (Short) width;
	if (useinsdel) {
	    /* do move with insline & delline */
	    if (term.tt_vscset && term.tt_scrup) {
		setreg (wint + line - num,
				    wint + line + aheight - 1, NO);
		MSCRUP (num);
	    }
	    else if (isinsdel && (smoothscroll || singlescroll)) {
		i = num;
		do {
		    toline (wint + line - num);
		    MDELLINE (1);
		    toline (wint + line + aheight - 1);
		    MINSLINE (1);
		} while (--i);
	    } else if (isinsdel) {
		i = num;
		toline (wint + line - num);
		do {
		    i -= MDELLINE (i);
		} while (i);
		i = num;
		toline (wint + line + aheight - num);
		do {
		    i -= MINSLINE (i);
		} while (i);
	    }
	    else {
		needrest = NO;
		if ((i = height - wint - line -
			 aheight - NPARAMLINES * savneedrest) > 0) {
		/* стираем от конца области до конца экрана (физически)*/
		    temp = aheight;
		    if (savneedrest && i > 0) {
			if (wint + line + aheight ==
			    height - 1 - NPARAMLINES)
			    temp++;
		    }
		    if (term.tt_clres) {
			toline (wint + line + temp);
			MCLREOS ();
		    }
		    else {
			beg = winul + col +
			      (wint + line + temp) * width;
			do {
			    dbfill (' ', beg, awidth, YES, NO);
			    beg += width;
			} while (--i);
		    }
		}
		toline (height - 1);
		MSCRUP (num);
		if ((i = wint + line - num -
			(savilin > 1)) > 0) {
		    /* стираем 0 -> top-1 на экране */
		    if (savilin > 1)
			i++;
		    beg = winul + col;
		    do {
			dbfill (' ', beg, awidth, YES, NO);
			beg += width;
		    } while (--i);
		    toline (1);
		    /* восстанавливаем 0 -> top-1 */
		    freshlines (1, wint + line - num - 1, NO);
		}
		usescroll = YES;
	    }
	    i = aheight;
	    do {
		dbmove (to + nch, to, awidth, NO, YES);
		to += width;
	    } while (--i);
	    if (usescroll) {
		i = num;
		do {
		    dbfill (' ', to, awidth, NO, YES);
		    to += width;
		} while (--i);
		if (term.tt_da)
		    if (term.tt_clres)  {
			toline (height - num);
			MCLREOS ();
		    }
		    else {
			i = num;
			beg = winul+col+(height - num)*width;
			do {
			    dbfill (' ', beg, awidth, YES, NO);
			    beg += width;
			} while (--i);
		    }
		freshlines (wint+line+aheight, height-1, NO);
	    }
	    else {      /* not scroll */
		i = num;
		do {
		    dbfill (' ', to, awidth, NO, YES);
		    to += width;
		} while (--i);
	    }
	} else {
	    i = aheight;
	    do {
		dbmove (to + nch, to, awidth, YES, YES);
		to += width;
	    } while (--i);
	}
    } else {
	if (smoothscroll) {
	    aheight += num - 1;
	    num = 1;
	}

	nch = num * width;
	if (useinsdel) {
	    /* do move with insline & delline */
	    if (term.tt_vscset && term.tt_scrdn) {
		setreg (wint + line,
				    wint + line + aheight - 1 + num, YES);
		MSCRDN (num);
	    }
	    else if (isinsdel && (smoothscroll || singlescroll)) {
		i = num;
		do {
		    toline (wint + line + aheight + num - 1);
		    MDELLINE (1);
		    toline (wint + line);
		    MINSLINE (1);
		} while (--i);
	    } else if (isinsdel) {
		i = num;
		toline (wint + line + aheight);
		do {
		    i -= MDELLINE (i);
		} while (i);
		i = num;
		toline (wint + line);
		do {
		    i -= MINSLINE (i);
		} while (i);
	    }
	    else {
		needrest = NO;
		if ((i = wint + line - savneedrest) > 0) {
		    /* стираем то, что осталсь вверху (физически) */
		    if (savneedrest)
			i++;
		    beg = winul + col;
		    do {
			 dbfill (' ', beg, awidth, YES, NO);
			 beg += width;
		    } while (--i);
		}
		toline (0);
		MSCRDN (num);
		if ((i = height - wint - line -
		       aheight - num - NPARAMLINES * savneedrest) > 0) {
		    /* стираем то, что осталось внизу (физически) */
		    temp = aheight;
		    if (savneedrest && i > 0) {
			if (wint + line + aheight + num ==
			    height - 1 - NPARAMLINES)
			    temp++;
		    }
		    if (term.tt_clres) {
			toline (wint + line + num + temp);
			MCLREOS ();
		    }
		    else {
			beg = winul + col +
			     (temp + wint + line + num) * width;
			do {
			    dbfill (' ', beg, awidth, YES, NO);
			    beg += width;
			} while (--i);
		    }
		    /* восстанавливаем низ */
		    freshlines (wint+line+aheight+num, height-1, NO);
		}
		usescroll = YES;
	    }
	    to = winul + col
	       + (Short) (line + aheight + num) * (Short) width;
	} else {
	    /* first display the change from top to bottom */
	    to = winul + col
		+ (Short) (line + num) * (Short) width;
	    i = aheight;
	    do {
		dbmove (to - nch, to, awidth, YES, NO);
		to += width;
	    } while (--i);
	}

	/* then update image from bottom to top */
	i = aheight;
	do {
	    to -= width;
	    dbmove (to - nch, to, awidth, NO, YES);
	} while (--i);

	if (useinsdel) {
	    if (usescroll) {
		i = num;
		do {
		    to -= width;
		    dbfill (' ', to, awidth, NO, YES);
		} while (--i);
		if (term.tt_db) {
		    i = num;
		    beg = winul + col;
		    do {
			dbfill (' ', beg, awidth, YES, NO);
			beg += width;
		    } while (--i);
		}
		freshlines(0, wint + line - 1, NO);
	    }
	    else {      /* not scroll */
		i = num;
		do {
		    to -= width;
		    dbfill (' ', to, awidth, NO, YES);
		} while (--i);
	    }
	}
    }
    icol = savicol;
    ilin = savilin;
    icursor = savicursor;
    noimage = NO;
    return num;
}

/*    Puts multiple characters to the display. */
void
putmult (chr)
register Short chr;
{
    register Short i, j;

    if (needrest)
	restborders();

    for (;;) {
	i = winr - (icursor - lincurs) + 1;
	j = min (i, arg);
	arg -= j;
	dbfill (chr, icursor, j, !silent, YES);
	if (j < i) {
	    icursor += j;
	    icol += j;
	    arg = 1;
	    return;
	}
	if (arg <= 0 || ilin == winb) {
	    arg = 1;
	    icursor += width - 2;
	    icol += width - 2;
	    return;
	}
	icursor += width;
	lincurs += width;
	ilin++;
	icol = 0;
    }
}

/*
    Fill the screen with nchars characters (chr) starting at image[to].
    If displflg, update the terminal screen.
    If wrtflg, update the internal image.
    Nchars must not be enough to go beyond the right edge of the screen.
    Try to use tt_erase if chr is a blank.
*/
void
dbfill (chr, to, nchars, displflg, wrtflg)
Short chr;
Short to;
Reg1 Short nchars;
Flag displflg;
Flag wrtflg;
{
    if (nchars <= 0)
	return;
    if (displflg) {
	Reg3 Short savicursor;
	Reg4 Scols savicol;
	Reg5 Slines savilin;

	savicol = icol;
	savilin = ilin;
	savicursor = icursor;
	icursor = to;
	ilin = to / width;
	icol = to % width;
	if (chr == ' ' && nchars > 5) {
	    if (term.tt_erase) {
		putscr (0); /* update terminal cursor */
		(*term.tt_erase) (nchars);
	    }
	    else if (term.tt_clrel) {
		Reg6 Echar *cp;

		Block {
		    Reg2 Echar *rto;
		    Reg3 Echar *lim;

		    rto = &image[to + nchars];
		    lim = &image[(ilin + 1) * width];
		    cp = (Echar *) NULL;
		    while (rto < lim) {
			if (*rto != ' ') {
			    if (cp)
				goto punt;
			    cp = rto;
			}
			rto++;
		    }

		}
		Block {
		    putscr (0); /* update terminal cursor */
		    (*term.tt_clrel) ();
		    if (cp) {
			icol = (icursor = cp - image) - ilin * width;
			putscr (*cp);
		    }
		}
	    }
	    else
		goto punt;
	}
	else Block {
	    Reg6 Short savnchars;
	    Reg2 Echar *rto;
punt:
	    rto = &image[to];
	    savnchars = nchars;
	    do {
		if (*rto != chr || redraw && chr != ' ')
		    putscr (chr);
		rto++;
		icursor++;
		icol++;
	    } while (--nchars);
	    nchars = savnchars;
	}
    icursor = savicursor;
    ilin = savilin;
    icol = savicol;
    }
    if (wrtflg) {
	Reg2 Short i;

	for (i = 0; i < nchars; i++)
	    image[to + i] = chr;
    }
}

void
dbmove (from, to, nchars, displflg, wrtflg)
Short from, to; /* indexes into the image[] array */
Short nchars;
Flag displflg;
Flag wrtflg;
{

    if (nchars <= 0 || from == to && (wrtflg || !redraw))
	return;
    if (displflg) {
	Reg1 Echar *rfrom;
	Reg2 Short rnchars;
	Short savicursor;
	Slines savilin;
	Scols savicol;
	Reg3 Echar *rto;

	rto = &image[to];
	rfrom = &image[from];
	rnchars = nchars;
	savicursor = icursor;
	savicol = icol;
	savilin = ilin;
	icursor = to;
	ilin = icursor / width;
	icol = icursor % width;
	do {
	    if (!redraw && *rto != *rfrom ||
		redraw && *rfrom != ' ')
		putscr (*rfrom);
	    rfrom++;
	    rto++;
	    icursor++;
	    icol++;
	} while (--rnchars);
	icursor = savicursor;
	ilin = savilin;
	icol = savicol;
    }
    else if (wrtflg)
	move ((char *) &image[from], (char *) &image[to],
	      (Uint) nchars * sizeof (Echar));
}


/*
    Put out chr at icursor.
    Should only be called with printing characters, or 0 which forces
    the cursor to be where it is supposed to be.
    This routine would be better structured if it called two routines:
    one to get us in the right position, and one to put out the character.
    However, since it is called once per output character, that would
    cost a bit in performance.
*/
static
void
putscr (chr)
Echar chr;
{
    Reg1 Slines lin;
    Reg2 Scols col;
    Reg3 Short c = U(chr);
    Reg4 Flag norm;

    if (needrest)
	restborders();
    if (silent)
	return;
    col = icol;
    lin = ilin;

    if (ocursor != icursor) {
	if (psgraph)
	    psexit ();
	if (wrapflg)
	    goto coordinate;
	if (col == 0) {
	    switch (lin - olin) {
	    case -1:
		if (olin >= otop && lin < otop ||
		    !term.tt_cret || !term.tt_nup)
		    goto addr;
		MCRET ();
		MUP ();
		break;

	    case 0:
		if (!term.tt_cret)
		    goto addr;
		MCRET ();
		break;

	    case 1:
		if (olin <= obottom && lin > obottom)
		    goto addr;
		if (MCWR () == 1 && icursor - ocursor == 1)
		    goto wrap;
		if (!term.tt_nl)
		    goto addr;
		MNL ();
		break;

	    default:
		if (icursor == 0) {
		    MHOME ();
		    break;
		}
		else if (term.tt_ll &&
			icursor == (height - 1) * width) {
			(*term.tt_ll) ();
			break;
		}
		goto try1;
	    }
	}
	else {
 try1:
	    switch (icursor - ocursor) {
	    case -3:
		if (   !tt_lt3
		    || (col >= width - 3 && MWL () != 1)
		   )
		    goto addr;
		MLEFT ();
		MLEFT ();
		MLEFT ();
		break;

	    case -2:
		if (   !tt_lt2
		    || (col >= width - 2 && MWL () != 1)
		   )
		    goto addr;
		MLEFT ();
		MLEFT ();
		break;

	    case -1:
		if (   term.tt_nleft == 0
		    || (col >= width - 1 && MWL () != 1)
		   )
		    goto addr;
		MLEFT ();
		break;

	    case 3:
		if (col <= 2 && MCWR () != 1)
		    goto addr;
		if (!tt_rt3) {
		    if (noimage || attributes || !MPRTOK () ||
			(image[ocursor] &~ CHARMASK) ||
			(image[ocursor + 1] &~ CHARMASK) ||
			(image[ocursor + 2] &~ CHARMASK) ||
			!isprint (image[ocursor]) ||
			!isprint (image[ocursor + 1]) ||
			!isprint (image[ocursor + 2])
		       )
			goto addr;
		    putchar (image[ocursor]);
		    putchar (image[ocursor + 1]);
		    putchar (image[ocursor + 2]);
		} else {
		    MRIGHT ();
		    MRIGHT ();
		    MRIGHT ();
		}
		break;

	    case 2:
		if (col <= 1 && MCWR () != 1)
		    goto addr;
		if (!tt_rt2) {
		    if (noimage || attributes || !MPRTOK () ||
			(image[ocursor] &~ CHARMASK) ||
			(image[ocursor + 1] &~ CHARMASK) ||
			!isprint (image[ocursor]) ||
			!isprint (image[ocursor + 1])
		       )
			goto addr;
		    putchar (image[ocursor]);
		    putchar (image[ocursor + 1]);
		} else {
		    MRIGHT ();
		    MRIGHT ();
		}
		break;

	    case 1:
		if (col <= 0 && MCWR () != 1)
		    goto addr;
		if (term.tt_nright != 1) {
		    if (!noimage && !attributes && MPRTOK () &&
			!(image[ocursor] &~ CHARMASK) &&
			isprint (image[ocursor]))
			putchar(image[ocursor]);
		    else if (term.tt_nright)
			goto wrap;
		    else
			goto addr;
		} else
wrap:                   MRIGHT ();
		break;

	    default:
	    addr:
		if (lin == olin) {
		    if (MAXIS () & 02)
			MCAD (col);
		    else
			goto coordinate;
		}
		else if (col == ocol) {
		    if (abs (lin - olin) <= 2) {
			if ((lin - olin < 0 && otop > lin && otop <= olin ||
			    lin - olin > 0 && obottom < lin && obottom >= olin))
			    goto coordinate;
			switch (lin - olin) {
			case -2:
			    if (!term.tt_nup)
				goto coordinate;
			    MUP ();
			    MUP ();
			    break;

			case -1:
			    if (!term.tt_nup)
				goto coordinate;
			    MUP ();
			    break;

			case 0:
			    /* not imossible (wrap) */
			    goto coordinate;

			case  2:
			    if (!term.tt_ndn)
				goto coordinate;
			    MDN ();
			    MDN ();
			    break;

			case  1:
			    if (!term.tt_ndn)
				goto coordinate;
			    MDN ();
			    break;
			}
		    }
		    else if (MAXIS () & 1)
			MLAD (lin);
		    else
			goto coordinate;
		}
		else
 coordinate:            MADDR (lin, col);
		break;
	    }
	}
	ocursor = icursor;
	olin = lin;
	ocol = col;
	wrapflg = NO;
    }

    if (c < 32) {
	gmexit ();
	return;
    }

    if (wrapflg)
	goto coordinate;

    if (ocol < width - 1) {
	ocursor++;
	ocol++;
    }
    else {
	switch (MPWR ()) {
	case 0:
	    if (ocursor < screensize - 1) {
		olin = height + 10;
		ocol = width + 10;
		ocursor = olin * width + ocol;
		otop = obottom = height + 10;
		if (term.tt_vscend)
		    endreg (0, 0);
	    }
	    break;

	case 1:
	    if (ocursor < screensize - 1) {
		if (term.tt_vscend && olin == obottom)
		    endreg (olin, ocol);
		ocursor++;
		olin++;
		ocol = 0;
	    }
	    break;

	case 2:
	    ocursor -= width - 1;
	    ocol = 0;
	    break;

	case 4:
	    wrapflg = YES;
	case 3:
	    break;
	}
    }

putx:
    norm = isprint (c)
#ifndef  CTYPE
	||
#ifdef  CHANGECTYPE
	!correctype &&
#endif
	isrus8 (c)
#endif
	   ;

    if (psgraph && norm)
	psexit ();

    if (MPRTOK () && norm)
	;
    else if (!norm) {
	MXLATE (chr);
	return;
    }
    else Block {
	Char x;
	Flag need = NO;

#ifndef CTYPE
	if (
#ifdef  CHANGECTYPE
	    !correctype &&
#endif
	    isrus8 (c)) {
	    need = YES;
	    c = tomap8 (c);
	    chr = (c | (chr &~ CHARMASK)) ^ IA_MR;
	}
#endif
	if (isuppercase && (x = changechar (c, YES)) != '\0') {
	    need = YES;
	    c = U(x);
	    chr = (c | (chr &~ CHARMASK)) ^ IA_MD;
	}
	if (need)
	    goto putx;
    }
    if (attributes != (chr &~ CHARMASK))
	set_attributes (chr &~ CHARMASK);
    putchar (c);
}


/*    Put up a fresh screen. */
void
fresh ()
{
    if (replaying) return;
    needrest = NO;
    if (silent) return;
    savecurs ();
    redraw = YES;
    if (windowsup) {
	psgraph = YES;
	attributes = ~0;
	otop = obottom = height + 10;
	(*kbd.kb_end) ();
	d_put (VCCEND);
	d_flush ();
    }
    (*kbd.kb_init) ();                  /* initialize keyboard */
    (*term.tt_ini1) ();                 /* initialize the terminal */
    d_init (NO, YES);
    d_flush ();
    d_write (image, screensize);
    redraw = NO;
    restcurs ();
}

static
void
freshlines (top, bottom, border)
{
	Reg1 Short i;

	if (top > bottom) return;
	if (top < 0) top = 0;
	if (bottom > height - 1) bottom = height - 1;
	redraw = YES;
	for (i = top; i <= bottom; i++) {
		if (!border &&
		    (i == 0 || i >= height - 1 - NPARAMLINES))
		    continue;
		dbmove (i * width, i * width,
		   width, YES, NO);
	}
	redraw = NO;
}

void
restborders()
{
	needrest = NO;
	if (silent) return;
	redraw = noimage = YES;
	freshlines (0, 0, YES);
	freshlines (height - 1 - NPARAMLINES,
		    height - 1, YES);
	redraw = noimage = NO;
}

void
gmexit ()
{
    if (psgraph)
	psexit ();
    if (attributes != IA_NORMAL)
	set_attributes (IA_NORMAL);
}

void
psexit ()
{
    if (psgraph) {
	if (term.tt_gexit)
	     (*term.tt_gexit) ();
	psgraph = NO;
    }

}

void
set_attributes (attrs)
Echar attrs;
{
    if (attrs == attributes)
	return;
    if (attrs == IA_NORMAL) {
	if (attributes & IA_AS) {
	    if (term.tt_ae)
		(*term.tt_ae) ();
	    attributes &= ~IA_AS;
	}
	if (attributes == IA_NORMAL)
	    return;
	if (term.tt_mexit)
	    (*term.tt_mexit) ();
	else if ((attributes & IA_SO) && term.tt_so)
	    (*term.tt_se) ();
	else if ((attributes & IA_US) && term.tt_us)
	    (*term.tt_ue) ();
	attributes = IA_NORMAL;
	return;
    }
    if (!(attrs & IA_US) && (attributes & IA_US) && term.tt_ue) {
	(*term.tt_ue) ();
	attributes &= ~IA_US;
    }
    if (!(attrs & IA_SO) && (attributes & IA_SO) && term.tt_se) {
	(*term.tt_se) ();
	attributes &= ~IA_SO;
    }
    if (!(attrs & IA_AS) && (attributes & IA_AS) && term.tt_ae) {
	(*term.tt_ae) ();
	attributes &= ~IA_AS;
    }
    if (!(attrs & IA_MD) && (attributes & IA_MD) ||
	!(attrs & IA_MB) && (attributes & IA_MB) ||
	!(attrs & IA_US) && (attributes & IA_US) ||
	!(attrs & IA_SO) && (attributes & IA_SO) ||
	!(attrs & IA_MH) && (attributes & IA_MH) ||
	!(attrs & IA_MR) && (attributes & IA_MR)) {
	if (term.tt_mexit)
	    (*term.tt_mexit) ();
	attributes &= IA_AS;
    }
    if ((attrs & IA_AS) && !(attributes & IA_AS)) {
	if (!term.tt_as)
	    goto us;
	(*term.tt_as) ();
	attributes |= IA_AS;
    }
    if ((attrs & IA_US) && !(attributes & IA_US)) {
us:
	if (!term.tt_us)
	    goto so;
	if (!(attributes & IA_US)) {
	    (*term.tt_us) ();
	    attributes |= IA_US;
	}
    }
    if ((attrs & IA_MR) && !(attributes & IA_MR)) {
	if (!term.tt_mr)
	    goto so;
	(*term.tt_mr) ();
	attributes |= IA_MR;
    }
    if ((attrs & IA_MH) && !(attributes & IA_MH)) {
	if (!term.tt_mh)
	    goto md;
	(*term.tt_mh) ();
	attributes |= IA_MH;
    }
    if ((attrs & IA_MD) && !(attributes & IA_MD)) {
md:
	if (!term.tt_md)
	    goto so;
	if (!(attributes & IA_MD)) {
	    (*term.tt_md) ();
	    attributes |= IA_MD;
	}
    }
    if ((attrs & IA_MB) && !(attributes & IA_MB)) {
	if (!term.tt_mb)
	    goto so;
	(*term.tt_mb) ();
	attributes |= IA_MB;
    }
    if ((attrs & IA_SO) && !(attributes & IA_SO)) {
so:
	if (term.tt_so && !(attributes & IA_SO)) {
	    (*term.tt_so) ();
	    attributes |= IA_SO;
	}
    }
}

bell ()
{
    d_put (VCCBEL);
}
