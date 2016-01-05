/*    The editing functions that do open, close, pick, put, erase, etc. */

#include "e.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.cm.h"
#include "e.e.h"

static void putbks ();
extern void clean ();
static void domark ();
static void boxseg ();
static Cmdret splitmark ();
static Cmdret joinmark ();
Cmdret edmark ();
static Cmdret edrect ();
static Cmdret edlines ();
static Cmdret lacmderr ();

/*
    Do the 'split' command.
    If marks,
	Split the line at the upper mark and put the remainder of the line
	at the lower mark, inserting blank lines if necesary.
    else
	Split the line at current col and move the remainder of the line to
	the beginning of the next line.
*/
Cmdret
split ()
{
    Reg1 Ncols k;
    Reg2 Nlines line;
    Reg3 Ncols col;

    if (!okwrite ())
	return NOWRITERR;
    if (curmark)
	return splitmark ();

    line = curwksp->wlin + cursorline;
    col = curwksp->wcol + cursorcol;
    if (offsetflg && line < la_lsize (curlas)) {
	getline (line);
	if ((k = fnbcol (cline, 0, ncline)) < 0)
	    k = 0;
	if (col < ncline - 1)
	    while (U(cline[col]) == ' ')
		col++;
    }
    else
	k = 0;

    return splitlines (line, col, (Nlines) 1, k, YES);
}

/*
    Split the line at the upper mark and put the remainder of the line
    at the lower mark, inserting blank lines if necesary.
*/
static
Cmdret
splitmark ()
{
    Reg1 Cmdret retval;
    Reg2 Flag moved;

    moved = gtumark (NO);
    retval = splitlines (topmark (cursorline),
			 curwksp->wcol + cursorcol,
			 marklines - 1,
			 curmark->mrkwincol + curmark->mrkcol,
			 !moved);
    domark (moved);
    return retval;
}

/*
    Split the line at col and move the remainder of the line to line+nl
    starting at newc.
    Update the display if puflg != 0.
*/
Cmdret
splitlines (line, col, nl, newc, puflg)
Reg1 Nlines  line;
Reg2 Ncols   col;
Reg3 Nlines  nl;     /* must be >= 1 */
Ncols   newc;
Flag    puflg;
{
    if (line >= la_lsize (curlas))
	return CROK;

    getline (line);     /* must do this to set ncline */
    if (col >= ncline - 1)
	return CROK;

    /* append or insert nl blank lines after line */
    getline (-1);
    if (line == la_lsize (curlas) - 1) {
	if (!extend (nl)) {
	    mesg (ERRALL + 1, ediag("Can't extend the file",
				    "Нельзя расширить файл"));
	    goto ret;
	}
    }
    else {
	(void) la_lseek (curlas, line + 1, 0);
	if (la_blank (curlas, nl) != nl) {
	    mesg (ERRALL + 1, ediag("Can't make file that long",
				    "Нельзя сделать такой длинный файл"));
	    goto ret;
	}
    }
    /* shorten the first line */
    getline (line);
    Block {
	Reg4 Ncols  nsave;
	Reg5 Echar csave;

	csave = cline[col];
	cline[col] = '\n';
	nsave = ncline;
	ncline = col + 1;
	fcline = YES;
	putline (NO);
	/* make the second line */
	cline[col] = csave;
	move ((char *) &cline[col], (char *) cline,
	      (Uint) (ncline = nsave - col) * sizeof (Echar));
	fcline = YES;
    }
    putbks ((Ncols) 0, newc);
    numtyp += ncline * 2 + newc;
    clinelas = curlas;
    clineno = line + nl;
    redisplay (curfile, line, (Nlines) 1, (Nlines) 0, puflg);
    redisplay (curfile, line + 1, (Nlines) 0, (Nlines) nl, puflg);

 ret:
    return CROK;
}

/*
    Do the 'join' command.
    If marks,
	Join two lines, and delete all text in between.  Use the upper and
	lower marks as the join points.  Put deleted text into the join
	buffer.
    else
	The join command looks forward starting at the beginning of the
	next line for the first non-blank character in the file and joins
	that and the rest of its line to the end of the current line.
	If the cursor position is beyond the end of the current line the
	join point is the cursor position, else it is one space beyond the
	end of the current line.  In no case is any printing text deleted.
	The join buffer is unaffected.
*/
Cmdret
join ()
{
    if (!okwrite ())
	return NOWRITERR;

    if (curmark)
	return joinmark ();

    return joinlines (curwksp->wlin + cursorline,
		      curwksp->wcol + cursorcol,
		      YES);
}

