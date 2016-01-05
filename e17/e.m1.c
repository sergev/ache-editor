#include "e.h"
#include "e.cc.h"
#include "e.fn.h"
#include "e.tt.h"
#include "e.se.h"
#include <sys/stat.h>
#include <fcntl.h>

extern void getprogname ();
extern void checkargs ();
extern void startup ();
extern void showhelp ();
extern void getout ();
extern void makescrfile ();
extern void getstate ();
extern void infoinit ();

extern char *getenv ();

Small numargs;
char **argarray;
extern Flag optgoto, optsearch;
#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
extern Nlines Igoto;
extern char *Isearch;
Flag helpflg,
     dbgflg;
char *dbgname;      /* argument to -debug=xxx option */
Small curarg;
Flag chosereplay;   /* user chose replay or recovery */
Flag crashed;       /* prev session was crashed */
#ifdef  SECURITY
extern Flag nomoretime;
#endif

/*    Write out an edit command to the keys file. */
static
void
keyedit (file)
char *file;
{
    Reg1 char *cp;

    cp = append ("edit ", file);
    writekeys (CCCMD, cp, CCRETURN);
    sfree (cp);
}

static
void
i_goto ()
{
    Reg1 char *cp;
    char buf[20];

    (void) sprintf (buf, "%d", Igoto);
    cp = append ("goto ", buf);
    writekeys (CCCMD, cp, CCRETURN);
    sfree (cp);
}

static
void
i_search ()
{
    writekeys (CCCMD, Isearch, CCPLSRCH);
}

static
void
setflag (name, val)
char *name;
Flag val;
{
    Reg1 char *cp;

    cp = append (val ? "" : "-", name);
    writekeys (CCCMD, cp, CCRETURN);
    sfree (cp);
}

static
void
putflags ()
{
    setflag ("inplace", inplace);
    setflag ("vistabs", visualtabs);
    setflag ("offset", offsetflg);
    setflag ("insmode", insmode);
    setflag ("binary", binary);
}

static
void
stay (go)
Flag go;
{
    if (optgoto) {
	if (!go)
	    gotomvwin ((Nlines) Igoto - 1, (Ncols) 0);
	i_goto ();
    }
    if (optsearch) {
	if (searchkey)
	    sfree (searchkey);
	searchkey = append (Isearch, "");
	if ((rex = regtest (searchkey)) < 0) {
	    sfree (searchkey);
	    searchkey = NULL;
	    return;
	}
	dosearch (1);
	i_search ();
    }
}

#define NO_WINDOWS  ' ' /* don't use any files from state file */
#define ALL_WINDOWS '!' /* use all windows and files */

/*
    All of the code prior to entering the main loop of the editor is
    either in or called by main1.
*/
void
main1 (argc, argv)
Reg3 int argc;
Reg2 char *argv[];
{
    char    ichar;      /* must be a char and can't be a register */

    numargs = argc;
    argarray = argv;
    getprogname (argv[0]);

#ifndef NOEDIAG
#if !defined(pdp11) && !defined(VAX)
    _setediag ();
#endif
#endif

    if ((STDIN = open ("/dev/tty", 2)) < 0) {
	getout (YES, ediag ("Can't open /dev/tty.\n",
			    "Нельзя открыть /dev/tty.\n"));
	/*NOTREACHED*/
    }
    STDOUT = STDIN;
    STDERR = -1;

    checkargs ();

    startup ();

    if (helpflg) {
	showhelp ();
	windowsup = NO; /* Not erase screen */
	cleanup (YES, YES);
	getout (NO, "");
    }

    if (dbgflg && (dbgfile = fopen (dbgname, "w")) == (FILE *) NULL)
	getout (YES, ediag("Can't open debug file: %s",
			    "Нельзя открыть файл отладки: %s"),
			    dbgname);

    if (replaying) Block {
	short tmp;      /* must be a short, can't be a register */
	short replterm; /* must be a short, can't be a register */

	if ((inputfile = fopen (inpfname, "r")) == (FILE *) NULL)
    OpErr:
	    getout (YES, ediag("Can't open replay file %s.",
			"Нельзя открыть файл протокола %s."),
				inpfname);
	if (!setflock (fileno (inputfile), 0, 0)) {
	    fclose (inputfile);
	    goto OpErr;
	}
	tmp = getshort (inputfile);
	if (feoferr (inputfile)) {
	    fclose (inputfile);
	    cleanup (NO, YES);
	    getout (YES, ediag("Replay file is empty.",
			       "Файл протокола пуст."));
	}
	if (tmp != revision) {
	    fclose (inputfile);
	    getout (YES, ediag(
"Replay file \"%s\" was made by revision %d of %s.",
"Файл протокола \"%s\" был сделан %d версией %s."),
		 inpfname, -tmp, progname);
	}
	ichar = getc (inputfile);
	replterm = getshort (inputfile);
	if (feoferr (inputfile)) {
	    fclose (inputfile);
	    cleanup (NO, YES);
	    getout (YES, ediag("Replay file is too short.",
				"Файл протокола слишком маленький."));
	}
	if (replterm != termtype) {
	    fclose (inputfile);
	    getout (YES, ediag(
"Replay file \"%s\" was made by a different type of terminal.",
"Файл протокола \"%s\" для другого типа терминала."),
		 inpfname, -tmp, progname);
	}
    }
    else if (curarg < argc || crashed && !chosereplay)
	ichar = NO_WINDOWS;
    else
	ichar = ALL_WINDOWS;

    if (recovering) {
	printf (ediag("\r\nRecovering from crash...\7\7",
		      "\r\nВосстановление после сбоя...\7\7"));
	fflush (stdout);
    }

    makescrfile (); /* before any calls to editfile () */

    if (!silent) {
	d_put (VCCICL);
	d_flush ();
    }

    getstate (ichar);
    putshort (revision, keyfile);
    putc (ichar, keyfile);
    putshort ((short) termtype, keyfile);
 /* mesg (TELALL);  */

    putflags ();

    infoinit ();

    if (!replaying && curarg < argc && *argv[curarg] != '\0') {
	for ( ; curarg < argc && *argv[curarg] != '\0'; curarg++) Block {
	    Nlines il;

	    keyedit (argv[curarg]);
	    il = optgoto ? (Igoto - 1) : 0;
	    if (editfile (argv[curarg], (Ncols) 0, il, 1, YES, NO) <= 0)
		(void) eddeffile (YES);
	    else
		stay (YES);
	}
    }
    else if (!replaying || ichar != NO_WINDOWS) {
	stay (NO);
	putupwin ();
    }
#ifdef  SECURITY
    if (nomoretime)
	alarm (60);
#endif
}

#if !defined(pdp11) && !defined(VAX)
static
_setediag ()
{
	char *s, *getenv (), *index ();

	_ediag = 1;
#ifdef M_XENIX
#ifdef  CHANGECTYPE
	correctype = is_russian ();
#endif
#endif
	if (
#ifdef M_XENIX
#ifdef  CHANGECTYPE
	    correctype &&
#endif
#endif
	    (!(s = getenv ("MSG"))
		|| *s && index ("rRрР", *s)))
	    _ediag = 0;
}
#endif

#ifdef M_XENIX
#include <locale.h>

Flag
is_russian ()
{
	static char Russian[] = "russian_ussr.koi8";

	return !strcmp (setlocale (LC_ALL, (char *)0), Russian);
}
#endif
