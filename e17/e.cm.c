/*
    command dispatching routine and some actual command-executing routines.
*/

#include "e.h"
#include "e.e.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.ru.h"
#include "e.cm.h"
#include "e.wi.h"
#include "e.tt.h"
#include SIG_INCL

extern Flag visualtabs;
extern Echar CurrFont;

/*NOXXSTR*/
S_looktbl cmdtable[] = {
#ifdef CMDVERALLOC
    "#veralloc", CMDVERALLOC,
#endif
    "-binary" , CMD_BINARY  ,
    "-blot"   , CMD_BLOT    ,
    "-close"  , CMD_CLOSE   ,
    "-command", CMD_COMMAND ,
    "-diff"   , CMD_DIFF    ,
    "-erase"  , CMD_ERASE   ,
    "-inplace", CMD_INPLACE ,
    "-insmode", CMD_INSMODE ,
    "-join"   , CMDSPLIT    ,
    "-macro"  , CMD_MACRO   ,
    "-mark"   , CMD_MARK    ,
    "-offset" , CMD_OFFSET  ,
    "-pick"   , CMD_PICK    ,
    "-range"  , CMD_RANGE   ,
    "-replace", CMD_REPLACE ,
    "-search" , CMD_SEARCH  ,
    "-split"  , CMDJOIN     ,
    "-tab"    , CMD_TAB     ,
    "-tabfile", CMD_TABFILE ,
    "-tabs"   , CMD_TABS    ,
    "-track"  , CMD_TRACK   ,
    "-update" , CMD_UPDATE  ,
    "-vistabs", CMD_VISTABS ,
    "-window" , CMD_WINDOW  ,
    "?range"  , CMDQRANGE   ,
    "abandon" , CMDABANDON  ,
    "b"       , CMDEXIT     ,
    "binary"  , CMDBINARY   ,
    "blot"    , CMDBLOT     ,
    "box"     , CMDBOX      ,
    "bye"     , CMDEXIT     ,
    "call"    , CMDCALL     ,
    "center"  , CMDCENTER   ,
    "clear"   , CMDCLEAR    ,
    "close"   , CMDCLOSE    ,
    "command" , CMDCOMMAND  ,
    "cover"   , CMDCOVER    ,
    "delete"  , CMDDELETE   ,
    "diff"    , CMDDIFF     ,
    "e"       , CMDEDIT     ,
    "edit"    , CMDEDIT     ,
    "endm"    , CMDENDM     ,
    "erase"   , CMDERASE    ,
    "exit"    , CMDEXIT     ,
    "feed"    , CMDFEED     ,
    "fill"    , CMDFILL     ,
    "font"    , CMDFONT     ,
    "goto"    , CMDGOTO     ,
    "help"    , CMDHELP     ,
    "horizontal", CMDHORWIN ,
    "inplace" , CMDINPLACE  ,
    "insert"  , CMDINSERT   ,
    "insmode" , CMDINSMODE  ,
    "join"    , CMDJOIN     ,
    "justify" , CMDJUST     ,
    "logout"  , CMDLOGOUT   ,
    "macro"   , CMDMACRO    ,
    "mark"    , CMDMARK     ,
    "name"    , CMDNAME     ,
    "offset"  , CMDOFFSET   ,
    "open"    , CMDOPEN     ,
    "overlay" , CMDOVERLAY  ,
    "pick"    , CMDPICK     ,
    "print"   , CMDPRINT    ,
    "quit"    , CMDEXIT     ,
    "range"   , CMDRANGE    ,
    "redraw"  , CMDREDRAW   ,
    "rename"  , CMDNAME     ,
    "replace" , CMDREPLACE  ,
    "run"     , CMDRUN      ,
#ifdef SIGTSTP  /* 4bsd vmunix */
    "s"       , CMDSTOP     ,
#endif
    "save"    , CMDSAVE     ,
    "search"  , CMDSEARCH   ,
    "shell"   , CMDSHELL    ,
    "split"   , CMDSPLIT    ,
#ifdef SIGTSTP  /* 4bsd vmunix */
    "stop"    , CMDSTOP     ,
#endif
    "tab"     , CMDTAB      ,
    "tabfile" , CMDTABFILE  ,
    "tabs"    , CMDTABS     ,
    "tag"     , CMDTAG      ,
    "track"   , CMDTRACK    ,
    "underlay", CMDUNDERLAY ,
    "undo"    , CMDUNDO     ,
    "update"  , CMDUPDATE   ,
    "vertical", CMDVERWIN   ,
    "vistabs" , CMDVISTABS  ,
    "window"  , CMDWINDOW   ,
    0, 0
};
/*YESXSTR*/
extern void dostop ();
extern Cmdret dotag ();