/*
    Join two lines, and delete all text in between.  Use the upper and lower
    marks as the join points.
*/
static
Cmdret
joinmark ()
{
    Reg1 Cmdret retval;
    Reg2 Flag moved;

    moved = gtumark (NO);
    retval = dojoin (topmark (cursorline),
		     curwksp->wcol + cursorcol,
		     marklines - 1,
		     curmark->mrkwincol + curmark->mrkcol,
		     YES,
		     !moved);
    domark (moved);
    return retval;
}

/*
    See join() above.
*/
Cmdret
joinlines (line, col, puflg)
Reg3 Nlines  line;
Ncols   col;
Flag    puflg;
{
    Reg2 Nlines ln;
    Ncols newc;

    getline (line);
    if (col < ncline - 1)
	col = ncline - 1;
    for (ln = line + 1; ;ln++) Block {
	Reg1 Echar *cp;

	getline (ln);
	if (xcline)
	    return CROK;
	for (cp = cline; U(*cp) == ' ' || U(*cp) == '\t'; cp++)
	    continue;
	if (U(*cp) != '\n') {
	    newc = cp - cline;
	    break;
	}
    }
    return dojoin (line, col, ln - line, newc, NO, puflg);
}

/*
    Join two lines, and delete all text in between.  Use [line,col] and
    [line+nl,newc] as the join points.
    Save the end of the last line into an array.
    If saveflg != 0
	Delete the end of the last line and set fcline.
	Save the beginning of the first line into an array,
	delete the beginning of the line, and set fcline.
	Pick all the lines into the join buffer, so the text
	from the end of the top line and the beginning of the bottom line
	are saved as whole lines and everything in between is saved in
	between.
	Replace the first line with the concatenation of the two
	strings.
    else
	Tack it onto the end of the first line, and set fcline.
    Close the remaining lines without putting them into any qbuffer.
*/
Cmdret
dojoin (line, col, nl, newc, saveflg, puflg)
Nlines  line;
Reg3 Ncols   col;
Reg4 Nlines  nl;
Ncols   newc;
Flag    saveflg;
Flag    puflg;
{
    getline (-1);
    Block {
	Reg1 Nlines lsize;

	if (line >= (lsize = la_lsize (curlas)))
	    return CROK;
	if (nl > lsize - line)
	    nl = lsize - line;
    }

    Block {
	Echar *l1;
	Echar *l2;
	Reg1 Ncols nl1;
	Reg2 Ncols nl2;
	Ncols ii;

	/* Save the end of the last line into an array. */
	getline (line + nl);
	if ((nl2 = ncline - newc) > 1) {
	    /* get the end of the lower line */
	    l2 = (Echar *) salloc ((int) nl2 * sizeof (Echar), YES);
	    move ((char *) &cline[newc], (char *) l2,
		  (Uint) nl2 * sizeof (Echar));
	}
	if (saveflg) {
	    /* Delete the end of the last line and set fcline. */
	    cline[newc] = '\n';
	    ncline = newc + 1;
	    fcline = YES;
	    /* Save the beginning of the first line into an array, */
	    getline (line);
	    if ((nl1 = min (col, ncline - 1)) > 0) {
		l1 = (Echar *) salloc ((int) nl1 * sizeof (Echar), YES);
		move ((char *) cline, (char *) l1,
		      (Uint) nl1 * sizeof (Echar));
		/* delete the beginning of the line, and set fcline. */
		if (col < ncline - 1)
		    move ((char *) &cline[col], (char *) cline,
			  (Uint) (ncline -= col) * sizeof (Echar));
		else {
		    cline[0] = '\n';
		    ncline = 1;
		}
		fcline = YES;
	    }
	    ed (OPPICK, QCLOSE, line, (Ncols) 0, nl + 1, (Ncols) 0, NO);
	}
	getline (line);
	if (col + nl2 > lcline)
	    excline (col + nl2);
	if (saveflg) {
	    if (nl1 > 0) {
		move ((char *) l1, (char *) cline,
		      (Uint) nl1 * sizeof (Echar));
		sfree ((char *) l1);
	    }
	}
	else
	    nl1 = min (col, ncline - 1);
	for (ii = nl1; ii < col; ii++)
	    cline[ii] = ' ';
	if (nl2 > 1) {
	    move ((char *) l2, (char *) &cline[col],
		  (Uint) nl2 * sizeof (Echar));
	    sfree ((char *) l2);
	}
	else
	    cline[col] = '\n';
	ncline = col + nl2;
    }
    fcline = YES;
    redisplay (curfile, line, (Nlines) 1, (Nlines) 0, puflg);
    /* close line + 1 thru line + nl into the 'join' buffer */
    savecurs ();
    (void) ed (OPCLOSE, QNONE, line + 1, (Ncols) 0, nl, (Ncols) 0, puflg);
    restcurs ();
    return CROK;
}

