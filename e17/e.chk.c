#include "e.h"
#include "e.tt.h"
#include "e.fn.h"

void cargs ();
#ifdef  SECURITY
extern Flag nowritefile;
extern Flag nomorecommand;
#endif

#define OPTREPLAY       0
#define OPTRECOVER      1
#define OPTDEBUG        2
#define OPTSILENT       3
#define OPTHELP         4
#define OPTNOTRACKS     5
#define OPTINPLACE      6
#define OPTNORECOVER    7
#define OPTTERMINAL     8
#define OPTKEYBOARD     9
#ifdef  KBFILE
#define OPTKBFILE      10
#endif  KBFILE
/*#define OPTBULLETS     11*/
/*#define OPTNOBULLETS   12*/
#ifdef TERMCAP
#define OPTDTERMCAP    13
#endif TERMCAP
#define OPTVISTABS     14
#define OPTNOVISTABS   15
#define OPTOFFSET      16
#define OPTNOOFFSET    17
#define OPTINSERT      18
#define OPTNOINSERT    19
#define OPTDIRECTORY   20
#define OPTBINARY      21
/*NOXXSTR*/
/* Только в алфавитном порядке */
S_looktbl opttable[] = {
    "binary"   , OPTBINARY   ,
    "debug"    , OPTDEBUG    ,
    "directory", OPTDIRECTORY,
#ifdef TERMCAP
    "dtermcap" , OPTDTERMCAP ,
#endif
    "help"     , OPTHELP     ,
    "inplace"  , OPTINPLACE  ,
    "insmode"  , OPTINSERT   ,
#ifdef  KBFILE
    "kbfile"   , OPTKBFILE   ,
#endif
    "keyboard" , OPTKEYBOARD ,
    "noinsmode", OPTNOINSERT ,
    "nooffset" , OPTNOOFFSET ,
    "norecover", OPTNORECOVER,
    "notracks" , OPTNOTRACKS ,
    "novistabs", OPTNOVISTABS,
    "offset"   , OPTOFFSET   ,
    "replay"   , OPTREPLAY   ,
    "silent"   , OPTSILENT   ,
    "terminal" , OPTTERMINAL ,
    "vistabs"  , OPTVISTABS  ,
    (char *) NULL, 0
};
/*YESXSTR*/
Flag helpflg,
     dbgflg;
extern Small numargs;
extern Small curarg;
static char *curargchar;
extern char **argarray;
Flag optvistabs = -1;   /* YES = -vistabs, NO = -novistabs */
#ifdef TERMCAP
Flag optdtermcap;       /* YES = force use of termcap */
#endif
Flag optgoto = NO;      /* при запуске встать на строку с номером Igoto */
Nlines Igoto = 0;
Flag optsearch = NO;    /* при запуске поискать строку Isearch */
Flag optoffset = -1;
Flag optinsert = -1;
char *Isearch;
char *dbgname;      /* argument to -debug=xxx option */
char *dirname;      /* argument to -dir=xxx option */
char *replfname;    /* argument to -replay=xxx option */
char *opttname;
char *optkname;     /* Keyboard name */
#ifdef  KBFILE
char *kbfile;           /* name of input (keyboard) table file */
char *optkbfile;        /* Keyboard translation file name */
#endif