/*
    Parses a command line and dispatches on the command.
    If a routine that was called to execute a command returns a negative
    value, then the error message is printed out here, and returns CROK.
    Else returns the completion status of the command.
*/
Cmdret
command ()
{
    Short cmdtblind;
    Cmdret retval;
    Short cmdval;
    char *cmdstr;
    Small which;

    nxtop = paramv;
    cmdstr = getword (&nxtop);
    if (cmdstr[0] == 0)
	return CROK;
    cmdtblind = lookup (cmdstr, cmdtable);
    if (cmdtblind == -1) {
/*NOXXSTR*/
	mesg (ERRALL + 3, ediag("Command \"","Команда \""),
/*YESXSTR*/
cmdstr, ediag("\" not recognized","\" не найдена"));
	sfree (cmdstr);
	return CROK;
    }
    else if (cmdtblind == -2) {
/*NOXXSTR*/
	mesg (ERRALL + 3, ediag("Command \"","Команда \""),
/*YESXSTR*/
cmdstr, ediag("\" ambiguous","\" неоднозначна"));
	sfree (cmdstr);
	return CROK;
    }
    sfree (cmdstr);

    cmdopstr = nxtop;
    opstr = getword (&nxtop);

    cmdname = cmdtable[cmdtblind].str;
    switch (cmdval = cmdtable[cmdtblind].val) {
#ifdef CMDVERALLOC
    case CMDVERALLOC:
	veralloc ();
	retval = CROK;
	break;
#endif

    case CMDRANGE:
    case CMD_RANGE:
    case CMDQRANGE:
	retval = rangecmd (cmdval);
	break;

    case CMDTRACK:
	curwin->winflgs |= TRACKSET;
	infotrack (YES);
	retval = CROK;
	break;

    case CMD_TRACK:
	curwin->winflgs &= ~TRACKSET;
	infotrack (NO);
	retval = CROK;
	break;

#ifdef SIGTSTP  /* 4bsd vmunix */
    case CMDSTOP:
	dostop ();
	retval = CROK;
	break;
#endif

    case CMDUPDATE:
    case CMD_UPDATE:
	retval = doupdate (cmdval == CMDUPDATE);
	break;

    case CMDTAB:
	retval = dotab (YES);
	break;

    case CMD_TAB:
	retval = dotab (NO);
	break;

    case CMDTABS:
	retval = dotabs (YES);
	break;

    case CMD_TABS:
	retval = dotabs (NO);
	break;

    case CMDTABFILE:
    case CMD_TABFILE:
	retval = tabfile (cmdval == CMDTABFILE);
	break;

    case CMDREPLACE:
    case CMD_REPLACE:
	if (!okwrite ()) {
	    retval = NOWRITERR;
	    break;
	}
	retval = replace (cmdval == CMDREPLACE? 1: -1);
	break;
/*
    case CMDDIFF:
    case CMD_DIFF:
	retval = diff (cmdval == CMDDIFF? 1: -1);
	break;
*/
    case CMDNAME:
	retval = name ();
	break;

    case CMDDELETE:
	retval = delete ();
	break;

    case CMDCOMMAND:
	mesg (TELALL + 1, ediag("CMDS: ", "КОМ: "));
    case CMD_COMMAND:
	cmdmode = cmdval == CMDCOMMAND ? YES : NO;
	retval = CROK;
	break;

    case CMDPICK:
    case CMDCLOSE:
    case CMDERASE:
    case CMDOPEN:
    case CMDBOX:
	retval = areacmd (cmdval - CMDPICK);
	break;

    case CMDCOVER:
    case CMDOVERLAY:
    case CMDUNDERLAY:
    case CMDBLOT:
    case CMD_BLOT:
    case CMDINSERT:
	retval = insert (cmdval - CMDCOVER);
	break;

    case CMD_PICK:
    case CMD_CLOSE:
    case CMD_ERASE:
	retval = insbuf (cmdval - CMD_PICK + QPICK);
	break;

    case CMDCALL:
	retval = call ();
	break;

    case CMDSHELL:
	retval = shell ();
	break;

    case CMDLOGOUT:
	if (!loginflg) {
	    mesg (ERRALL + 1, ediag(
"This is not your login program.  Use \"exit\".",
"Это не интерпретатор команд. Выполняется \"exit\"."));
	}
    case CMDEXIT:
	retval = eexit ();
	/* если синтаксис exit правилен и все успешно сохранилось,
	/* eexit не возвращается. */
	break;

    case CMDREDRAW:
	fresh ();
	retval = CROK;
	break;

    case CMDSPLIT:
    case CMDJOIN:
	if (*cmdopstr)
	    retval = CRTOOMANYARGS;
	else
	    retval = cmdval == CMDJOIN ? join () : split ();
	break;

    case CMDRUN:
    case CMDFEED:
	if (!okwrite ()) {
	    retval = NOWRITERR;
	    break;
	}
	retval = run (cmdopstr, cmdval);
	break;

    case CMDFILL:
	retval = filter (FILLNAMEINDEX, YES);
	break;

    case CMDJUST:
	retval = filter (JUSTNAMEINDEX, YES);
	break;

    case CMDCENTER:
	retval = filter (CENTERNAMEINDEX, YES);
	break;

    case CMDPRINT:
	retval = filter (PRINTNAMEINDEX, YES);
	break;

    case CMDSAVE:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	retval = save ();
	break;

    case CMDEDIT:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	retval = edit ();
	break;

    case CMDTAG:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	retval = dotag ();
	break;

    case CMDVERWIN:
    case CMDHORWIN:
	which = cmdval == CMDVERWIN ? 0 : 1;
	goto mkwin;

    case CMDWINDOW:
	which = -1;
mkwin:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	if (*opstr == '\0')
	    makewindow ((char *) NULL, which);
	else {
	    if (*nxtop) {
		retval = CRTOOMANYARGS;
		break;
	    }
	    makewindow (opstr, which);
	}
	retval = CROK;
	break;

    case CMD_WINDOW:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	if (*opstr) {
	    retval = CRTOOMANYARGS;
	    break;
	}
	removewindow ();
	retval = CROK;
	break;

    case CMDGOTO:
	retval = gotocmd ();
	break;

    case CMDMACRO:
    case CMD_MACRO:
	retval = domacro (cmdval == CMDMACRO);
	break;

    case CMDVISTABS:
    case CMD_VISTABS:
	retval = CROK;
	if (visualtabs == (cmdval == CMDVISTABS))
	    break;
	visualtabs = cmdval == CMDVISTABS;
Drwsds:
	savecurs ();
	drawborders (curwin, WIN_ACTIVE);
	restcurs ();
	break;

    case CMDOFFSET:
    case CMD_OFFSET:
	offsetflg = cmdval == CMDOFFSET;
	retval = CROK;
	break;

    case CMDINSMODE:
    case CMD_INSMODE:
	if (insmode != (cmdval == CMDINSMODE))
	    tglinsmode ();
	retval = CROK;
	break;

    case CMDINPLACE:
    case CMD_INPLACE:
	inplace = cmdval == CMDINPLACE;
	retval = CROK;
	break;

    case CMDBINARY:
    case CMD_BINARY:
	if (binary != (cmdval == CMDBINARY)) {
	    getline (-1);
	    binary = !binary;
	    putupwin ();
	}
	retval = CROK;
	break;

    case CMDFONT:
	retval = dofont ();
	break;

    case CMDHELP:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	if (*nxtop)
	    return CRTOOMANYARGS;
	retval = dohelp ();
	break;

    case CMDABANDON:
	if (curmark) {
	    retval = NOMARKERR;
	    break;
	}
	if (*nxtop)
	    return CRTOOMANYARGS;
	dlcurwk ();
	retval = CROK;
	break;

    default:
/*NOXXSTR*/
	mesg (ERRALL + 3, ediag("Command \"","Команда \""),
cmdtable[cmdtblind].str,
/*YESXSTR*/
ediag("\" not implemented yet", "\" еще не реализована"));
	retval = CROK;
	break;
    }
    if (opstr[0] != '\0')
	sfree (opstr);

    if (retval >= CROK)
	return retval;
    switch (retval) {
    case CRUNRECARG:
	mesg (1, ediag(" unrecoginzed argument to ",
		       " не найден аргумент для "));
	break;

    case CRAMBIGARG:
	mesg (1, ediag(" ambiguous argument to ",
		       " неоднозначный аргумент для "));
	break;

    case CRTOOMANYARGS:
	mesg (ERRSTRT + 1, ediag("Too many of arguments to ",
				 "Слишком много аргументов для "));
	break;

    case CRNEEDARG:
	mesg (ERRSTRT + 1, ediag("Need an argument to ",
				 "Нужен аргумент для "));
	break;

    case CRNOVALUE:
	mesg (ERRSTRT + 1, ediag("No value for option to ",
				"Нет значения ключа для "));
	break;

    case CRMULTARG:
	mesg (ERRSTRT + 1, ediag("Duplicate arguments to ",
				"Одинаковые аргументы для "));
	break;

    case CRMARKCNFL:
	return NOMARKERR;

    case CRBADARG:
    default:
	mesg (ERRSTRT + 1, ediag("Bad argument(s) to ",
				 "Неверные аргументы для "));
    }
    mesg (ERRDONE + 3, "\"", cmdname, "\"");

    return CROK;
}