/*
    Do a pick, close, erase, etc. as chosen by 'which' on the current area.
*/
Cmdret
areacmd (which)
Reg2 Small which;
{
    static short  opc[5] = {OPPICK, OPCLOSE, OPERASE, OPOPEN, OPBOX};
    static ASmall buf[5] = {QPICK, QCLOSE, QERASE};

    if (opstr[0] == '\0') {
	if (curmark)
	    return edmark (opc[which], buf[which]);
	else
	    return ed (opc[which], buf[which], curwksp->wlin + cursorline,
		       (Ncols) 0, (Nlines) 1, (Ncols) 0, YES);
    }
    if (curmark)
	return NOMARKERR;
    if (*nxtop)
	return CRTOOMANYARGS;

    Block {
	char *str;

	str = opstr;
	switch (getpartype (&str, YES, NO, curwksp->wlin + cursorline)) {
	case 1: Block {
		Reg1 char *cp;

		for (cp = str; *cp && *cp == ' '; cp++)
		    continue;
		if (*cp)
		    goto badarg;
	    }
	    return ed (opc[which], buf[which], curwksp->wlin + cursorline,
		       (Ncols) 0, parmlines, (Ncols) 0, YES);

	case 2:
	    return ed (opc[which], buf[which], curwksp->wlin + cursorline,
		       curwksp->wcol + cursorcol, parmlines, parmcols, YES);

	case 3:
	    if (which != 0) {
		break;
	    }
	badarg:
	default:
	    mesg (ERRSTRT + 1, opstr);
	    return CRUNRECARG;
	}
    }
    return CROK;
}

/*
    Edit the marked area.  Opc and buf tell which operation and which qbuffer
    to use.
*/
Cmdret
edmark (opc, buf)
Small opc;
Small buf;
{
    Cmdret retval;
    Reg1 Flag moved;

    moved = gtumark (YES);
    retval = ed (opc, buf, topmark (cursorline),
		 markcols ? leftmark (cursorcol) : (Nlines) 0,
		 marklines, markcols, !moved);
    domark (moved);
    return retval;
}

/*    common code from the end of splitmark, etc. */
static
void
domark (moved)
Reg1 Flag moved;
{
    if (moved)
	putupwin ();
    unmark ();
}

