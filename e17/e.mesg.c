#include "e.h"
#include "e.tt.h"
#include "e.ml.h"
#include "e.mac.h"
#ifdef UNIXV7
#include <ctype.h>
#endif
#include <varargs.h>

#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
S_window *msowin;
S_wksp *msowksp;
Scols  msocol;
Slines msolin;
Scols lastcol = 0;

/*
    Put a message on the Command Line.
    The least significant 3 bits of 'parm' tell how many string arguments
    follow.  The rest of the bits are used to control the workings of
    'mesg'.
.
      TELSTRT 0010  Start a new message
      TELSTOP 0020  End of this message
      TELCLR  0040  Clear rest of Command Line after message
      TELDONE 0060  (TELSTOP | TELDONE)
      TELALL  0070  (TELSTRT | TELDONE)
      TELERR  0100  This is an error message.
    The following are analogous to TEL...
      ERRSTRT 0110
      ERRSTOP 0120            no more to write
      ERRCLR  0140            clear rest of line
      ERRDONE 0160
      ERRALL  0170
*/
/*VARARGS1*/
void
mesg (parm, va_alist)
Reg5 Small parm;
va_dcl
{
    Reg6 Small nmsg;
    Reg7 va_list msgp;
    Reg4 Flag to_image;         /* YES = to screen image, NO = putchar */

    to_image = windowsup || replaying;
    if (to_image) {
	if (parm & TELSTRT) {
	    msowin = curwin;        /* save old window info   */
	    msowksp = curwin->wksp;
	    msocol = cursorcol;
	    msolin = cursorline;
	    switchwindow (&enterwin);
	    if (cmdmode)
		poscursor (ediag (5/*CMDS: */,4/*λον: */), 0);
	    else
		poscursor ((Scols) 0, (Slines) 0);
	}
	else
	    if (curwin != &enterwin)
		return;
    }
    else if (parm & TELSTRT) {
	putchar ('\n');
	fflush (stdout);
    }

    if ((parm & ERRSTRT) == ERRSTRT) Block {
	Reg1 char *cp;

	d_put (VCCMB);
/*NOXXSTR*/
	for (cp = "\7 *** "; cursorcol < enterwin.redit && *cp; cp++)
/*YESXSTR*/
	    if (to_image)
		putch (*cp, NO);
	    else
		putchar (*cp);
    }

    if (nmsg = parm & 7) {
	va_start (msgp);
	for ( ; nmsg-- > 0; ) Block {
	    Reg2 char *cp;
	    Reg1 Uchar chr;

	    cp = va_arg (msgp, char *);
	    if (to_image) {
		for (; cursorcol < enterwin.redit && (chr = *cp); cp++) {
		    if (isprint (chr)
#ifndef  CTYPE
			||
#ifdef  CHANGECTYPE
			!correctype &&
#endif
			isrus8 (chr)
#endif
			|| chr == '\7') {
			Reg3 int ch;

			if (!cyrillflg && isrus8 (chr)) {
			    chr = tomap8 (chr);
			    putch (U(CYRILLCH), NO);
			}
			if (uppercaseflg && isprint (chr) &&
			    (ch = changechar (chr, YES)) != '\0') {
			    if (cursorcol < enterwin.redit)
				putch (U(BEFORECH), NO);
			    chr = ch;
			}
			if (cursorcol < enterwin.redit)
			    putch (chr, NO);
		    }
		    else {
			putch (U(ESCCHAR), NO);
			if (U(chr) != ESCCHAR && cursorcol < enterwin.redit)
			    putch ((chr & 037) | 0100 | (uppercaseflg ? 040 : 0), NO);
		    }
		}
	    }
	    else {
		while (*cp)
		    putchar (*cp), cp++;
	    }
	}
	va_end (msgp);
    }

    if ((parm & ERRSTOP) == ERRSTOP) {
	if (to_image) {
	    d_put (VCCMBE);
	    bell ();
	    loopflags.hold = YES;
	}
	else
	    putchar ('\7');
	if (mread) {
	    macerr (mread);
	    infomacro (NO, 0, 0);
	}
    }

    if (to_image) {
	if ((parm & TELCLR) && cursorcol < lastcol) Block {
	    Scols thiscol = cursorcol;
	    Slines thisline = cursorline;

	    multchar (' ', lastcol - cursorcol);
	    if (!(parm & TELSTOP))
		poscursor (thiscol, thisline);
	    lastcol = thiscol;
	}
	else if (cursorcol > lastcol)
	    lastcol = cursorcol;
    }

    if (parm & TELSTOP) {         /* remember last position and.. */
	if (to_image) {
	    if (curwin != msowin)
		switchwindow (msowin);
	    poscursor (msocol, msolin);
	}
	else
	    putchar (' ');
    }
    d_flush ();
}

/*
    Put 'msg' up on the Info Line at 'column'.
    'ncols' is the length of the previous message that was there.
    If 'ncols' is longer than the length of 'msg', the remainder of
    the old message is blanked out.
*/
void
info (column, ncols, msg, uppflg)
Scols column;
Reg3 Scols ncols;
Reg2 char *msg;
Flag uppflg;
{
    Reg1 Uchar chr;
    Reg4 S_window *oldwin;
    Reg5 Scols  oldcol;
    Reg6 Slines oldlin;

    oldwin = curwin;        /* save old window info   */
    oldcol = cursorcol;
    oldlin = cursorline;
    switchwindow (&infowin);
    poscursor (column, 0);
    for (; cursorcol < infowin.redit && (chr = *msg++); )
	if (isprint (chr)
#ifndef  CTYPE
	    ||
#ifdef  CHANGECTYPE
	    !correctype &&
#endif
	    isrus8 (chr)
#endif
	    || chr == '\7') Block {
	    Reg3 char ch;

	    if (!cyrillflg && isrus8 (chr)) {
		putch (U(CYRILLCH), NO);
		chr = tomap8 (chr);
		ncols--;
	    }
	    if (uppflg && uppercaseflg && isprint (chr) &&
		(ch = changechar (chr, YES)) != '\0') {
		if (cursorcol < infowin.redit) {
		    putch (U(BEFORECH), NO);
		    ncols--;
		}
		chr = ch;
	    }
	    else if (!uppflg && uppercaseflg && isupper (chr))
		chr = tolower (chr);
	    if (cursorcol < infowin.redit) {
		putch (chr, NO);
		ncols--;
	    }
	}
	else {
	    putch (U(ESCCHAR), NO);
	    ncols--;
	    if (U(chr) != ESCCHAR && cursorcol < infowin.redit) {
		putch ((chr & 037) | 0100 | (uppercaseflg ? 040 : 0), NO);
		ncols--;
	    }
	}
    d_put (VCCNRM);
    multchar (' ', ncols);
    switchwindow (oldwin);
    poscursor (oldcol, oldlin);
}

