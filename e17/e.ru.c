/* functions requiring forking a subprocess */

#include "e.h"
#include "e.e.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.cm.h"
#include "e.ru.h"
#ifdef SIGNALS
#include SIG_INCL
#ifdef SIGVOID
#define SIGTYPE void
#else
#define SIGTYPE int
#endif /*SIGVOID*/
#endif /*SIGNALS*/
#ifdef  UNIXV7
#include <ctype.h>
#endif
#ifdef  SECURITY
extern Flag nowritefile;
#endif

#define CANTEXEC (-2)
/*NOXXSTR*/
static
S_looktbl filltable[] = {
    "width"   , 0       ,
    0         , 0
};
/*YESXSTR*/
extern void doexec ();
extern int alarmproc ();

/*    Common code for the "fill", "justify", and "center" commands. */
Cmdret
filter (whichfilter, closeflg)
Small   whichfilter;
Flag    closeflg;
{
    Nlines stline;
    Reg1 Cmdret tmp;
    Reg2 Cmdret retval = 0;
    char *cp;
    Cmdret fillopts ();

    if (!okwrite ())
	return NOWRITERR;

    cp = cmdopstr;
    tmplinewidth = linewidth;
    if ((tmp = scanopts (&cp, whichfilter != CENTERNAMEINDEX,
			 filltable, fillopts)) < 0)
	return tmp;
/*       3 marked area      \
/*       2 rectangle          \
/*       1 number of lines     > may stopped on an unknown option
/*       0 no area spec       /
/*      -2 ambiguous option     ) see parmlines, parmcols for type of area
/*   <= -3 other error
/**/
    if (*cp != '\0')
	return CRBADARG;
    switch (tmp) {
    case 0:
	linewidth = tmplinewidth;
	if (whichfilter == PRINTNAMEINDEX) {
	    stline = 0;
	    parmlines = la_lsize (curlas);
	}
	else {
	    parmlines = whichfilter == CENTERNAMEINDEX? 1
			: lincnt (curwksp->wlin + cursorline, 1, 2);
	    stline = curwksp->wlin + cursorline;
	}
	retval = filtlines (whichfilter,
			     stline,
			      parmlines, YES, closeflg);
	break;

    case 1:
	linewidth = tmplinewidth;
	retval = filtlines (whichfilter, curwksp->wlin + cursorline,
			    parmlines, YES, closeflg);
	break;

    case 2:
	return NORECTERR;

    case 3:
	if (markcols)
	    return NORECTERR;
	linewidth = tmplinewidth;
	retval = filtmark (whichfilter, closeflg);
	break;

    default:
	return tmp;
    }
    return retval;
}

/*
    Get the options to the "fill", "justify", etc. commands.
    For now the only option is "width=".
*/
/* not static */
Cmdret
fillopts (cp, str, tmp, equals)
char *cp;
char **str;
Small tmp;
Flag equals;
{
    if (tmp == 0) { /* "width" table entry */
	if ((tmp = doeq (&cp, &tmplinewidth)) < 0)
	    return tmp;
	*str = cp;
	return (Cmdret) 1;
    }
    return (Cmdret) -1;
}

/*
    Common code for the "fill", "justify", and "center" commands
    when there is an area marked.
*/
static
Cmdret
filtmark (whichfilter, closeflg)
Small   whichfilter;
Flag    closeflg;
{
    Reg1 Flag moved;
    Reg2 Cmdret retval = 0;

    if (markcols)
	return NORECTERR;
    moved = gtumark (YES);
	retval = filtlines (whichfilter, topmark (cursorline), marklines,
			    !moved, closeflg);
    if (moved)
	putupwin ();
    unmark ();
    return retval;
}

extern char *filters[], *filterpaths[];

