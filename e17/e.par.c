#include "e.h"
#include "e.mac.h"
#include "e.cc.h"
#include "e.tt.h"
#include "e.m.h"
#include "e.hi.h"
#ifdef UNIXV7
#include <ctype.h>
#endif

static void partyp ();
static void parputch ();
static void movehistory ();
extern Flag entering;   /* in e.t.c */
#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
extern S_window *msowin;  /* e.mesg.c */
extern Scols  msocol;
extern Slines msolin;
extern Scols lastcol;

/*
    Take input from the Command Line.
    For historical reasons, this is spoken of as 'getting a parameter'.
    There are three types of parameters:
	    paramtype = -1 -- cursor defined
	    paramtype = 0  -- just <arg> <function>
	    paramtype = 1  -- string, either integer or text:
    Returns pointer to the text string entered.
    The pointer is left in the global variable paramv, and
    its alloced length in bytes (not string length) in ccmdl.
*/
void
param ()
{
    Reg1 Short i;
    Reg4 Short ppos;
    Reg5 Flag cmdflg;
    Reg7 Flag uselast;
    Small history;

    uselast = NO;
    history = -1;       /* первый раз */
    entering = YES;     /* set this flag for getkey1() */

    savecurs ();
    setbul (NO);

    if (ccmd.l == 0) {  /* first time only по каждому из history */
	ccmd.p = salloc (ccmd.l = LPARAM, YES);
	ccmd.p[ccmd.len = 0] = '\0';
    }
    if (hcmd == (S_cmd *) NULL)
	hcmd = (S_cmd *) salloc (nhistory * sizeof (S_cmd), YES);

 parmstrt:
    if (uselast) Block {
	Reg1 char *c1;
	Reg2 char *c2;
	char tmp[30];

	uselast = NO;
	(void) sprintf (tmp, "[-%d] ", history + 1);
	mesg ((TELSTRT|TELCLR) + 1, cmdmode ? "" : tmp);
	if (hcmd[history].len > 0) {
	    if (hcmd[history].len + 1 > ccmd.l) {
		ccmd.l = hcmd[history].len / LPARAM + 1;
		ccmd.p = gsalloc (ccmd.p, ccmd.l *= LPARAM, YES);
	    }
	    for (c1 = ccmd.p, c2 = hcmd[history].p; *c2 != '\0'; c2++)
		parputch (*c1++ = *c2);
	    *c1 = '\0';
	    ppos = ccmd.len = hcmd[history].len;
	}
	else {
	    ccmd.p[0] = '\0';
	    ppos = Z;
	    ccmd.len = Z;
	}
	getkey (WAIT_KEY);
    }
    else {      /* Not use last command */
	mesg ((TELSTRT|TELCLR) + 1, cmdmode ? "" : ediag("CMD: ","АРГ: "));
	ccmd.p[0] = '\0';
	ppos = Z;
	ccmd.len = Z;
	getkey (WAIT_KEY);
	switch (key) {
	case CCDELCH:
	/*
	case CCMOVELEFT:
	case CCMOVERIGHT:
	case CCMOVEUP:
	case CCMOVEDOWN:
	case CCENDS:
	case CCBEGS:
	*/
	case CCBACKSPACE:
	    goto done;
	}
    }

    cmdflg = NO;
    for (; ; cmdflg = key == CCCMD, keyused = YES, getkey (WAIT_KEY)) {
	i = max (ccmd.len, ppos);
	if (i + 1 > ccmd.l) {
	    ccmd.l = i / LPARAM + 1;
	    ccmd.p = gsalloc (ccmd.p, ccmd.l *= LPARAM, YES);
	}
	while (ccmd.len < ppos)
	    ccmd.p[ccmd.len++] = ' ';
	ccmd.p[ccmd.len] = '\0';
	if (!isprint (key)
#ifndef CTYPE
	    && (
#ifdef  CHANGECTYPE
	    correctype ||
#endif
	    !isrus8 (key))
#endif
	   )
	    switch (key) {
	    case CCBACKSPACE:
		if (ppos) {
		    if (cmdflg) {
			if (insmode) Block {
			    Reg1 char  *c1;
			    Reg2 char *c2;

			    poscursor (cursorcol - ppos, cursorline);
			    if ((i = ccmd.len - ppos) > 0) {
				c2 = &(c1 = ccmd.p)[ppos];
				do {
				    parputch (*c1++ = *c2++);
				} while (--i);
			    }
			    i = ppos;
			    multchar (' ', i);
			    poscursor (cursorcol - ccmd.len, cursorline);
			    ccmd.len -= ppos;
			    ppos = 0;
			}
			else Block {
			    Reg1 char  *c1;

			    i = ppos;
			    c1 = ccmd.p;
			    poscursor (cursorcol - ppos, cursorline);
			    do
				parputch (*c1++ = ' ');
			    while (--i);
			}
		    }
		    else {
			if ((i = ccmd.len - ppos--) > 0 && insmode) {
			    movecursor (LT, 1);
			    goto delchr;
			}
			else {
			    if (i == 0)
				ccmd.len--;
			    movecursor (LT, 1);
			    parputch (ccmd.p[ppos] = ' ');
			    movecursor (LT, 1);
			}
		    }
		}
		break;

	    case CCDELCH:
		if ((i = ccmd.len - ppos) > 0) {
		    if (cmdflg) {
			ccmd.p[ppos] = '\0';
			savecurs ();
			multchar (' ', i);
			restcurs ();
			ccmd.len = ppos;
		    }
		    else {
 delchr:                if (i > 0)
			    move (&ccmd.p[ppos + 1], &ccmd.p[ppos],
				  (Uint) i);
			ccmd.len--;
			savecurs ();
			for (i = ppos; i < ccmd.len; )
			    parputch (ccmd.p[i++]);
			parputch (' ');
			restcurs ();
		    }
		}
		break;

	    case CCINSMODE:
		tglinsmode ();
		break;

	    case CCALT:
		infoinreg (rusbit = !rusbit);
		break;

	    case CCRUS:
	    case CCLAT:
		infoinreg (rusbit = (key == CCRUS));
		break;

	    case CCBEGS:
	    case CCMOVELEFT:
		if (cmdflg)
		    goto done;
		if (ppos) {
		    if (key == CCBEGS) {
			while (ccmd.p[ccmd.len - 1] == ' ')
			    ccmd.len--;
			ccmd.p[ccmd.len] = '\0';
			for (i = 0; i < ccmd.len && ccmd.p[i] == ' '; i++)
			    ;
		    }
		    if ((i = key == CCBEGS ? ppos - i : 1) > 0) {
			movecursor (LT, i);
			do {
			    if (ppos-- == ccmd.len && ccmd.p[ppos] == ' ')
				ccmd.p[--ccmd.len] = '\0';
			} while (--i);
		    }
		    else {
			movecursor (RT, -i);
			ppos += -i;
		    }
		}
		break;

	    case CCENDS:
	    case CCMOVERIGHT:
		if (cmdflg)
		    goto done;
		if (key == CCENDS) {
		    while (ccmd.p[ccmd.len - 1] == ' ')
			ccmd.len--;
		    ccmd.p[ccmd.len] = '\0';
		}
		for (;;) {
		    if (ppos < ccmd.len) {
			ppos++;
			movecursor (RT, 1);
			if (key == CCMOVERIGHT)
			    break;
		    }
		    else if (key == CCENDS) {
			if (ppos == ccmd.len)
			    break;
			else {
			    ppos--;
			    movecursor (LT, 1);
			}
		    }
		    else {
			ppos++;
			movecursor (RT, 1);
			break;
		    }
		}
		break;

	    case CCTAB:
	    case CCBACKTAB:
		if (cmdflg)
		    goto done;
		i = cursorcol - ppos;
		movecursor (cntlmotions[key], 1);
		ppos = cursorcol - i;
		break;

	    case CCPICK:
		if (!cmdflg)
		    goto done;
		Block {
		    int wlen; /* must be int because of pkarg argument */
		    Reg1 char *wcp;

		    if ((wcp = pkarg (msowin->wksp,
				       msowin->wksp->wlin + msolin,
					msowin->wksp->wcol + msocol,
					 &wlen, PK_WHOLE)) == (char *) NULL)
			break;
		    if (ccmd.len + wlen + 1 > ccmd.l) {
			ccmd.l = (ccmd.len + wlen) / LPARAM + 1;
			ccmd.p = gsalloc (ccmd.p, ccmd.l *= LPARAM, YES);
		    }
		    if ((i = ccmd.len - ppos) > 0)
			move (&ccmd.p[ppos], &ccmd.p[ppos + wlen],
			      (Uint) i);
		    /* insert the word */
		    if (wlen > 0)
			(void) move (wcp, &ccmd.p[ppos], (Uint) wlen);
		    sfree (wcp);
		    ccmd.len += wlen;
		    /* update screen */
		    Block {
			Reg1 char  *c1;
			Reg3 Slines lin;
			Reg6 Scols col;

			col = cursorcol + wlen;
			lin = cursorline;
			c1 = &ccmd.p[ppos];
			for (i = ccmd.len - ppos; i-- > 0; )
			    parputch (*c1++);
			poscursor (col, lin);
		    }
		    ppos += wlen;
		}
		break;

	    case CCMOVEUP:
		if (cmdflg)
		    goto done;
		if (history == -1 || ++history > nhistory - 1)
		    history = 0;
		uselast = YES;
		keyused = YES;
		mesg (TELSTOP);
		goto parmstrt;

	    case CCMOVEDOWN:
		if (cmdflg)
		    goto done;
		if (history == -1 || --history < 0)
		    history = nhistory - 1;
		uselast = YES;
		keyused = YES;
		mesg (TELSTOP);
		goto parmstrt;

	    case CCRETURN:
		goto done;

	    case CCCMD:
		break;

	    case CCCTRLQUOTE:
		key = ESCCHAR;
		goto prchar;

	    case CCINT:
		keyused = YES;
		putc (CCINT, keyfile);
		if (cmdmode || cmdflg) {
		    mesg (TELSTOP);
		    goto parmstrt;
		}
		i = ccmd.len;
		ccmd.len = 0;
		goto done;

	    default:
		if (!cmdflg)
		    goto done;
	    }
	else Block { /* печатный символ */
	    Reg2 char chr;

prchar:     chr = '\0';
	    if (!cyrillflg && isrus8 (key)) {
		chr = tomap8 (key);
		key = CYRILLCH;
	    }
	    else if (uppercaseflg && isprint (key) &&
		(chr = changechar (key, YES)) != '\0')
		key = BEFORECH;
Yet_another:
	    if (insmode && ppos < ccmd.len) {
		(void) move (&ccmd.p[ppos], &ccmd.p[ppos + 1],
		      (Uint) (ccmd.len++ - ppos));
		ccmd.p[ppos] = key;
		Block {
		    Reg2 Slines lin;
		    Reg3 Scols  col;

		    col = cursorcol + 1;
		    lin = cursorline;
		    for (i = ppos++; i < ccmd.len; i++)
			    parputch (ccmd.p[i]);
		    poscursor (col, lin);
		}
	    }
	    else {
		if (ppos == ccmd.len)
		    ccmd.len++;
		parputch (ccmd.p[ppos++] = key);
	    }
	    if (chr != '\0') {
		if (ccmd.len + 1 > ccmd.l) {
		    ccmd.l = ccmd.len / LPARAM + 1;
		    ccmd.p = gsalloc (ccmd.p, ccmd.l *= LPARAM, YES);
		}
		key = U(chr);
		chr = '\0';
		goto Yet_another;
	    }
	}
    }
 done:
    if ((i = (key == CCINT ? i : ccmd.len) - ppos) > 0)
	movecursor (RT, i);
    mesg (TELSTOP);
    ccmd.p[ccmd.len] = '\0';
    if (ccmd.len == 0)
	paramtype = 0;
    else
	partyp ();
    paramv = ccmd.p;
    if (ccmd.len > 0) {
	movehistory ();
	if (hcmd[0].len + 1 > ccmd.l) {
	    ccmd.l = hcmd[0].len / LPARAM + 1;
	    ccmd.p = gsalloc (ccmd.p, ccmd.l *= LPARAM, YES);
	}
	move (hcmd[0].p, ccmd.p, (Uint) (hcmd[0].len + 1));
	ccmd.len = hcmd[0].len;
    }
    restcurs ();
    clrbul ();
    entering = NO;
}

