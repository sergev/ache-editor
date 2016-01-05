/*    cursor marking */

#include "e.h"
#include "e.mk.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.tt.h"

extern void markprev ();

/*    Mark this spot. */
void
mark ()
{
    if (prevmark == (struct markenv *) NULL) {
	cmarkleft = cmarkright = -1;
	cmarktop = cmarkbot = -1;
	if (!HelpActive) {
	    d_put (VCCMD);
	    info (inf_mark, 4, "MARK", NO);
	}
    }
    if (curmark)
	loopflags.flash = YES;
    markprev ();
    exchmark (YES);
    updatemark ();
}

/*    Remove marks, if any. */
void
unmark ()
{
    if (curmark) {
	cmarktop = topmark (cursorline);
	cmarkbot = botmark (cursorline);
	cmarkleft = leftmark (cursorcol);
	cmarkright = rightmark (cursorcol);
	if (!HelpActive) {
	    info (inf_mark, 4, "", NO);
	    info (inf_area, infoarealen, "", NO);
	}
	curmark = (struct markenv *) NULL;
	updatemark ();
    }
    prevmark = (struct markenv *) NULL;
    infoarealen = 0;
    marklines = 0;
    markcols = 0;
    mklinstr[0] = 0;
    mkcolstr[0] = 0;
}

/*
    Go to upper mark.
    if leftflg is YES, go to upper left corner
    else if lines must be exchanged, then exchange columns also
    else don't exchange columns
*/
Flag
gtumark (leftflg)
Flag leftflg;
{
    Reg1 Short tmp;
    Flag exchlines = NO;
    Flag exchcols  = NO;

    /* curmark is the OTHER corner */
    markprev ();

    if (    curmark->mrkwinlin + curmark ->mrklin
	 < prevmark->mrkwinlin + prevmark->mrklin
       )
	exchlines = YES;
    if (   (   leftflg
	    &&    curmark->mrkwincol + curmark ->mrkcol
	       < prevmark->mrkwincol + prevmark->mrkcol
	   )
	|| (!leftflg && exchlines)
       )
	exchcols = YES;
    if (exchlines || exchcols) {
	if (!exchcols) {
	    tmp = curmark->mrkwincol;
	    curmark->mrkwincol = prevmark->mrkwincol;
	    prevmark->mrkwincol = tmp;
	    tmp = curmark->mrkcol;
	    curmark->mrkcol = prevmark->mrkcol;
	    prevmark->mrkcol = tmp;
	}
	else if (!exchlines) {
	    tmp = curmark->mrkwinlin;
	    curmark->mrkwinlin = prevmark->mrkwinlin;
	    prevmark->mrkwinlin = tmp;
	    tmp = curmark->mrklin;
	    curmark->mrklin = prevmark->mrklin;
	    prevmark->mrklin = tmp;
	}
	return exchmark (NO) & WINMOVED;
    }
    return 0;
}

/*    Exchange the two marked positions. */
Small
exchmark (puflg)
Flag puflg;
{
    Reg2 struct markenv *tmp;
    Short retval = 0;

    tmp = curmark;
    if (curmark) Block {
	Reg3 Nlines lin = curmark->mrkwinlin + curmark->mrklin;
	Reg5 Ncols  col = curmark->mrkwincol + curmark->mrkcol;
	Reg1 Nlines winlin = curwksp->wlin;
	Reg6 Ncols  wincol = curwksp->wcol;
	Reg4 Nlines cl;
	Reg7 Ncols  cc;

	if (lin >= winlin &&
	    lin <= winlin + curwin->btext)
	    cl = lin - winlin;
	else {
	    cl = curmark->mrklin;
	    winlin = curmark->mrkwinlin;
	}
	if (col >= wincol &&
	    col <= wincol + curwin->rtext)
	    cc = col - wincol;
	else {
	    cc = curmark->mrkcol;
	    wincol = curmark->mrkwincol;
	}
	curmark = prevmark;
	prevmark = tmp;
	return movewin (winlin, wincol, cl, cc, puflg);
    }
    curmark = prevmark;
    prevmark = tmp;
    return retval;
}

/*    Copy the current position into the prevmark structure. */
void
markprev ()
{
    static struct markenv mk1, mk2;

    if (prevmark == (struct markenv *) NULL)
	prevmark = curmark == &mk1 ? &mk2 : &mk1;
    prevmark->mrkwincol = curwksp->wcol;
    prevmark->mrkwinlin = curwksp->wlin;
    prevmark->mrkcol = cursorcol;
    prevmark->mrklin = cursorline;
}

markrect (blin, bcol, elin, ecol)
Nlines blin;
Nlines elin;
Ncols bcol;
Ncols ecol;
{
    if (curmark)
	unmark ();
    gotomvwin (blin, bcol);
    markprev ();
    exchmark (YES);
    gotomvwin (elin, ecol);
    markprev ();
    exchmark (YES);
    cmarkleft = cmarkright = -1;
    cmarktop = cmarkbot = -1;
    updatemark ();
}