/*   Do the "goto" command. */
Cmdret
gotocmd ()
{
    if (opstr[0] == '\0') {
	gotomvwin ((Nlines) 0, curwksp->wcol + cursorcol);
	return CROK;
    }
    if (*nxtop)
	return CRTOOMANYARGS;

    Block {
	Reg2 Short tmp;
	Reg1 char *cp;
	char *cp1;

	cp1 = opstr;
	tmp = getpartype (&cp1, 0, 0, 0);
	if (tmp == 1) {
	    for (cp = cp1; *cp && *cp == ' '; cp++)
		continue;
	    if (*cp == 0) {
		gotomvwin (parmlines - 1, curwksp->wcol + cursorcol);
		return CROK;
	    }
	}
	else if (tmp == 2)
	    return CRBADARG;
    }

    Block {
/*NOXXSTR*/
	static S_looktbl gttbl[] = {
	    "begin",        0,
	    "end",          1,
	    "rbegin",       2,
	    "rend",         3,
	    0,              0
	};
/*YESXSTR*/
	Reg1 Small ind;
	Reg2 Small val;

	if ((ind = lookup (opstr, gttbl)) < 0) {
	    mesg (ERRSTRT + 1, opstr);
	    return ind == -2 ? CRAMBIGARG : CRUNRECARG;
	}
	switch (val = gttbl[ind].val) {
	case 0:
	    gotomvwin ((Nlines) 0, curwksp->wcol + cursorcol);
	    break;

	case 1:
	    gotomvwin (la_lsize (curlas), curwksp->wcol + cursorcol);
	    break;

	case 2:
	case 3:
	    if (curwksp->brnglas)
		gotomvwin (la_lseek (val == 2
				     ? curwksp->brnglas : curwksp->ernglas,
				     0, 1), curwksp->wcol + cursorcol);
	    else
		return NORANGERR;
	    break;
	}
    }
    return CROK;
}