static
void
partyp ()
{
	/*  0: was empty
	/*  1: contained only a number
	/*  2: contained only a rectangle spec (e.g. "12x16")
	/*  3: contained a single-word string
	/*  4: contained a multiple-word string
	/**/
	char *c2;

	c2 = ccmd.p;

	paramtype = getpartype (&c2, YES, NO, curwksp->wlin + cursorline);

	switch (paramtype) {
	case 1:
	case 2:
	    Block {
		Reg1 char *cp;

		for (cp = c2; *cp && *cp == ' '; cp++)
		    continue;
		if (*cp)
		    paramtype = 4; /* more than one string */
	    }
	    break;

	case 3:
	    Block {
		Reg1 char *cp;

		for (cp = ccmd.p; *cp && *cp == ' '; cp++)
		    continue;
		for (; *cp && *cp != ' '; cp++)
		    continue;
		for (; *cp && *cp == ' '; cp++)
		    continue;
		if (*cp)
		    paramtype = 4;
	    }
	}
}

/*
    Продвигает вперед history, стирая последнюю команду,
    делая первую пустой.
*/
static
void
movehistory ()
{
    Reg1 Short i;
    S_cmd temp;

    temp = ccmd;
    ccmd = hcmd[nhistory - 1];
    if (ccmd.l == 0)   /* first time only по каждому из history */
	ccmd.p = salloc (ccmd.l = LPARAM, YES);
    ccmd.p[ccmd.len = 0] = '\0';
    for (i = nhistory - 2; i >= 0; i--)
	hcmd[i + 1] = hcmd[i];
    hcmd[0] = temp;
}

