/*    exit command and related stuff */

#include "e.h"
#include "e.cc.h"
#include "e.m.h"
#include "e.cm.h"
#ifdef SIGNALS
#include SIG_INCL
#ifdef SIGVOID
#define SIGTYPE void
#else
#define SIGTYPE int
#endif /*SIGVOID*/
#endif /*SIGNALS*/
#ifdef ENVIRON
extern char *getenv ();
#endif
static void docall ();
static void reenter ();
#ifdef  SECURITY
extern Flag nowritefile;
#endif

/*    Do the "shell" command. */
Cmdret
shell ()
{
    register char *cp;
    static char *args[] = {" shell", (char *)0};

    for (cp = cmdopstr; *cp && *cp == ' '; cp++)
	continue;

    if (*cp != '\0')
	return call ();

/*    if (saveall () == NO)
	return CROK;
*/
    docall (args);

    return CROK;
}

/*    Do the "call" command. */
Cmdret
call ()
{
    register char *cp;
    static char *args[] = {" call", "-c", (char *)0, (char *)0};

    for (cp = cmdopstr; *cp && *cp == ' '; cp++)
	continue;

    if (*cp == '\0')
	return CRNEEDARG;

/*    if (saveall () == NO)
	return CROK;
*/
    args[2] = cp;
    docall (args);

    return CROK;
}

/*
    Code called by shell() and call().
    Do all the cleanup as you would on exit.
    Fork a shell with 'args' and when it exits, re-exec editor.
*/
static
void
docall (args)
char *args[];
{
    if (replaying) return;
    screenexit (NO);                    /* clean up screen as if exiting */
    fixtty ();                          /* restore tty modes */
    putchar ('\n');
    fflush (stdout);

    syscom (shpath, args);

    /* we reenter here after stop */
    reenter ();
    setitty ();                         /* set tty modes */
    setotty ();                         /* set tty modes */
    fresh ();                           /* redraw the screen */
    windowsup = YES;                    /* windows are set up */
}