Cmdret
dofont ()
{
    Small ind;
/*NOXXSTR*/
    static S_looktbl fonttable[] = {
	"bold"   ,  0,
	"italic" ,  1,
	"romanic",  2,
	0        ,  0
    };
/*YESXSTR*/

    if (*nxtop)
	return CRTOOMANYARGS;
    if (opstr[0] == '\0')
	return CRNEEDARG;
    switch (ind = lookup (opstr, fonttable)) {
	case 0:
	    CurrFont = IA_MD;
	    break;
	case 1:
	    CurrFont = IA_AS;
	    break;
	case 2:
	    CurrFont = IA_NORMAL;
	    break;
	case -1:
	case -2:
	    mesg (ERRSTRT + 1, opstr);
	    return ind;
    }
    return CROK;
}

/*
    Do the "update" command.
    The 'on' argument is non-0 for "update" and 0 for "-update"
*/
Cmdret
doupdate (on)
Flag on;
{
    Small ind;
/*NOXXSTR*/
    static S_looktbl updatetable[] = {
	"-inplace", 0,
	"inplace",  0,
	0        ,  0
    };
/*YESXSTR*/
    if (*nxtop || !on && opstr[0] != '\0')
	return CRTOOMANYARGS;

    if (on && !(fileflags[curfile] & DWRITEABLE)) {
	mesg (ERRALL + 1, ediag("Don't have directory write permission",
				"Каталог закрыт на запись"));
	return CROK;
    }
    if (opstr[0] != '\0') {
	ind = lookup (opstr, updatetable);
	if (ind == -1  || ind == -2) {
	    mesg (ERRSTRT + 1, opstr);
	    return ind;
	}
	/* ind can be 0 = -inplace or 1 = inplace */
	if (ind) { /* inplace */
	    if (fileflags[curfile] & NEW) {
		mesg (ERRALL + 1, ediag("\"inplace\" is useless;  file is new",
					"\"inplace\" бессмыслено;  файл новый"));
		return CROK;
	    }
	    if (!(fileflags[curfile] & FWRITEABLE)) {
		mesg (ERRALL + 1, ediag("Don't have file write permission",
					"Файл закрыт на запись"));
		return CROK;
	    }
	    fileflags[curfile] |= INPLACE | CANMODIFY;
	}
	else /* -inplace */
	    fileflags[curfile] &= ~INPLACE;
    }
    if (on)
	fileflags[curfile] |= UPDATE;
    else
	fileflags[curfile] &= ~UPDATE;
    return CROK;
}