/*
    Called by filter() and filtmark().
    Prepare for and call to runlines().
*/
static
Cmdret
filtlines (whichfilter, from, number, puflg, closeflg)
register Small whichfilter;
Nlines  from;
Nlines  number;
Flag    puflg;
Flag    closeflg;
{
    char *args[4];
    Reg1 Small ix;
    char buf[10];

    ix = Z;
    args[ix] = filters[whichfilter];
    switch (whichfilter) {
    case FILLNAMEINDEX:
    case JUSTNAMEINDEX:
    case CENTERNAMEINDEX:
	sprintf (buf, "-l%d", linewidth);
	args[++ix] = buf;
	break;
    case PRINTNAMEINDEX:
	break;

    default:
	mesg (TELALL + 1, "BUG!");
	return CRBADARG;
    }
    args[++ix] = (char *) NULL;

    mesg (TELALL + 2, cmdname, "...");
    loopflags.beep = YES;
    return runlines (filters[whichfilter], filterpaths[whichfilter],
		     args, from, number, QADJUST, puflg, closeflg, YES);
}

#ifdef  UNIXV7
extern char *getenv ();
#endif

/*    Do the "run" command. */
Cmdret
run (cmdstr, cmdval)
char *cmdstr;
Short cmdval;
{
    static char *shargs[] = {"sh", "-c", (char *) NULL, (char *) NULL};
    Reg1 Small j;
    Cmdret   retval;
    Nlines  tlines;
    Nlines  stline,     /* start line of exec */
	    nmlines;    /* num of lines to send to program */
    Flag   moved = NO;
    Short  cnttype = 1;
    char *cp;

    cp = cmdstr;
    j = getpartype (&cp, YES, NO, curwksp->wlin + cursorline);
    if (j == 1) {
	if (curmark)
	    return NOMARKERR;
	tlines = parmlines;
	if (tlines < 0) {
	    tlines = -tlines;
	    cnttype = 2;
	}
    }
    else if (j == 2)
	return NORECTERR;
    else if (curmark) {
	if (markcols)
	    return NORECTERR;
	moved = gtumark (YES);
	tlines = marklines;
	unmark ();
    }
    else
	tlines = (cmdval != CMDRUN) ? la_lsize (curlas) : 0;

    for (; *cp && isspace (*cp); cp++)
	continue;

    stline = curwksp->wlin + cursorline;
    nmlines = lincnt (stline, tlines, cnttype);

    mesg (TELALL + 2, ediag("RUN: ","Выполнение: "), cp);
    loopflags.beep = YES;
    shargs[2] = cp;

    retval = runlines (cmdval == CMDRUN ? "run" : "feed",
		       shpath, shargs, stline, nmlines,
		       QRUN, !moved, cmdval != CMDRUN, NO);
    if (moved)
	putupwin ();

    return retval;
}

#ifdef VFORK
Flag execfailed;
#endif

