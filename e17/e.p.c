/*    Process printing characters */

#include "e.h"
#include "e.cc.h"
#include "e.cm.h"
#ifdef UNIXV7
#include <ctype.h>
#endif

static void shortencline ();
extern Scols putupdelta;
#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
extern Echar CurrFont;

Small
printchar ()
{
    Reg1 int ch;
    Reg2 Small ret;

    if (!okwrite ())
	return NOWRITERR;

    if (!cyrillflg && isrus8 (key)) {
	ch = tomap8 (key); /* Only for KOI8 */
	key = CYRILLCH;
	if ((ret = printnormchar (NO)) != CONTIN)
	    return ret;
	key = ch;
    }
    if (uppercaseflg && isprint (key)
	&& (ch = changechar(key, YES)) != '\0') {
	key = BEFORECH;
	if ((ret = printnormchar (NO)) != CONTIN)
	    return ret;
	key = ch;
    }
    return printnormchar (YES);
}

/*
    Process a printing char for mainloop ().
*/
static
Small
printnormchar (Last)
Flag Last;
{
    Reg1 Ncols curcol;
    Reg2 Nlines ln;
    Reg3 Flag bkspflg;
    Reg4 Flag inschar;

    bkspflg = NO;
    inschar = NO;

    if (key != CCBACKSPACE && cursorcol > curwin->rtext - defrwin / 3)
	movecursor (RT, 0);

    getline (ln = curwksp->wlin + cursorline);

    curcol = cursorcol + curwksp->wcol;
    if (key == CCDELCH || key == CCBACKSPACE) {
	if (key == CCBACKSPACE) {
	    if (cursorcol == 0)
		return CONTIN;
	    movecursor (LT, 1);
	    curcol--;
	    bkspflg = YES;
	}

	if (curcol >= ncline - 1)    /*  \n at end of cline */
	    return CONTIN;

	if (   key == CCDELCH
	    || insmode
	   ) Block {
	    Slines thislin = cursorline;

	    if (ncline - 2 - curcol > 0)
		move ((char *) &cline[curcol + 1], (char *) &cline[curcol],
		      (Uint) (ncline - 2 - curcol) * sizeof (Echar));
	    ncline--;
	    curcol -= curwksp->wcol;
	    shortencline ();
	    putupdelta = -1;
	    putup (-1, cursorline, (Scols) curcol, MAXWIDTH);
	    poscursor ((Scols) curcol, thislin);
	    fcline = YES;

	    numtyp++;             /*  modified text   */

	    return CONTIN;
	}
	key = ' ';
    }
    else if (key == CCCTRLQUOTE) {
	key = ESCCHAR;
	ecline = YES;
    }
    else if (key == CCINSCH) {
	inschar = YES;
	key = ' ';
    }
    else if (Last && key != ' ')
	key |= CurrFont;

    if (curcol >= lcline - 2 && key != ' ')
	excline (curcol + 2);
    if (curcol >= ncline - 1) {
	if (inschar)
	    return CONTIN;
	if (   xcline
	    && !extend (ln - la_lsize (curlas) + 1)
	   ) {
	    mesg (ERRALL + 1, ediag("Can't extend the file",
				"Нельзя расширить файл"));
	    return CONTIN;
	}
	if (key != ' ') Block {
	    Short ii;

	    for (ii = 0; ii < curcol + 2 - ncline; ii++)
		cline[ncline - 1 + ii] = ' ';
	    cline[curcol] = key;
	    ncline = curcol + 2;
	    cline[ncline - 1] = '\n';
	}
	if (xcline) Block {
	    Scols thiscol = cursorcol;
	    Slines thisline = cursorline;

	    xcline = 0;
	    putup (-1, cursorline, cursorcol, MAXWIDTH);
	    poscursor (thiscol + 1, thisline);
	}
	else
	    putch (key, YES);
    }
    else {
	if (insmode || inschar) {
	    Slines thisline = cursorline;

	    if (ncline >= lcline)
		excline ((Ncols) 0);
	    move ((char *) &cline[curcol], (char *) &cline[curcol + 1],
		  (Uint) (ncline - curcol) * sizeof (Echar));
	    ncline++;
	    cline[curcol] = key;
	    curcol -= curwksp->wcol;
	    putupdelta = 1;
	    putup (-1, cursorline, (Scols) curcol, MAXWIDTH);
	    if (!inschar)
		curcol++;
	    poscursor ((Scols) curcol, thisline);
	}
	else {
	    cline[curcol] = key;
	    putch (key, YES);
	}
	shortencline ();
    }

    curwin->redit = curwin->rtext;

    fcline = YES;
    if (bkspflg)
	movecursor (LT, 1);

    if (curwksp->wcol + cursorcol == linewidth) {
	 bell ();
	 /*movecursor (RN, 1);*/
    }

    numtyp++;             /*  modified text   */

    return CONTIN;
}

char
changechar (chr, inp)
Reg1 Echar chr;
Reg2 Flag inp;
{
    Reg3 int i1;

    if (!isuppercase)
	return 0;

    chr = U(chr);

    if (!isprint (chr))
	return 0;

    for (i1 = 0; i1 < NSPCHARS; i1++)
	if (chr == cspchars[inp][i1])
	    return cspchars[!inp][i1];

    if ((inp ? isupper (chr): islower (chr)))
	return U(inp ? tolower (chr) : toupper (chr));

    return 0;
}

Short
chanchcnt (str)
char *str;
{
    Reg1 char *s;
    Reg2 Short cnt;
    Reg3 Short c;

    for (s = str, cnt = 0; *s; s++) {
	c = U(*s);
	if (isprint (c)
#ifndef  CTYPE
	    ||
#ifdef  CHANGECTYPE
	    !correctype &&
#endif
	    isrus8 (c)
#endif
	   ) {
	    if (uppercaseflg && changechar (c, YES) != '\0')
		cnt++;
	    if (!cyrillflg && isrus8 (c))
		cnt++;
	}
	else if (c < ESCCHAR)
	    cnt++;
	else
	    cnt += 3;
	cnt++;
    }
    return cnt;
}

/*
    Shorten cline if necessary so that there are no trailing blanks or tabs,
    and make sure cline[ncline -1] = '\n';
*/
static
void
shortencline ()
{
    Reg1 Echar *cp;
    Reg2 Echar *rcline;

    if (binary || ncline <= 1)
	return;
    rcline = cline;
    for (cp = &rcline[ncline - 2]
	; cp >= rcline && *cp == ' '
	; cp--
	)
	continue;
    *++cp = '\n';
    ncline = cp - rcline + 1;
}
