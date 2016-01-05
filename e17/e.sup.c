#include "e.h"
#include "e.tt.h"
#include "e.fn.h"
#ifdef SIGNALS
#include SIG_INCL
#endif
#include <sys/stat.h>
#ifdef  M_SYSV
#include <sys/utsname.h>
#endif

#if defined(BIGADDR) || defined(M_LDATA)
#define NUMFFBUFS       64
#else
#if defined(DATA64K) || defined(M_SDATA)
#define NUMFFBUFS       25
#else
#define NUMFFBUFS       15
#endif /*DATA64K*/
#endif /*BIGADDR*/

extern void gettermtype ();
extern void setitty ();
extern void setotty ();
extern void getout ();
extern void dorecov ();
extern char *rindex ();

extern Flag crashed;       /* prev session crashed */
extern Flag chosereplay;   /* user chose replay or recovery */
extern Flag waschoice;  /* dorecov уже вызывалась */
char *replfname;    /* argument to -replay=xxx option */
extern char *dirname;
#ifdef  SECURITY
extern int sec_timeout ();
extern Flag nomoretime;
#endif

/*    Various initializations. */
void
startup ()
{
    Reg2 char  *name;
    Reg1 Short  i;

#if   defined(M_XENIX)
    printf ("LE v%d.%02d for XENIX/%d; ", -revision, subrev,
#if   defined(M_I386)
	    386
#elif defined(M_I286)
	    286
#elif defined(M_I186)
	    186
#elif defined(M_I8086)
	    86
#else
	    -1
#endif
	   );
#elif   defined(pdp11)
    printf ("LE v%d.%02d for PDP11; ", -revision, subrev);
#elif   defined(VAX)
    printf ("LE v%d.%02d for VAX11; ", -revision, subrev);
#elif   defined(LABTAM_BUG)
    printf ("LE v%d.%02d for Genix/Labtam; ", -revision, subrev);
#elif   defined(ISC22)
    printf ("LE v%d.%02d for INTERACTIVE UNIX; ", -revision, subrev);
#elif   defined(sun)
    printf ("LE v%d.%02d for SunOS; ", -revision, subrev);
#else
    printf ("LE v%d.%02d for unknown system; ", -revision, subrev);
#endif
    printf ("Copyright (C) 1990-92 by Ache, Moscow. All rights reserved.\n");
    fflush (stdout);
#ifdef  SHORTUID
    userid = getuid ();
    groupid = getgid ();
#else
    userid  = U(getuid ());
    groupid = U(getgid ());
#endif
    /* Если стояли биты -s- */
    (void) setuid (userid);
    (void) setgid (groupid);

    Block {
	Reg1 Short i;

	for (i=0; i<MAXFILES ; i++) {
		names[i] = "";
/*
		oldnames[i] = "";
*/
	}
     }

#ifdef  SIGNALS
    for (i = 1; i <= NSIG; ++i)
	switch (i) {
#ifdef  SECURITY
	case SIGALRM:
	    signal (i, sec_timeout);
	    break;
#endif
	case SIGINT:
	    signal (i, SIG_IGN);
	    break;
#ifdef M_SYSV
#ifdef SIGCLD   /* sys V */
	case SIGCLD:
#endif /*SIGCLD*/
#else
#ifdef SIGCHLD  /* 4bsd vmunix */
	case SIGCHLD:
#endif /*SIGCHLD*/
#endif /*M_SYSV*/
#ifdef SIGTSTP  /* 4bsd vmunix */
	case SIGTSTP:
	case SIGCONT:
#endif
#ifdef SIGWINCH
	case SIGWINCH:
#endif
	    break;

	default:
	    if (signal (i, SIG_DFL) != SIG_IGN)
		(void) signal (i, sig);
	}
#endif  /*SIGNALS*/

#ifdef PROFILE
    Block {
	extern Flag profiling;
	/* only 1/8 of all editor runs */
	if (strttime & 7)
	    profil ((char *) 0, 0, 0, 0);
	else
	    profiling = YES;
    }
#endif

    getmypath ();
    if (!dirname) {
	if (!(name = rindex (mypath, '/')) || *(name + 1))
	    dirname = append (mypath, "/.le");
	else
	    dirname = append (mypath, ".le");
    }
    else
	dirname = append (dirname, "");
    if (access (dirname, 7) < 0) Block {
	char *kind;

	if (access (dirname, 0) >= 0) {
/*NOXXSTR*/
	    kind = ediag ("access", "Недоступен");
/*YESXSTR*/
	    goto GoOut;
	}
	else Block {
	    static char *args[] = {"mkdir", (char *)0, (char *)0};

	    args[1] = dirname;
	    if (syscom ("/bin/mkdir", args) != 0) {
/*NOXXSTR*/
		kind = ediag ("create", "Нельзя создать");
/*YESXSTR*/
	GoOut:
		getout (NO, ediag (
"Can't %s work directory %s,\n\
use option -directory=new_directory_name",
"%s рабочий каталог %s,\n\
используйте ключ -directory=новое_имя_каталога"), kind, dirname);
	    }
	}
    }

    Block {
	static Ff_buf ffbufhdrs[NUMFFBUFS];
	static char ffbufs[NUMFFBUFS][FF_BSIZE];

	for (i = 0; i < NUMFFBUFS; i++) {
	    ffbufhdrs[i].fb_buf = ffbufs[i];
	    ff_use (&ffbufhdrs[i], 0);
	}
    }

    gettchars ();       /* CTRL-S/CTRL-Q */
    gettermtype ();

    Block {
	extern time_t time ();

	(void) time (&strttime);
    }

    setitty ();
    setotty ();

    if (!silent) {
	d_put (VCCICL);
	d_flush ();
	windowsup = YES;
    }

#ifdef VBSTDIO
    setvbuf (stdout, (char *) NULL, _IOFBF, screensize);
#endif

    la_maxchans = MAXSTREAMS;

    /* set up cline & deline
    /**/
    cline = (Echar *) salloc ((Uint) ((lcline = icline) + 1) * sizeof (Echar), YES);
    deline = salloc ((Uint) (ldeline = ideline) + 1, YES);
#ifdef  SECURITY
    nomoretime = ChkPnt2 ();
#endif
    /* make the names of the working files */
#define PRIV (S_IREAD | S_IWRITE)
    Block {
	struct stat statbuf;

	if (stat (".", &statbuf) >= 0) Block {
	    char tmpbuf[20];

	    sprintf (tmpbuf, "/%x:%x:", statbuf.st_dev, statbuf.st_ino);
	    tmppath = append (dirname, tmpbuf);
	}
	else
	    tmppath = append (dirname, "/:");
    }

    la_nbufs = NUMFFBUFS;  /* Все буфера ff нужны только la */

    la_cfile = append (tmppath, tmpnstr);
    rfile = append (tmppath, rstr);
    keytmp  = append (tmppath, keystr);
    inpfname = bkeytmp = append (tmppath, bkeystr);

    VRSCHAR = strlen (la_cfile) - 1;

    for (evrsn = '1'; ; evrsn++) {
	if (evrsn > '9')
	    getout (NO, ediag(
"\n%s: No free temp file names left in this directory\n",
"\n%s: Не осталось свободных имен временных файлов в этом каталоге\n"),
progname);

	/* make the rest of the file names */
	la_cfile[VRSCHAR] = evrsn;
	rfile[VRSCHAR] = evrsn;
	bkeytmp[VRSCHAR] = keytmp[VRSCHAR] = evrsn;

	if (access (la_cfile, 0) == 0) Block {
	    Fd ifd;

	    if ((ifd = open (la_cfile, 0)) >= 0) {
		if (getflock (ifd)) {
		    close (ifd);
		    continue;
		}
		close (ifd);
		dorecov (NO);
		if (chosereplay)
		    crashed = YES;
		break;
	    }
	    /* exists and someone is using it */
	    else
		continue;
	}

	if (access (rfile, 0) == 0) {
	    if (!notracks && (i = open (rfile, 0)) >= 0) {
		close (i);
		break;
	    }
	    else
		continue;
	}
	break;
    }

    names[CHGFILE] = la_cfile;
    fileflags[CHGFILE] = INUSE;
    fileflags[NULLFILE] = INUSE;

    if (!crashed && !waschoice && access (keytmp, 0) == 0)
	dorecov (YES);
    if (!chosereplay)
	inpfname = replfname;

    keysmoved = mv (keytmp, bkeytmp);

    if ((keyfile = fopen (keytmp, "w")) == (FILE *) NULL)
Erk1:
	getout (YES, ediag(
"Can't create keystroke file: \"%s\".",
"Нельзя создать файл протокола: \"%s\"."),
keytmp);
    (void) chmod (fileno (keyfile), PRIV);
    if (!setflock (fileno (keyfile), 1, 0)) {
	fclose (keyfile);
	goto Erk1;
    }

    /* Set up utility windows.
    /**/
    /* wholescreen window. */
    setupwindow (&wholescreen, 0, 0,
		 term.tt_width - 1, term.tt_height - 1, NO);
    /* parameter entry window */
    setupwindow (&enterwin, 0,
		 term.tt_height - NENTERLINES - NINFOLINES,
		 term.tt_width - 1,
		 term.tt_height - 1 - NINFOLINES, NO);
    enterwin.redit = term.tt_width - 1;
    /* info display window. */
    setupwindow (&infowin, 0,
		 term.tt_height - NINFOLINES,
		 term.tt_width - 1,
		 term.tt_height - 1, NO);

    curwin = &wholescreen;
}