/*
    Do the fork and exec, send the lines to the execed program,
    and arrange that the stdout and stderr of the program are collected
    as one stream and inserted in the current file.
    If closeflg is non-0, delete the lines sent to the program and put them
    into iqbuf.
#ifndef RUNSAFE
    If safe, then
	e | prog >> chgfile
    else
	e | prog | EDIR(run) >> chgfile
    The only purpose served by the second case is to prevent prog from
    seeking backwards on the changes file.  It'd be nice if we could do
    that without the extra process and pipe.
#endif
*/
static
Cmdret
runlines (funcnm, progpath, args, from, number, iqbuf, puflg, closeflg
#ifndef RUNSAFE
, safe
#endif
)
char *funcnm;
char   *progpath;
char  *args[];
Nlines  from;
Nlines  number;
Small   iqbuf;
Flag    puflg;
Flag    closeflg;
#ifndef RUNSAFE
Flag    safe;       /* OK for program to write directly on changes file */
#endif
{
    Fd   pipe1[2];
    int  child1;
    long chgend;
#ifndef RUNSAFE
    int  child2;
    int  progid;
    Fd   output;
    int  pipe2[2];
#ifndef ANSI
    static char *runargs[2] = {EDIR(run), (char *) NULL};
#else
    static char *runargs[2] = {EDIR("run"), (char *) NULL};
#endif
#endif

#ifdef SECURITY
    if (nowritefile)
	return CROK;
#endif
    if (args[0] == (char *) NULL)
	args[0] = funcnm;

    if (pipe (pipe1) == -1) {
	nopipemesg ();
	return CROK;
    }
/*
     We are about to let a forked program write directly onto the end of
     the changes file.  The FF buffer cache might contain the last
     block of the changes file, and it might need to be written out, so
     we must do it now, not after we have written onto the end of the
     changes file or the new stuff might get clobbered with zeros
*/
    getline (-1);
    ff_flush (la_chgffs);
    chgend = ff_seek (la_chgffs, (long) 0, 2);
    (void) lseek (ff_fd (la_chgffs), chgend, 0);

    if (!setflock (ff_fd (la_chgffs), 2, 1)) { /* Unlock from curr to EOF */
	nolockmesg ();
	return CROK;
    }

    /* Make the process that writes onto the change file */
    /* If safe, then this is the program to be run. */
#ifdef VFORK
    execfailed = NO;
    if ((child1 = vfork ()) == -1) {
#else
    if ((child1 = fork ()) == -1) {
#endif
	close (pipe1[0]);
	close (pipe1[1]);
	noforkmesg ();
	goto ret;
    }
    if (!child1) { /* child */
	Fd chgfd;

	ischild = YES;          /* looked at by fatal (..) */
	if ((chgfd = open (la_cfile, 1)) == -1)
	    _exit (-1);
	(void) lseek (chgfd, chgend, 0);

	/* application program run by sh */
#ifndef RUNSAFE
	if (safe)
#endif
	    doexec (pipe1[0], chgfd, progpath, args);
#ifndef RUNSAFE
	else
	    doexec (pipe1[0], chgfd, runargs[0], runargs);
#endif
	/* NOTREACHED */
    }
    /* Father */
    ischild = NO;
    close (pipe1[0]);
#ifdef VFORK
    if (execfailed) {
	close (pipe1[1]);
	noexecmesg (
#ifndef RUNSAFE
	    safe ?
#endif
	    progpath
#ifndef RUNSAFE
	    : runargs[0]
#endif
	);
	goto ret;
    }
#endif /*VFORK*/

#ifndef RUNSAFE
    /* Next, if not safe, make the 'run' process between LE and
    /* the program */
    if (safe) {
	progid = child1;
	output = pipe1[1];
	child2 = 0;
    }
    else {
	if (pipe (pipe2) == -1) Block {
	    int retstat;

	    nopipemesg ();
 nofork:
	    close (pipe1[1]);
 killret:
	    kill (child1, SIGKILL);
	    while (wait (&retstat) != child1)
		continue;
	    goto ret;
	}
#ifdef VFORK
	if ((child2 = vfork ()) == -1) {
#else
	if ((child2 = fork ()) == -1) {
#endif
	    close (pipe2[0]);
	    close (pipe2[1]);
	    noforkmesg ();
	    goto nofork;
	}
	if (!child2) { /* child */
	    ischild = YES;      /* looked at by fatal (..) */
	    doexec (pipe2[0], pipe1[1], progpath, args);
	    /* NOTREACHED */
	}
#ifdef VFORK
	ischild = NO;
#endif
	close (pipe1[1]);
	close (pipe2[0]);
	output = pipe2[1];
	progid = child2;
#ifdef VFORK
	if (execfailed) {
	    close (pipe2[1]);
	    noexecmesg (progpath);
	    goto killret;
	}
#endif
    }
#endif /*RUNSAFE*/

    /* send the lines to the program */
    if (number > 0) {
	if (number > la_lsize (curlas) - from)
	    number = la_lsize (curlas) - from;
	intok = YES;
#ifdef RUNSAFE
	if (la_lflush (curlas, from, number, pipe1[1], YES, EXECTIM)
#else
	if (la_lflush (curlas, from, number, output, YES, EXECTIM)
#endif
	    < number) {
	    intok = NO;
	    execabortmesg (funcnm);
	    goto ret;
	}
	intok = NO;
    }
#ifdef RUNSAFE
    close (pipe1[1]);
#else
    close (output);
#endif

#ifdef RUNSAFE
    if (   !dowait (child1)
#else
    if (   !dowait (progid, child1, child2)
#endif
       ) {
       execabortmesg (funcnm);
       goto ret;
    }

    if (!setflock (ff_fd (la_chgffs), 1, 0)) { /* Wrlock from 0 to EOF */
	nolockmesg ();
	return CROK;
    }

    if (!receive (chgend, from, closeflg ? number : 0, iqbuf, puflg))
	nolockmesg ();

    return CROK;

ret:
    if (!setflock (ff_fd (la_chgffs), 1, 0))
	nolockmesg ();
    return CROK;
}

/*    Do the exec for runlines(). */
static
void
doexec (fd0, fd1, path, args)
Fd  fd0;
Fd  fd1;
char *path;
char *args[];
{
    Reg1 int j;

    close (0);
    dup (fd0);
    close (1);
    dup (fd1);
    close (2);
    dup (fd1);
    for (j = 3; j < NOFILE; )
	close (j++);
    execv (path, args);
#ifdef VFORK
    execfailed = YES;
#endif
    _exit (CANTEXEC);
    /* NOTREACHED */
}

#ifdef RUNSAFE
/*
    Wait for the child process to exit,
    and watch for the user to type CCINT.
    Alarms are used to periodically interrupt the wait system call so that
    we can see if the user has interrupted.
    If the user interrupts while we are waiting for the child process,
    kill it.
    Collect the exit status of the progid and print an error message if it
    is offensive.
    If interrupted by the user, return NO, else YES.
*/
#else
/*
    Wait for the child processes child1 and child2 to exit,
    and watch for the user to type CCINT.
    Alarms are used to periodically interrupt the wait system call so that
    we can see if the user has interrupted.
    If the user interrupts while we are waiting for either or both of the
    child processes, kill it or them.
    Progid tells which of child1 or child2 we are to look to for the
    exit status.
    Collect the exit status of progid and print an error message if it
    is offensive.
    If interrupted by the user, return NO, else YES.
*/
#endif
static
void
dokill (progid
#ifndef RUNSAFE
, child1, child2
#endif
)
int progid;
#ifndef RUNSAFE
Reg1 int child1;
Reg2 int child2;
#endif
{
    int retstat;

#ifdef RUNSAFE
    if (progid) {
	kill (progid, SIGKILL);
	while (wait (&retstat) != progid)
	    continue;
    }
#else
    if (child1) {
	kill (child1, SIGKILL);
	while (wait (&tretstat) != child1)
	    continue;
    }
    if (child2) {
	kill (child2, SIGKILL);
	while (wait (&tretstat) != child2)
	    continue;
    }
#endif
}

static
Flag
dowait (progid
#ifndef RUNSAFE
, child1, child2
#endif
)
int progid;
#ifndef RUNSAFE
Reg1 int child1;
Reg2 int child2;
#endif
{
#ifndef RUNSAFE
    int tretstat;
#endif
    int retstat;
    Reg4 SIGTYPE (*alarmsig) ();

    alarm (0);
    for (;;) {
	alarmed = NO;
	alarmsig = signal (SIGALRM, alarmproc);
	alarm (EXECTIM);
#ifdef RUNSAFE
	if (wait (&retstat) == progid)
	    progid = 0;
	if (!alarmed || !progid) {
#else
	do Block {
	    Reg3 int proc;

	    proc = wait (&tretstat);
	    if (proc == child1)
		child1 = 0;
	    else if (proc == child2)
		child2 = 0;
	    if (proc  == progid)
		retstat = tretstat;
	} while (!alarmed && (child1 || child2));
	if (!alarmed || (!child1 && !child2)) {
#endif
	    alarm (0);
	    alarmed = NO;
	    break;
	}
	if (sintrup ())
	    break;
    }
    (void) signal (SIGALRM, alarmsig);

    if (alarmed) {
	alarmed = NO;
	dokill (progid
#ifndef RUNSAFE
		    , child1, child2
#endif
	);
	return NO;
    }
    if (sintrup ())
	return NO;

    Block {
	Reg3 int exitstat;
	char buf[10];

	exitstat = (retstat >> 8) & 0377;
	if (exitstat == CANTEXEC) {
	    mesg (ERRALL + 1, ediag(
"Can't find program to execute.",
"Нельзя запустить программу."));
	    return YES;
	}
	if (exitstat) {
	    sprintf (buf, "%d", exitstat);
	    mesg (ERRALL + 2, ediag(
"Program exit code: ",
"Код завершения программы: "), buf);
	}
	else if (retstat & 0377) {
	    sprintf (buf, "%d", retstat & 0177);
	    mesg (ERRALL + 3, ediag(
"Program recieve signal: ",
"Программе пришел сигнал: "), buf,
/*NOXXSTR*/
retstat & 0200 ? " (core)" : "");
/*YESXSTR*/
	}
    }
    return YES;
}

/*
    Receive the output from the forked program.
    It will have been written onto the end of the changes file starting at
    chgend.  Parse it, and insert it at 'from' after closing 'nclose' lines
    and putting them into 'iqbuf' if there were any to close.
*/
static
Flag
receive (chgend, from, nclose, iqbuf, puflg)
long    chgend;
Nlines  from;
Nlines  nclose;
Small   iqbuf;
Flag    puflg;          /* putup when done */
{
    Nlines nn;
    register Nlines nins;
    Nlines lsize;
    Nlines endgap;
    La_bytepos oldbsize;
    /*  first order of business after letting another program write on the
    /*  end of the changes file is to do an la_tcollect to nail down
    /*  the new size of the changes file and collect the lines of text.
    /**/

    if (!la_tcollect (chgend)) {
	mesg (ERRALL + 1, "tcollect bug");
	return YES;
    }
    oldbsize = la_bsize (curlas);
    getline (-1);
    if ((endgap = from - (lsize = la_lsize (curlas))) > 0) {
	if (!extend (endgap)) {
	    mesg (ERRALL + 1, ediag("Can't extend the file",
			"Нельзя расширить файл"));
	    numtyp += abs (la_bsize (curlas) - oldbsize);
	    return YES;
	}
	else
	    lsize += endgap;
    }

    (void) la_lseek (curlas, from, 0);
    if (nclose <= 0 || from >= lsize) Block {
	static La_linepos zlines = 0;

	nclose = 0;
	nins = la_lrcollect (curlas, &zlines);
    }
    else Block {
	Reg1 La_stream *tlas;
	Reg2 La_stream *dlas;

	nclose = min (nclose, lsize - from);
	clean (OLDLFILE);
	nn = la_lsize (dlas = &fnlas[OLDLFILE]);
	(void) la_align (dlas, tlas = &qbuf[iqbuf].buflas);
	la_stayset (tlas);
	nins = la_lrcollect (curlas, &nclose, dlas);
	la_setrlines (tlas, nclose);
	la_stayclr (tlas);
	qbuf[iqbuf].ncols = 0;
	redisplay (OLDLFILE, nn, 0, nclose, YES);
    }
    redisplay (curfile, from, min (nins, nclose), nins - nclose, puflg);
    if (puflg)
	poscursor (cursorcol, from - curwksp->wlin);
    numtyp += abs (la_bsize (curlas) - oldbsize);
    return YES;
}

/*    Called from SIGALRM interrupt.  Merely sets the global flag 'alarmed'. */
int
alarmproc ()
{
    alarmed = YES;
}

#ifdef VFORK
static
noexecmesg (progpath)
char *progpath;
{
    mesg (ERRALL + 2, ediag("Can't execute ","Нельзя запустить "), progpath);
}
#endif

static
nopipemesg ()
{
    mesg (ERRALL + 1, ediag("Can't open a pipe.","Нельзя открыть трубу."));
}

static
noforkmesg ()
{
    mesg (ERRALL + 1, ediag("Can't fork a sub-process.",
			"Нельзя породить процесс."));
}

static
execabortmesg (name)
char *name;
{
    mesg (ERRALL + 3, "\"", name, ediag("\" command aborted.",
				"\": команда прекращена."));
}

static
nolockmesg ()
{
    mesg (ERRALL + 1, ediag("Can't handle output.",
				"Нельзя обработать вывод."));
}