/*
    Scan the arguments to the editor.  Set flags and options and check for
    errors in the option arguments.
*/
void
checkargs ()
{
    Reg1 char *s1;
    Reg2 char *s2;
    Reg3 Short i;
    Small cure;
    char *args[30];

#ifdef  SECURITY
    nomorecommand = ChkPnt1 ();
#endif
    if ((s1 = getenv("LEKEYS")) != (char *) NULL && *s1 != '\0') {
	s2 = salloc (strlen (s1) + 1, YES);
	(void) strcpy (s2, s1);
	i = 0;
/*NOXXSTR*/
	args[i++] = ediag ("Environment", "Окружение");
/*YESXSTR*/
	for (s1 = s2; *s1 ; s1++) {
	    args[i++] = s1;
	    while (*s1 && *s1 != ',')
		s1++;
	    if (!*s1)
		break;
	    *s1 = '\0';
	}
	args[i] = (char *) NULL;
	cargs (i, args, &cure);
	if (cure < i)
	    getout (YES, ediag("%s: Syntax error in LEKEYS environment variable",
				"%s: Синтаксическая ошибка в переменной окружения LEKEYS"),
				argarray[0]);
    }
    cargs (numargs, argarray, &curarg);
#ifdef  SECURITY
    nowritefile = ChkPnt3 ();
#endif
}


static
void
cargs (numargs, argarray, curarg)
Small numargs;
char **argarray;
Small *curarg;
{
    dbgflg = NO;
/*NOXXSTR*/
    Isearch = ediag ("search_string", "образец_поиска");
/*YESXSTR*/
    for (*curarg = 1; *curarg < numargs; (*curarg)++) Block {
	Reg1 char *cp;
	Reg2 Flag opteqflg;
	Reg3 Short tmp;
	Reg4 char *opterr;

	curargchar = argarray[*curarg];
	if (*curargchar != '-' && *curargchar != '+')
	    break;
	curargchar++;
	opteqflg = NO;
	if (curargchar[-1] == '+') Block {
	    char *s, *rindex ();
	    int i;

	    if (*curargchar == '/') {
		if ((s = rindex (curargchar, '/')) == curargchar)
		    goto unrecog;
		*s = '\0';
		if (optsearch)
		    goto repeat;
		if (*(++curargchar) == '\0') {
		    opterr = ediag("empty","пустой");
		    goto error;
		}
		Isearch = curargchar;
		optsearch = YES;
		continue;
	    }
	    if ((s = s2i (curargchar, &i)) == curargchar || i < 1)
		goto unrecog;
	    if (optgoto)
		goto repeat;
	    Igoto = i;
	    optgoto = YES;
	    continue;
	}
	for (cp = curargchar; *cp; cp++)
	    if (*cp == '=') {
		opteqflg = YES;
		*cp = 0;
		break;
	    }
	if (cp == curargchar)
	    goto unrecog;
	tmp = lookup (curargchar, opttable);
	if (opteqflg)
	    *cp++ = '=';
	if (tmp == -1)
	    goto unrecog;
	if (tmp == -2) {
	    opterr = ediag("ambiguous","неоднозначен");
	    goto error;
	}
	switch (opttable[tmp].val) {
	case OPTHELP:
	    helpflg = YES;
	    break;

#ifdef TERMCAP
	case OPTDTERMCAP:
	    optdtermcap = YES;
	    break;
#endif TERMCAP

	case OPTDEBUG:
	    /* file for debug info */
	    if (!opteqflg || *cp == 0)
		getout (YES, ediag("%s: -debug='debug file name'",
				   "%s: -debug='имя файла отладки'"),
				   argarray[0]);
	    if (dbgflg)
		goto repeat;
	    dbgflg = YES;
	    dbgname = cp;
	    break;

	case OPTINPLACE:
	    inplace = YES;
	    break;

	case OPTBINARY:
	    binary = YES;
	    break;

	case OPTSILENT:
	    silent = YES;
	    break;

	case OPTNORECOVER:
	    norecover = YES;
	    break;

	case OPTNOTRACKS:
	    notracks = YES;
	    break;

	case OPTREPLAY:
	    replaying = YES;
	    if (opteqflg && *cp)
		replfname = cp;
	    break;

	case OPTKEYBOARD:
	    if (opteqflg && *cp)
		optkname = cp;
	    break;

#ifdef  KBFILE
	case OPTKBFILE:
	    if (opteqflg && *cp)
		optkbfile = cp;
	    break;
#endif  KBFILE

	case OPTDIRECTORY:
	    if (!opteqflg || *cp == 0)
		getout (YES, ediag("%s: -directory='work directory name'",
				   "%s: -directory='имя рабочего каталога'"),
				   argarray[0]);
	    dirname = cp;
	    break;

	case OPTTERMINAL:
	    if (opteqflg && *cp)
		opttname = cp;
	    break;

	case OPTVISTABS:
	case OPTNOVISTABS:
	    optvistabs = opttable[tmp].val == OPTVISTABS;
	    break;

	case OPTOFFSET:
	case OPTNOOFFSET:
	    offsetflg = optoffset = opttable[tmp].val == OPTOFFSET;
	    break;

	case OPTINSERT:
	case OPTNOINSERT:
	    insmode = optinsert = opttable[tmp].val == OPTINSERT;
	    break;

	default:
	unrecog:
	    opterr = ediag("not recognized","не распознан");
	error:
/*NOXXSTR*/
	    getout (YES, ediag("%s: %s option %s","%s: Ключ %s %s"),
/*YESXSTR*/
			 argarray[0], argarray[*curarg], opterr);
	    /*NOTREACHED*/
	repeat:
	    opterr = ediag("repeated","уже был");
	    goto error;
	}
    }
}