/*
    Edit the specified area.  Opc and buf tell which operation and which
    qbuffer to use.
.
    OPPICK:     buf (to)
    OPCLOSE:    buf (to)
    OPERASE:    buf (to)
    OPCOVER:    buf (from)
    OPOVERLAY:  buf (from)
    OPUNDERLAY: buf (from)
    OPBLOT:     buf (from)
    OP_BLOT:    buf (from)
    OPINSERT:   buf (from)
    OPOPEN:      X
    OPBOX:       X
.
    'buf' can be QNONE for OPCLOSE only.
*/
Cmdret
ed (opc, buf, line, col, nlines, ncols, puflg)
Small opc;
Small buf;
Nlines line;
Ncols col;
Nlines nlines;
Ncols ncols;
Flag puflg;
{
    Nlines lsize;
    Nlines endgap;

    if (   !(opc & OPPICK)
	&& !okwrite ()
       )
	return NOWRITERR;

    if (   (opc & OPBOX)
	&& ncols == 0
       )
	return NOTRECTERR;

    if (opc & OPQFROM) {
	/* buf can NOT be QNONE */
	if (buf == QBOX) {
	    if (nlines <= 0 || ncols <= 0)
		return NOTRECTERR;
	}
	else {
	    if ((nlines = la_reserved (&qbuf[buf].buflas)) == 0)
		return NOBUFERR;
	    ncols = qbuf[buf].ncols;
	}
    }

    getline (-1);
    endgap = line - (lsize = la_lsize (curlas));
    if (opc & (OPQFROM | OPBOX)) {
	if (   endgap > 0
	    && !extend (endgap)
	   ) {
	    mesg (ERRALL + 1, ediag("Can't extend the file",
				    "Нельзя расширить файл"));
	    return CROK;
	}
    }
    else { /* opc & (OPCLOSE | OPPICK | OPERASE | OPOPEN) */
	if ((opc & OPQTO) && ncols == 0)
	    nlines = min (nlines, lsize - line);
	if (endgap >= 0) {
	    if ((opc & OPQTO) && buf != QNONE)
		la_setrlines (&qbuf[buf].buflas, (Nlines) 0);
	    return CROK;
	}
    }

    if (ncols > 0)
	return edrect (opc, buf, line, col, nlines, ncols, puflg);
    else
	return edlines (opc, buf, line, nlines, puflg);
}

/*
    Edit the specified lines.  Opc and buf tell which operation and which
    qbuffer to use.
.
    OPPICK:     buf (to)
    OPCLOSE:    buf (to)
    OPERASE:    buf (to)
    OPCOVER:    buf (from)
    OPOVERLAY:  buf (from)
    OPUNDERLAY: buf (from)
    OPBLOT:     buf (from)
    OP_BLOT:    buf (from)
    OPINSERT:   buf (from)
    OPOPEN:      X
    OPBOX:       X
.
    'buf' can be QNONE for OPCLOSE only.
*/
static
Cmdret
edlines (opc, buf, line, nlines, puflg)
Small opc;
Small buf;
Nlines line;
Nlines nlines;
Flag puflg;
{
    La_stream *dlas;    /* where to put the deleted lines */
    La_stream *blas;    /* the Q buffer */
    La_bytepos oldbsize;

    if (buf != QNONE) {
	blas = &qbuf[buf].buflas;
	if (opc & OPQTO) {
	    dlas = &fnlas[qtmpfn[buf]];
	    la_stayset (blas);
	    qbuf[buf].ncols = 0;
	}
    }

    (void) la_lseek (curlas, line, 0);

    oldbsize = la_bsize (curlas);

    if (opc & OPCLOSE) {
	if (   curfile == OLDLFILE
	    || buf == QNONE
	   ) {
	    if (la_ldelete (curlas, nlines, (La_stream *) NULL) != nlines) {
 err1:
		la_stayclr (blas);
		numtyp += abs (la_bsize (curlas) - oldbsize);
		return lacmderr ("edlines");
	    }
	}
	else Block {
	    Nlines nn;

	    clean (OLDLFILE);
	    nn = la_lsize (dlas);
	    (void) la_align (dlas, blas);
	    if (la_ldelete (curlas, nlines, dlas) != nlines)
		goto err1;
	    la_setrlines (blas, nlines);
	    redisplay (OLDLFILE, nn, (Nlines) 0, nlines, YES);
	}
	redisplay (curfile, line, (Nlines) 0, -nlines, puflg);
    }
    else if (opc & OPPICK) {
	if (curfile == PICKFILE)
	    (void) la_align (curlas, blas);
	else Block {
	    Nlines nn;

	    clean (PICKFILE);
	    (void) la_align (dlas, blas);
	    nn = la_lsize (dlas);
	    if (la_lcopy (curlas, dlas, nlines) != nlines)
		goto err1;
	    redisplay (PICKFILE, nn, (Nlines) 0, nlines, YES);
	}
	la_setrlines (blas, nlines);
    }
    else if (opc & OPERASE) {
	if (curfile == OLDLFILE) {
	    if (   la_ldelete (curlas, nlines, (La_stream *) NULL) != nlines
		|| la_blank (curlas, nlines) != nlines
	       )
		goto err1;
	}
	else Block {
	    Nlines nn;

	    clean (OLDLFILE);
	    nn = la_lsize (dlas);
	    (void) la_align (dlas, blas);
	    if (la_ldelete (curlas, nlines, dlas) != nlines)
		goto err1;
	    if (la_blank (curlas, nlines) != nlines) {
		(void) la_ldelete (dlas, nlines, curlas);
		goto err1;
	    }
	    redisplay (OLDLFILE, nn, (Nlines) 0, nlines, YES);
	    la_setrlines (blas, nlines);
	}
	redisplay (curfile, line, nlines, (Nlines) 0, puflg);
    }
    else if (opc & OPOPEN) {
	if (la_blank (curlas, nlines) != nlines) {
 err2:
	    return lacmderr ("edlines");
	}
	redisplay (curfile, line, (Nlines) 0, nlines, puflg);
    }
    else if (opc & OPQFROM) {
#ifdef DEBUGDEF
	dbgpr ("Put attempt: ln=%d rlines=%d putline=%d\n",
	    la_linepos (blas),
	    nlines,
	    line);
#endif
	if (opc & OPINSERT) {
	    if (la_lcopy (blas, curlas, nlines) <= 0)
		goto err2;
	    redisplay (curfile, line, (Nlines) 0, nlines, puflg);
	}
	else {
	    /**/
	    mesg (ERRALL + 1, ediag("not implemented yet", "еще не реализовано"));
	}
    }

    if ((opc & OPQTO) && buf != QNONE)
	la_stayclr (blas);

    numtyp += abs (la_bsize (curlas) - oldbsize);

    return CROK;
}