/*
    Get the argument from the edit window.
    Called from mainloop ().
*/
void
getarg (pktype)
Small pktype;
{
    Reg1 char  *cp;
    int len;     /* not register */

    if (ccmd.l == 0) {
	ccmd.p = salloc (ccmd.l = LPARAM, YES);
	ccmd.p[ccmd.len = 0] = '\0';
    }

    if ((cp = pkarg (curwksp, curwksp->wlin + cursorline,
		     curwksp->wcol + cursorcol, &len, pktype)) == (char *) NULL) {
	ccmd.p[ccmd.len = 0] = '\0';
	return;
    }

    if (ccmd.l < len + 1) {
	ccmd.l = len / LPARAM + 1;
	ccmd.p = gsalloc (ccmd.p, ccmd.l *= LPARAM, YES);
    }
    /* arg = rest of "word" */
    if (len)
	move (cp, ccmd.p, (Uint) len);
    sfree (cp);
    ccmd.p[len] = '\0';
    ccmd.len = len;
    paramv = ccmd.p;
}

/*
    Get the argument from wksp [line, col]
    pktype = PK_WHOLE -     вся строка
    pktype = PK_IDENT -     идентификатор
    pktype = PK_UPTOS -     до пробела
    pktype = PK_FIELD -     до двоеточия
    Return pointer to string within cline.
    Return length of string in len.
*/
char *
pkarg (wksp, line, col, len, pktype)
Reg4 S_wksp *wksp;
Reg5 Nlines line;
Reg3 Ncols col;
int *len;
Small pktype;
{
    Reg6 La_stream *olas;
    Reg2 int rlen;
    Reg1 Uchar *cp;
    char *ckl;

    olas = curlas;
    curlas = &wksp->las;
    getline (line);
    curlas = olas;

    if (col >= ncline - 1)
	return (char *) NULL;

    while (cline[col] == ' ')
	col++;
    if (pktype == PK_IDENT) {   /* Немного назад */
	while (col - 1 > 0) Block {
	    char ch;

	    ch = cline[col - 1];
	    if (ch != '_' && !(isalnum (ch)
#ifndef CTYPE
		||
#ifdef  CHANGECTYPE
		!correctype &&
#endif
		isrus8 (ch)
#endif
	       ))
		break;
	    col--;
	}
    }
    if (cline[col] == '\n')
	return (char *) NULL;

    if ((ckl = salloc ((int) (ncline - col + 1), NO)) == (char *) NULL)
	return (char *) NULL;

    for (rlen = 0, cp = (Uchar *) ckl; ; rlen++, cp++) {
	*cp = cline[col + rlen];
	if (*cp == '\n')
	    break;
	switch (pktype) {
	case PK_WHOLE:
	    continue;
	case PK_UPTOS:
	    if (isspace (*cp))
		break;
	    /* Fall through */
	case PK_FIELD:
	    if (*cp == ':')
		break;
	    continue;
	case PK_IDENT:
	    if (*cp != '_'
		 && (!(isalnum (*cp)
#ifndef CTYPE
		       ||
#ifdef  CHANGECTYPE
		       !correctype &&
#endif
		       isrus8 (*cp)
#endif
		      )
		     || rlen == 0 && isdigit (*cp)))
		break;
	    continue;
	}
	break;
    }
    *len = rlen;

    return ckl;
}

static
void
parputch (chr)
Reg1 Uchar chr;
{
    if (cursorcol > curwin->redit || cursorline > curwin->bedit ||
	cursorcol < curwin->ledit + 5 || cursorline < curwin->tedit)
	poscursor (min (curwin->ledit + 5, curwin->redit), curwin->tedit); /* CMD: */
    if (cursorcol + 1 > lastcol)
	lastcol = cursorcol + 1;
    putch (U(chr), NO);
}
