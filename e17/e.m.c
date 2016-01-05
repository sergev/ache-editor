/*
    mainloop
*/

#include "e.h"
#include "e.cc.h"
#include "e.e.h"
#include "e.cm.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.tt.h"
#include "e.se.h"
#ifdef UNIXV7
#include <ctype.h>
#endif

extern Flag begline ();
extern Flag endline ();
extern Flag leftword ();
extern Flag rightword ();
extern Cmdret cleareol ();
extern void offsetdown ();
extern Flag imbedtag ();

extern Ncols ret_offset;
extern Nlines  infosize;       /* all lines count */
extern Scols inf_avail;
extern int curfnum;
extern int infofnum;
#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
extern void srchtag ();
#ifdef  SECURITY
int CommCnt = 0;
extern Flag nomorecommand;
extern char *violation[];
#endif

/*
    Get keys from the input and process them.
*/
void
mainloop ()
{
    extern Flag usescroll, needrest;
    Flag savesilent;

    for (;;) {
funcdone:
	loopflags.clear = YES;
newnumber:
	if (usescroll) {
	    d_flush ();
	    savesilent = silent;
	    silent = YES;
	}
	if (loopflags.clear && !loopflags.hold) {
	    mesg (TELALL + 1, loopflags.beep ? "\007" : "");
	    loopflags.beep = NO;
	}
#ifdef  SECURITY
	if (nomorecommand && ++CommCnt % 20 == 0)
	    fatal (FATALSEC, ediag (violation[0], violation[1]));
#endif
	Block {
	    Reg1 Nlines nlines;
	    Nlines all;
	    char ich[30];

	    nlines = curwksp->wlin + cursorline + 1;
	    all = la_lsize (curlas);
	    if (infoline != nlines || infosize != all) {
		(void) sprintf (ich, "%ld (%ld)",
				(long) nlines,
				(long) all
			       );
		info (inf_line, 13, ich, NO);
		infosize = all;
		infoline = nlines;
	    }
	}

	if (curfnum != infofnum && (fileflags[curfile] & INUSE)) Block {
	    char buf[10];
	    char str[10];

	    (void) sprintf (buf, "%d(%d)",
				 curfnum + 1,
				 getnumbywk (last_wksp) + 1);
	    (void) sprintf (str, "%6s:", buf);
	    info (inf_in, 7, str, NO);
	    infofnum = curfnum;
	}

	if (curfile != infofile && (fileflags[curfile] & INUSE)) Block {
	    char *s;
	    Short len;

	    len = strlen (names[curfile]);
	    if (len > inf_avail)
		s = append ("...", names[curfile] + len + 3 - inf_avail);
	    else
		s = append (names[curfile], "");
	    d_put (VCCAS);
	    info (inf_file, inf_avail, s, YES);
	    sfree (s);
	    infofile = curfile;
	}

	if (loopflags.flash) {
	    setbul (YES);
	    loopflags.flash = NO;
	}

contin:
	keyused = YES;
	if (curwksp->wkflags & RANGESET)
	    infoprange (curwksp->wlin + cursorline);

	if (curmark) Block {
	    Reg2 Nlines nlines;

	    Block {
		Reg1 Nlines curline;
		curline = curwksp->wlin + cursorline;
		if ((nlines = curline - (curmark->mrkwinlin + curmark->mrklin)) < 0)
		    nlines = -nlines;
	    }
	    ++nlines;
	    Block {
		Reg1 Ncols ncols;

		if ((ncols = curwksp->wcol + cursorcol
		    - (curmark->mrkwincol + curmark->mrkcol)) < 0)
		    ncols = -ncols;
		if (marklines != nlines) {
		    (void) sprintf (mklinstr, "%ld", (long) nlines);
		    marklines = nlines;
		}
		if (markcols != ncols) {
		    if (ncols)
			(void) sprintf (mkcolstr, "x%ld", (long) ncols);
		    else
			mkcolstr[0] = '\0';
		    markcols = ncols;
		}
	    }
	    if (!HelpActive) Block {
		Reg1 char *cp;
		Reg3 int len;

		cp = append (mklinstr, mkcolstr);
		len = strlen (cp);
		info (inf_area, max (len, infoarealen), cp, NO);
		infoarealen = len;
		sfree (cp);
	    }
	}

	if (!usescroll)
	    loopflags.clear = NO;

	if (cmdmode) {
	    loopflags.hold = NO;
	    if (usescroll) {
		usescroll = NO;
		silent = savesilent;
		needrest = YES;
	    }
	    goto gotcmd;
	}

	if (numtyp > MAXTYP)
	    flushkeys ();

	getkey (WAIT_KEY);

	if (usescroll) {
	    usescroll = NO;
	    silent = savesilent;
	    needrest = YES;
	}
	if (loopflags.hold) {
	    loopflags.hold = NO;
	    mesg (TELALL);
	}
	Block {
	    Reg1 Small donetype;

	    if (curmark) {
		cmarktop = topmark (cursorline);
		cmarkbot = botmark (cursorline);
		cmarkleft = leftmark (cursorcol);
		cmarkright = rightmark (cursorcol);
	    }
	    if (   isprint(key)
#ifndef  CTYPE
		    ||
#ifdef  CHANGECTYPE
		    !correctype &&
#endif
		    isrus8 (key)
#endif
		|| key == CCCTRLQUOTE
		|| key == CCBACKSPACE
		|| key == CCDELCH
		|| key == CCINSCH
	       ) {
		donetype = printchar ();
		goto doneswitch;
	    }
	    Block {
		Reg2 Small cm;

		if (key < MAXMOTIONS && (cm = cntlmotions[key])) {
		    if (HelpActive && HelpMove (cm))
			goto funcdone;
		    /*  EXCEPT
		    /* +TAB, -TAB, and <- and->arrows
		    /**/
		    if (   (cm == RT /*&& cursorcol + 1 <= curwin->redit*/)
			|| (cm == LT /*&& cursorcol - 1 >= curwin->ledit*/)
			|| cm == TB || cm == BT
			|| cm == RW || cm == LW
			|| cm == C1
		       )
			{}
		    else
			credisplay (NO);
		    if (key == CCRETURN && cursorline == curwin->btext)
			/* bottom line => +lines    */
			vertmvwin (defplline);
		    if (offsetflg && key == CCRETURN)
			offsetdown (curwksp->wlin + cursorline,
				    curwksp->wcol + cursorcol);
		    else
			movecursor (cm, 1);
		    if (insmode && key == CCRETURN && okwrite ()) {
			/* insert blank line */
			donetype = ed (OPOPEN, 0,
				   curwksp->wlin + cursorline, (Ncols) 0,
				   (Nlines) 1, (Ncols) 0, YES);
			goto doneswitch;
		    }
		    if (curmark)
			updatemark ();
		    if (infoline != curwksp->wlin + cursorline + 1)
			goto newnumber;
		    goto contin;
		}
	    }
	    credisplay (NO);
	    donetype = CROK;

	    if (cursorcol > curwin->rtext)
		poscursor (curwin->rtext, cursorline);

	    switch (key) {
		case CCALT:
		    infoinreg(rusbit = !rusbit);
		    goto funcdone;

		case CCRUS:
		case CCLAT:
		    infoinreg (rusbit = (key == CCRUS));
		    goto funcdone;

		case CCINT:
		case CCCMD:
		    keyused = YES;
		    goto gotcmd;

		case CCPLFILE:
		case CCMIFILE:
		    if (curmark) {
			if (HelpActive)
			    unmark ();
			else
			    goto nomarkerr;
		    }
		    switchfile (key == CCPLFILE);
		    goto funcdone;

		case CCCHWINDOW:
		    if (curmark) {
			if (HelpActive)
			    unmark ();
			else
			    goto nomarkerr;
		    }
		    if (nwinlist > 1)
			chgwindow (-1);
		    goto funcdone;

		case CCMKWIN:
		    if (curmark)
			goto nomarkerr;
		    makewindow ((char *) NULL, -1);
		    goto funcdone;

		case CCOPEN:
		case CCCLOSE:
		case CCPICK:
		case CCERASE:
		    donetype = edkey (key, NO);
		    goto doneswitch;

		case CCMISRCH:
		case CCPLSRCH:
		    dosearch (key == CCPLSRCH ? 1 : -1);
		    goto doneswitch;

		case CCINSMODE:
		    tglinsmode ();
		    goto funcdone;

		case CCLWINDOW:
		    if (HelpActive && curmark)
			unmark ();
		    horzmvwin (-deflwin);
		    if (HelpActive)
			HelpGotoMarg (NO);
		    goto funcdone;

		case CCRWINDOW:
		    if (HelpActive && curmark)
			unmark ();
		    horzmvwin (defrwin);
		    if (HelpActive)
			HelpGotoMarg (YES);
		    goto funcdone;

		case CCPLLINE:
		    if (HelpActive && curmark)
			unmark ();
		    vertmvwin (defplline);
		    if (HelpActive)
			HelpGotoMarg (YES);
		    goto funcdone;

		case CCMILINE:
		    if (HelpActive && curmark)
			unmark ();
		    vertmvwin (-defmiline);
		    if (HelpActive)
			HelpGotoMarg (NO);
		    goto funcdone;

		case CCPLPAGE:
		    if (HelpActive && curmark)
			unmark ();
		    vertmvwin (defplpage * (1 + curwin->btext));
		    if (HelpActive)
			HelpGotoMarg (YES);
		    goto funcdone;

		case CCMIPAGE:
		    if (HelpActive && curmark)
			unmark ();
		    vertmvwin (-defmipage * (1 + curwin->btext));
		    if (HelpActive)
			HelpGotoMarg (NO);
		    goto funcdone;

		case CCROLLUP:
		case CCROLLDOWN:
		    if (HelpActive)
			HelpMove (key == CCROLLUP ? UP: DN);
		    else
			rollmvwin (key == CCROLLUP ? (Nlines) -1 : (Nlines) 1);
		    goto funcdone;
		/*
		case CCINT:
		    goto nointerr;
		*/
		case CCTABS:
		    sctab (curwksp->wcol + cursorcol, YES);
		    updatetabs ();
		    goto doneswitch;

		case CCMARK:
		    mark ();
		    goto funcdone;

		case CCREPLACE:
		    replkey ();
		    goto doneswitch;

		case CCSPLIT:
		    donetype = split ();
		    goto doneswitch;

		case CCJOIN:
		    donetype = join ();
		    goto doneswitch;

		case CCREDRAW:
		    fresh();
		    goto funcdone;

		case CCBEGS:
		    if (HelpActive)
			HelpGotoMarg (NO);
		    else if (!begline (curwksp->wlin + cursorline,
				  curwksp->wcol + cursorcol))
			goto funcdone;
		    goto doneswitch;

		case CCENDS:
		    if (HelpActive)
			HelpGotoMarg (YES);
		    else if (!endline (curwksp->wlin + cursorline,
				  curwksp->wcol + cursorcol))
			goto funcdone;
		    goto doneswitch;

		case CCLWORD:
		    if (HelpActive)
			(void) HelpMove (LT);
		    else if (!leftword (curwksp->wlin + cursorline,
				   curwksp->wcol + cursorcol))
			goto funcdone;
		    goto doneswitch;

		case CCRWORD:
		    if (HelpActive)
			(void) HelpMove (RT);
		    if (!rightword (curwksp->wlin + cursorline,
				    curwksp->wcol + cursorcol))
			goto funcdone;
		    goto doneswitch;

		case CCTAG:
		    if (curmark)
			goto nomarkerr;
		    if (!imbedtag (curwksp->wlin + cursorline,
				   curwksp->wcol + cursorcol))
			goto noargerr;
		    goto funcdone;

		case CCGOTO:
		    if (HelpActive && curmark)
			unmark ();
		    gotomvwin ((Nlines) 0, curwksp->wcol + cursorcol);
		    if (HelpActive)
			HelpGotoMarg (NO);
		    goto funcdone;

		case CCCLREOL:
		    donetype = cleareol (curwksp->wlin + cursorline,
					 curwksp->wcol + cursorcol);
		    goto doneswitch;

		case CCMEXEC:
		    keyused = YES;
		    getkey (WAIT_KEY);
		    key = (key & 0177) | 040;
		    if (key < 'a' || key > 'z')
			goto noargerr;
		    donetype = mexec (key - 'a', 1);
		    goto doneswitch;

		case CCABANDON:
		    if (curmark) {
			if (HelpActive)
			    unmark ();
			else
			    goto nomarkerr;
		    }
		    dlcurwk ();
		    goto funcdone;

		case CCUNAS1:
		case CCDEL:
		case CCMEND:
		    goto notimperr;

		case CCDELCH:
		case CCMOVELEFT:
		case CCTAB:
		case CCMOVEDOWN:
		case CCHOME:
		case CCRETURN:
		case CCMOVEUP:
		case CC1COLUMN:
		default:
		    goto badkeyerr;
	    }

gotcmd:
	    param ();

	    if (cmdmode && key != CCRETURN)
		goto notcmderr;

	    switch (key) {
		case CCCMD:
		    goto funcdone;

		case CCMEXEC:
		    keyused = YES;
		    getkey (WAIT_KEY);
		    key = (key & 0177) | 040;
		    if (key < 'a' || key > 'z')
			goto noargerr;

		    switch (paramtype) {
		    case 1:
			donetype = mexec (key - 'a', parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto doneswitch;

		case CCLWINDOW:
		case CCRWINDOW:
		    switch (paramtype) {
		    case 0:
			horzmvwin (key == CCRWINDOW
				   ? (Ncols) cursorcol : - curwksp->wcol);
			break;

		    case 1:
			horzmvwin (key == CCRWINDOW
				   ? (Ncols) parmlines : (Ncols) -parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCROLLUP:
		case CCROLLDOWN:
		    switch (paramtype) {
		    case 1:
			rollmvwin (key == CCROLLDOWN
				   ? (Nlines) parmlines : (Nlines) -parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCMIFILE:
		case CCPLFILE:
		    if (curmark) {
			if (HelpActive)
			    unmark ();
			else
			    goto nomarkerr;
		    }
		    if (paramtype == 0)
			getarg (PK_UPTOS);
		    if (*paramv == '\0')
			switchfile (key == CCPLFILE);
		    else
			takefile (paramv);
		    goto funcdone;

		case CCMKWIN:
		    if (curmark) {
			if (HelpActive)
			    unmark ();
			else
			    goto nomarkerr;
		    }
		    if (*paramv == 0)
			removewindow();
		    else
			makewindow (paramv, -1);
		    goto funcdone;

		case CCGOTO:
		    if (HelpActive && curmark)
			unmark ();
		    switch (paramtype) {
		    case 0:
			gotomvwin (la_lsize (curlas), curwksp->wcol + cursorcol);
			break;

		    case 1:
			if (parmlines <= 0)
			    goto notposerr;
			gotomvwin (parmlines - 1, curwksp->wcol + cursorcol);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCINT:
		    goto funcdone;

		case CCTAG:
		    if (curmark)
			goto nomarkerr;
		    if (paramtype == 0)
			getarg (PK_IDENT);
		    if (*paramv == 0)
			goto noargerr;
		    Block {
			Reg2 char *tag;

			for (tag = paramv; *tag; tag++)
			    if (!isalnum (*tag) && *tag != '_')
				goto noargerr;
			tag = append (paramv, "");
			srchtag (tag);
			sfree (tag);
		    }
		    goto funcdone;

		case CCMISRCH:
		case CCPLSRCH:
		    if (paramtype == 0)
			getarg (PK_IDENT);
		    if (*paramv == 0)
			goto noargerr;
		    if (searchkey)
			sfree (searchkey);
		    searchkey = append (paramv, "");
		    if ((rex = regtest (searchkey)) < 0) {
			sfree (searchkey);
			searchkey = NULL;
			goto funcdone;
		    }
		    dosearch (key == CCPLSRCH ? 1 : -1);
		    goto doneswitch;

		case CCBACKSPACE:
		    /* donetype =0 */
		    Block {
			Reg2 Ncols  k;

			k = curwksp->wcol;
			if (insmode)
			    donetype = ed (OPCLOSE, QCLOSE,
					   curwksp->wlin + cursorline, k,
					   (Nlines) 1, (Ncols) cursorcol, YES);
			else {
			    savecurs ();
			    donetype = ed (OPERASE, QERASE,
					   curwksp->wlin + cursorline, k,
					   (Nlines) 1, (Ncols) cursorcol, YES);
			    restcurs ();
			}
		    }
		    goto doneswitch;

		case CCDELCH:
		    donetype = cleareol (curwksp->wlin + cursorline,
					 curwksp->wcol + cursorcol);
		    goto doneswitch;

		case CCOPEN:
		case CCCLOSE:
		case CCPICK:
		case CCERASE:
		    donetype = edkey (key, YES);
		    goto funcdone;

		case CCMARK:
		    if (paramtype != 0)
			goto notimperr;
		    unmark ();
		    goto funcdone;

		case CCMOVELEFT:
		case CCMOVEDOWN:
		case CCMOVEUP:
		case CCMOVERIGHT:

		case CCTAB:
		case CCHOME:
		case CCBACKTAB:
		    switch (paramtype) Block {
			Reg2 int lns;

		    case 0:
			switch (key) {
			case CCHOME:
			    movecursor (cntlmotions[CCHOME], 1);
			    key = CCMOVEDOWN;
			    goto llcor;

			case CCMOVEDOWN:
			    if ((lns = la_lsize (curlas) -
				       (curwksp->wlin + cursorline)) > 0)
				lns = min (lns, curwin->btext - cursorline);
			    else
		    llcor:      lns = curwin->btext - cursorline;
			    break;

			case CCMOVEUP:
			    lns = cursorline;
			    break;

			case CCBACKTAB:
			case CCTAB:
			    goto notimperr;

			case CCMOVELEFT:
			    lns = cursorcol;
			    break;

			case CCMOVERIGHT:
			    if (cursorcol >= curwin->rtext)
				key = CCMOVELEFT;
			    lns = abs (curwin->rtext - cursorcol);
			    break;
			}
			if (lns <= 0)
			    goto funcdone;
			goto multmove;

		    case 1:
			if (parmlines <= 0)
			    goto notposerr;
			if (key == CCHOME)
			    goto notimperr;
			lns = parmlines;
 multmove:              movecursor (cntlmotions[key], lns);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto doneswitch;

		case CCRETURN:
		    donetype = command ();
		    goto doneswitch;

		case CCMIPAGE:
		case CCPLPAGE:
		    switch (paramtype) {
		    case 0:
			gotomvwin (key == CCPLPAGE ? la_lsize (curlas) : (Nlines) 0,
						curwksp->wcol + cursorcol);
			break;

		    case 1:
			vertmvwin ((key == CCPLPAGE ? parmlines : -parmlines )
				    * (1 + curwin->btext));
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCMILINE:
		case CCPLLINE:
		    switch (paramtype) {
		    case 0:
			vertmvwin (cursorline - (key == CCPLLINE
				   ? 0 : curwin->btext));
			break;

		    case 1:
		       vertmvwin (key == CCPLLINE ? parmlines : -parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCCHWINDOW:
		    if (curmark) {
			if (HelpActive)
			    unmark ();
			else
			    goto nomarkerr;
		    }
		    switch (paramtype) {
		    case 0:
			goto notimperr;

		    case 1:
			if (parmlines <= 0)
			    goto notposerr;
			chgwindow (parmlines - 1);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCTABS:
		    if (paramtype == 0) {
			sctab (curwksp->wcol + cursorcol, NO);
			updatetabs ();
		    }
		    else
			goto notimperr;
		    goto doneswitch;

		case CCINSMODE:
		    goto badkeyerr;

		case CCSPLIT:
		case CCJOIN:
		case CCUNAS1:
		case CCREPLACE:
		case CCLWORD:
		case CCRWORD:
		case CCDEL:
		default:
		    goto notimperr;
	    }

doneswitch:
	    if (curmark)
		updatemark ();

	    switch (donetype) {
nomemerr:
	    case NOMEMERR:
		mesg (ERRALL + 1, ediag(
"You have run out of memory.  Exit now!",
"Кончилась свободная память. Вам лучше выйти!"));
		break;

	    case NOSPCERR:
		mesg (ERRALL + 1, ediag(
"No disk space.  Get help!",
"Нет места на диске. Попробуйте освободить!"));
		break;

	    case TOOLNGERR:
		mesg (ERRALL + 1, ediag(
"Can't make file that long",
"Нельзя сделать такой длинный файл."));
		break;

	    case NOTSTRERR:
		mesg (ERRALL + 1, ediag(
"Argument must be a string.",
"Аргумент должен быть строкой."));
		break;

	    case NOWRITERR:
		mesg (ERRALL + 1, ediag(
"You cannot modify this file!",
"Вам нельзя изменять этот файл!"));
		break;

	    noargerr:
	    case NOARGERR:
		mesg (ERRALL + 1, ediag("Invalid argument.",
					"Неверный аргумент."));
		break;

	    case NDMARKERR:
		mesg (ERRALL + 1, ediag("Area must be marked",
					"Область должна быть помечена"));
		break;

	    nomarkerr:
	    case NOMARKERR:
		mesg (ERRALL + 1, ediag("Can't do that with marks set",
			"Нельзя сделать это при заданной области"));
		break;

	    norecterr:
	    case NORECTERR:
		mesg (ERRALL + 1, ediag("Can't do that to a rectangle",
			"Нельзя сделать это с прямоугольником"));
		break;

	    case NOTRECTERR:
		mesg (ERRALL + 1, ediag("Can only do that to a rectangle",
				"Это можно делать только с прямоугольником"));
		break;

	    case NORANGERR:
		mesg (ERRALL + 1, ediag("Range not set",
				"Диапазон не задан"));
		break;

	    case NOBUFERR:
		mesg (ERRALL + 1, ediag("Nothing in that buffer.",
				"Этот буфер пуст."));
		break;

	    case CONTIN:
		goto contin;

	    case MARGERR:
		mesg (ERRALL + 1, ediag("Cursor stuck on right margin.",
			"Курсор достиг правой границы."));
		break;

	    notinterr:
	    case NOTINTERR:
		mesg (ERRALL + 1, ediag("Argument must be numeric.",
				"Аргумент должен быть числом."));
		break;

	    notposerr:
	    case NOTPOSERR:
		mesg (ERRALL + 1, ediag("Argument must be positive.",
			"Аргумент должен быть положительным."));
		break;

	    notimperr:
	    case NOTIMPERR:
		mesg (ERRALL + 1, ediag(
"That key sequence is not implemented.",
"Эти клавиши не задействованы."));
		if (!replaying)
		    flinput ();
		break;
	    }
	    continue;

	    notcmderr:
	    mesg (ERRALL + 1, ediag("Can't do that in Command Mode",
				"Нельзя сделать это в командном режиме"));
	    continue;
/******************
	    nointerr:
	    mesg (ERRALL + 1, ediag("No operation to interrupt",
			"Нет ничего, что можно прервать"));
	    continue;
********************/
	    badkeyerr:
	    mesg (ERRALL + 1, ediag("Bad key error - editor error",
				"Внутреняя ошибка редактора"));
	    continue;
	}
    }
    /* NOTREACHED */
}