/*    Do the help option. */
void
showhelp ()
{
    char buf[50];

    fixtty ();
    printf (ediag("\n\
syntax: %s [option]... [file]...\n\
options are:\n",
"\n\
Вызов: %s [флаг]... [файл]...\n\
Флаги:\n"),
progname);
/*NOXXSTR*/
    if (optgoto)
	(void) sprintf (buf, "%d", Igoto);
    else
	(void) sprintf (buf, "%s",
/*YESXSTR*/
			ediag ("linenumber", "номер_строки"));
    printf ("\
%c +%s\n", (optgoto ? '*' : ' '), buf);
    printf ("\
%c +/%s/\n", (optsearch ? '*' : ' '), Isearch);
    printf ("\
%c -binary\n", binary ? '*' : ' ');
    if (dbgflg)
	printf ("\
* -debug=%s\n", dbgname);
    printf ("\
* -directory=%s\n", dirname);
    printf ("\
* -help\n");
    printf ("\
%c -inplace\n", inplace ? '*' : ' ');
    printf ("\
%c -insmode\n", optinsert == YES ? '*' : ' ');
#ifdef  KBFILE
/*NOXXSTR*/
    printf ("\
%c -kbfile=%s\n", optkbfile ? '*' : ' ',
/*YESXSTR*/
	    kbfile ? kbfile : ediag("(none)", "(отсутствует)"));
#endif  KBFILE
    printf ("\
%c -keyboard=%s\n", optkname ? '*' : ' ',
	    kname ? kname : tname);
    printf ("\
%c -noinsmode\n", optinsert == NO ? '*' : ' ');
    printf ("\
%c -nooffset\n", optoffset == NO ? '*' : ' ');
    printf ("\
%c -norecover\n", norecover ? '*' : ' ');
    printf ("\
%c -notracks\n", notracks ? '*' : ' ');
    printf ("\
%c -novistabs\n", optvistabs == NO ? '*' : ' ');
    printf ("\
%c -offset\n", optoffset == YES ? '*' : ' ');
/*NOXXSTR*/
    printf ("\
%c -replay=%s\n", replaying ? '*' : ' ', replaying ? inpfname :
/*YESXSTR*/
ediag("filename", "имя_файла"));
    printf ("\
%c -silent\n", silent ? '*' : ' ');
    printf ("\
%c -terminal=%s\n", opttname ? '*' : ' ', tname);
    printf ("\
%c -vistabs\n", optvistabs == YES ? '*' : ' ');
    printf (ediag("\
\"*\" means option was in effect.",
"\n\
\"*\" означает, что флаг включен."));
}