/*
    Edit the specified rectangle.  Opc and buf tell which operation and which
    qbuffer to use.  See 'ed()'.
.
    OPPICK:     buf (to)
    OPCLOSE:    buf (to)
    OPERASE:    buf (to)
    OPCOVER:    buf (from)
    OPOVERLAY:  buf (from)
    OPUNDERLAY: buf (from)
    OPBLOT:     buf (from)
    OP_BLOT:    buf (from)
    OPINSERT:   buf (from)
    OPOPEN:      X
    OPBOX:       X
*/
static
Cmdret
edrect (opc, buf, line, col, nlines, ncols, puflg)
Small opc;
Small buf;
Nlines line;
Ncols col;
Nlines nlines;
Ncols ncols;
Flag puflg;
{
    Reg5 Echar *linebuf;
    La_stream *dlas;
    La_stream *blas;
    La_stream *olas;
    La_stream qlas;
    Nlines bufline;
    Fn buffn;
    Cmdret errval;
    Ncols ii;

    if (   (opc & (OPCLOSE | OPPICK | OPERASE | OPOPEN))
	&& (ncols <= 0 || nlines <= 0)
       )
	return CROK;

    if (!(opc & OPPICK))
	clean (OLDLFILE);

    blas = &qbuf[buf].buflas;
    if (opc & OPQTO) {
	if ((buffn = qtmpfn[buf]) == PICKFILE)
	    clean (PICKFILE);
	dlas = &fnlas[buffn];
	(void) la_align (dlas, blas);
	la_stayset (blas);
    }

    if ((opc & OPQFROM) && buf != QBOX) {
	if (!la_clone (blas, &qlas))
	    fatal (FATALBUG, ediag("La_clone failed in edrect()",
				   "la_clone свалилась в edrect()"));
	bufline = la_lseek (&qlas, (Nlines) 0, 1);
	nlines = la_reserved (blas);
	ncols = qbuf[buf].ncols;
	olas = curlas;
    }

    errval = CROK;
    if (!(opc & (OPCLOSE | OPBOX))) {
	if ((linebuf = (Echar *) salloc (
	     (Uint) (ncols + 1) * sizeof (Echar), NO)) == (Echar *) NULL) {
	    errval = NOMEMERR;
	    goto err3;
	}
    }

    /* first write out lines to qbuf */
    if (opc & OPQTO) Block {
	static Nlines zerolines = 0;
	Reg3 Nlines itmp;
	Reg4 Nlines nn;
	int collect;

	collect = YES;
	for (itmp = 0; itmp < nlines; itmp++) Block {
	    Reg1 Echar *cp;
	    Reg2 Ncols j;

	    getline (line + itmp);
	    if ((j = ncline - col) > 0) {
		getline (-1);   /* Forget cline */
		if ((j = min (j, ncols)) > 0)
		    move ((char *) (cline + col), (char *) cline,
			  (Uint) j * sizeof (Echar));
		cline[j] = '\n';
		j = dechars (); /* May reset deline */
		nn = la_lcollect (collect, deline, (int) j);
	    }
	    else
		nn = la_lcollect (collect, "\n", 1);
	    if (nn < 0) {
		nlines = 0;
 err1:          errval = lacmderr ("edrect");
		goto err2;
	    }
	    collect = NO;
	}
	nn = la_lsize (&fnlas[buffn]);
	if (la_lrcollect (dlas, &zerolines) != nlines) {
	    nlines = 0;
	    goto err1;
	}
	else
	    redisplay (buffn, nn, (Nlines) 0, nlines, YES);
	la_stayclr (blas);
	la_setrlines (blas, nlines);
	qbuf[buf].ncols = ncols;
    }

    /* now to modify the lines in curfile */
    if (!(opc & OPPICK)) Block {
	Reg4 Nlines itmp;
	Nlines nn;
	Ncols endcol;
	int collect;
	Nlines ndel;

	numtyp += nlines * ncols;
	endcol = col + ncols;
	collect = YES;
	for (itmp = 0; itmp < nlines; itmp++) {
	    if (opc & OPQFROM) {
		if (buf != QBOX) Block {
		    Reg1 Ncols j;

		    curlas = &qlas;
		    getline (bufline + itmp);
		    curlas = olas;
		    if ((j = min (ncols, ncline)) > 0)
			move ((char *) cline, (char *) linebuf,
			      (Uint) j * sizeof (Echar));
		    if ((j = ncline - 1) < ncols)
			for (ii = j; ii < ncols; ii++)
			    linebuf[ii] = ' ';
		}
		else {
		    if (   ncols > 1
			&& (itmp == 0 || itmp == nlines - 1)
		       ) Block {
			Reg1 Echar *cp;
			Short lim;

			cp = linebuf;
			if (nlines > 1) {
			    *cp++ = '+';
			    linebuf[ncols - 1] = '+';
			}
			lim = (nlines > 1) ? ncols - 2 : ncols;
			for (ii = 0; ii < lim; ii++)
			    *cp++ = '-';
		    }
		    else {
			linebuf[0] = '|';
			linebuf[ncols - 1] = '|';
			for (ii = 1; ii < ncols - 1; ii++)
			    linebuf[ii] = ' ';
		    }
		}
	    }

	    getline (line + itmp);
	    getline (-1);  /* because dechars on cline */

	    if (   (opc & (OPQFROM | OPBOX))
		|| col < ncline
	       ) {
		if (opc & (OPLENGTHEN)) {
		    putbks (col, ncols);
		    fcline = NO;
		}
		else {
		    if (endcol >= ncline) {
			if (opc & (OPCLOSE | OPERASE))
			    ncline = col + 1;
			else {
			    putbks ((Ncols) (ncline - 1),
				    endcol + 1 - ncline);
			    fcline = NO;
			}
		    }
		    else {
			/* new end will be before current end of line */
			if (opc & OPCLOSE) {
			    /* copy down rest of line */
			    move ((char *) &cline[endcol], (char *) &cline[col],
				  (Uint) (ncline - endcol) * sizeof (Echar));
			    ncline -= ncols;
			}
			else if (opc & OPERASE)
			    for (ii = 0; ii < ncols; ii++)
				cline[col + ii] = ' ';
		    }
		}
		if (opc & OPQFROM) {
		    if (opc & (OPOVERLAY | OPUNDERLAY | OPBLOT | OP_BLOT)) Block {
			Reg1 Ncols j;
			Reg2 Echar *cpfrom;
			Reg3 Echar *cpto;

			cpfrom = linebuf;
			cpto = &cline[col];
			for (j = 0; j < ncols; j++, cpfrom++, cpto++) {
			    switch (opc) {
			    case OPBLOT:
				if (U(*cpfrom) != ' ')
				    *cpto = ' ';
				continue;
			    case OP_BLOT:
				if (U(*cpfrom) == ' ')
				    *cpto = ' ';
				continue;
			    case OPOVERLAY:
				if (U(*cpfrom) == ' ')
				    continue;
				break;
			    case OPUNDERLAY:
				if (U(*cpto) != ' ')
				    continue;
				break;
			    }
			    if (buf == QBOX)
				boxseg (cpto, *cpfrom, (Ncols) 1);
			    else
				*cpto = *cpfrom;
			}
		    }
		    else
			move ((char *) linebuf, (char *) &cline[col],
			      (Uint) ncols * sizeof (Echar));
		}
		else if (opc & OPBOX) Block {
		    Reg1 Echar *cpto;

		    cpto = &cline[col];
		    if (   ncols > 1
			&& (itmp == 0 || itmp == nlines - 1)
		       ) {
			if (nlines != 1) {
			    *cpto++ = '+';
			    cpto[ncols - 2] = '+';
			}
			boxseg (cpto, '-', nlines == 1 ? ncols : ncols - 2);
		    }
		    else {
			boxseg (cpto, '|', (Ncols) 1);
			boxseg (&cpto[ncols - 1], '|', (Ncols) 1);
		    }
		}
		cline[ncline - 1] = '\n';
	    }
	    Block {
		Ncols len;

		len = dechars (); /* May reset deline */
		if (la_lcollect (collect, deline, (int) len) < 0)
		    goto err1;
	    }
	    collect = NO;
	}
	nn = la_lsize (&fnlas[OLDLFILE]);
	(void) la_lseek (curlas, line, 0);
	ndel = nlines;
	if ((itmp = la_lrcollect (curlas, &ndel, &fnlas[OLDLFILE])) != nlines) {
	    nlines = 0;
	    goto err2;
	}
	else {
	    redisplay (curfile, line, nlines, (Nlines) 0, puflg);
	    if (opc & OPQTO)
		redisplay (OLDLFILE, nn, (Nlines) 0, nlines, YES);
	}
    }

 err2:
    if (!(opc & (OPCLOSE | OPBOX)))
	sfree ((char *) linebuf);
 err3:
    if ((opc & OPQFROM) && buf != QBOX)
	(void) la_close (&qlas);

    return errval;
}