syscom (path, args)
char *path;
char *args[];
{
    int child, w;
    int retstat = -1;
    Reg1 Small j;
    SIGTYPE (*istat)(), (*qstat)();

#ifdef VFORK
    if ((child = vfork ()) != -1) {
#else
    if ((child = fork ()) != -1) {
#endif
	if (child == 0) {
	    /* Сынок */
	    for (j = 1; j <= NSIG ; j++)
		signal (j, SIG_DFL);
	    for (j = 2; j < NOFILE /*HIGHFD*/; )
		close (j++);

	    dup (fileno (stdout));
	    execv (path, args);
	    printf (ediag("Can't exec %s\n",
			  "Нельзя запустить %s\n"), path);
	    fflush (stdout);
#ifdef PROFILE
	    monexit (-1);
#else
	    _exit (-1);
#endif
	    /* NOTREACHED */
	}
	else {
	    istat = signal (SIGINT, SIG_IGN);
	    qstat = signal (SIGQUIT, SIG_IGN);
	    while ((w = wait (&retstat)) != child && w != -1)
		continue;
	    signal (SIGINT, istat);
	    signal (SIGQUIT, qstat);
	    if (w == -1) {
		printf (ediag("\r\nWaiting error\n",
			      "\r\nОшибка ожидания\n"));
		fflush (stdout);
		retstat = -1;
	    }
	}
    }
    else {
	printf (ediag("Can't fork a shell\n",
		      "Нельзя породить процесс\n"));
	fflush (stdout);
    }

    return retstat;
}


/*
    Ask the user to "Hit <RETURN> when ready to resume editing. ".
    When he has typed a <RETURN> return to caller.
*/
static
void
reenter ()
{
    Char j;

    putc (CCSTOP, keyfile);
    printf (ediag("\nHit <RETURN> when ready to resume editing. ",
"\nНажмите <ВК> для продолжения редактирования. "));
    fflush (stdout);
    while ((j = getchar ()) != EOF  && j != '\n')
	continue;
}

#define EXABORT         0
#define EXDUMP          1
#define EXNORMAL        2
#define EXNOSAVE        3
#define EXQUIT          4

/*
    Exit from the editor.
    Do appropriate saves, cleanups, etc.
    Only returns if there is a syntax error in the exit command or an error
    happened while saving files.  Otherwise it ends with a call to exit().
.
		  saves   updates      deletes     deletes
		  files  state file   keystroke    'changes'
			   (.es1)    file (.ek1)  file (.ec1)
		  -----  ----------  -----------  -----------
    exit           YES      YES          YES         YES
    exit nosave     -       YES          YES         YES
    exit quit       -        -           YES         YES
    exit abort      -        -            -          YES
    exit dump       -        -            -           -
*/
Cmdret
eexit ()
{
    Small extblind;
    static S_looktbl exittable[] = {
	"abort"   , EXABORT    ,
	"dump"    , EXDUMP     ,
	"nosave"  , EXNOSAVE   ,
	"quit"    , EXQUIT     ,
	0, 0
    };

    if (opstr[0] == '\0')
	extblind = EXNORMAL;
    else {
	if (*nxtop)
	    return CRTOOMANYARGS;
	extblind = lookup (opstr, exittable);
	if (extblind == -1  || extblind == -2) {
	    mesg (ERRSTRT + 1, opstr);
	    return (Cmdret) extblind;
	}
	extblind = exittable[extblind].val;
    }

    switch (extblind) {
    case EXNORMAL:
	if (saveall () == NO)
	    return CROK;
	break;

    case EXABORT:
    case EXDUMP:
    case EXNOSAVE:
    case EXQUIT:
	screenexit (YES);
	fixtty ();
	break;

    default:
	return CRBADARG;
    }

    switch (extblind) {
    case EXDUMP:
	fatal (FATALEXDUMP, ediag("Aborted","Прекращен"));

    case EXNORMAL:
    case EXNOSAVE:
	if (!notracks)
	    savestate ();
    case EXQUIT:
    case EXABORT:
	cleanup (YES, extblind != EXABORT);
	if (extblind == EXNORMAL) {
#ifdef PROFILE
	    monexit (0);
#else
	    exit (0);
#endif
	    /* NOTREACHED */
	}
    }
#ifdef PROFILE
    monexit (1);
#else
    exit (1);
#endif
    /* NOTREACHED */
}

/*
    Fix tty modes, put cursor at lower left.
    Do the actual modifcations to disk files:
      Save all the files that were modified and for which UPDATE is set.
      Rename files that were renamed.
      Delete files that were deleted.
    Return YES if all went OK, else NO.
*/
Flag
saveall ()
{
    Reg1 Fn i;
    Reg2 int j;

    screenexit (NO);
    fixtty ();
/*
     The strategy here is to stave off all permanent actions until as
     late as possible.  Deletes and renames are not done until necessary.
     On the other hand, according to this precept, all of the modified
     files should be saved to temp files first, and then linked or
     copied to backups, etc.  But this would take too much disk space.
     So the saves go one at a time, with some deleting and renaming along
     the way.  If we bomb during a save and any deletes or renames have
     happened, we're probably screwed if we want to replay
     delete all backup files to make disk space
     then do the saves
*/
#ifdef  SECURITY
    if (!nowritefile)
#endif
    for (j = 1; ;) {
	for (i = FIRSTFILE + NTMPFILES; i < MAXFILES; i++) {
	    if (   (fileflags[i] & (INUSE | UPDATE | DELETED))
		   == (INUSE | UPDATE)
		&& la_modified (&fnlas[i])
		&& savefile ((char *) NULL, i,
			     (fileflags[i] & INPLACE) != 0, j) == NO
	       ) {
 err:           putchar ('\n');
		reenter ();
		setitty ();
		setotty ();
		windowsup = YES;
		fresh ();
		return NO;
	    }
	}
	if (j-- == 0)
	    break;
	putline (YES);
	la_sync (NO/*YES*/);   /* flush all cached buffers out */
    }

    /* do any deleting as necessary */
    for (i = FIRSTFILE + NTMPFILES; i < MAXFILES; i++) {
	if ((fileflags[i] & (UPDATE | DELETED)) == (UPDATE | DELETED)) {
	    mesg (TELALL + 2, ediag("DELETE: ","Уничтожение: "), names[i]);
	    unlink (names[i]);
	}
    }

    /* do any renaming as necessary   */
    for (i = FIRSTFILE + NTMPFILES; i < MAXFILES; i++) {
	if (   (fileflags[i] & (UPDATE | RENAMED)) == (UPDATE | RENAMED)
	    && !svrename (i)
	   )
	    goto err;
    }

    putchar ('\n');
    fflush (stdout);
    return YES;
}