/*
    Inserts n blanks starting at col of cline.
    Lengthens cline and fills distance from ncline to col as necessary.
*/
static
void
putbks (col, n)
Reg1 Ncols col;
Reg2 Ncols n;
{
    Reg3 Ncols ii;
    Reg4 Ncols len;

    if (n <= 0)
	return;

    fcline = YES;

    if (col >= ncline) {
	n += col - (ncline - 1);
	col = ncline - 1;
    }
    if (lcline <= (ncline += n))
	excline (n);

    len = col + n;
    if (ncline - len > 0)
	move ((char *) &cline[col], (char *) &cline[len],
	      (Uint) (ncline - len) * sizeof (Echar));
    for (ii = col; ii < len; ii++)
	cline[ii] = ' ';
}

/*
    Write num chr's at cpto.
    Chr will be a box character, i.e. '|', '-', or '+'.
    Look to see if a box character going perpendicular to chr is at each
    position, and if so, put in a '+' for a crossing.
*/
static
void
boxseg (cpto, chr, num)
Reg1 Echar *cpto;
Reg2 Echar chr;
Reg3 Ncols num;
{
    for (; --num >= 0; cpto++) {
	if (U(*cpto) != '+') {
	    if (chr == '|') {
		if (U(*cpto) == '-') {
		    *cpto = '+';
		    continue;
		}
	    }
	    else if (chr == '-') {
		if (U(*cpto) == '|') {
		    *cpto = '+';
		    continue;
		}
	    }
	    *cpto = chr;
	}
    }
}

/*    Clean up tmpfile, i.e. trim it down to free up some memory. */
void
clean (tmpfile)
Small tmpfile;
{
#ifdef lint
    if (tmpfile)
	{}
#endif
}

/*    Do the right thing depending on which LA err happened. */
static
Cmdret
lacmderr (str)
char *str;
{
    switch (la_error ()) {
    case LA_INT:
	mesg (ERRALL + 1, ediag("Operation interrupted",
				"Операция прервана"));
	return CROK;
    case LA_NOMEM:
	return NOMEMERR;
    case LA_WRTERR:
	return NOSPCERR;
    case LA_ERRMAXL:
	return TOOLNGERR;
    }
    fatal (FATALBUG, str);
    /* NOTREACHED */
}
