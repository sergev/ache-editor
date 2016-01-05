#ifndef _E_H
#define _E_H
#include <c_env.h>
#include <localenv.h>
#ifdef UNIXV7
#include <sys/types.h>
#endif

#include <la.h>     /* ff.h is included by la.h */
#ifndef ANSI
#define EDIR(PROG) "/usr/local/lib/e/PROG"
#else
#define EDIR(PROG) "/usr/local/lib/e/" PROG
#endif

#define KBFILE          /* -kbfile option */

#define Block
/* Char and Short are used to point out the minimum storage required for
/*   the item.  It may be that for a given compiler, int will take up no
/*   more storage and actually execute faster for one or both of these,
/*   so they should be defined to be ints for that case.
/*   Especially, note that defining these to be ints gets around bugs in
/*   both the Ritchie PDP-11 compiler and the Johnson VAX compiler regarding
/*   declaring types smaller than int in registers.
/**/
#define Char  int
#define UChar unsigned int
#define Short int

#define Uint unsigned int

#ifdef NOSIGNEDCHAR
# define Uchar char
# define UNSCHAR
#else  /*-NOSIGNEDCHAR*/
# ifdef UNSCHAR
#   define Uchar unsigned char
# else  /*-UNSCHAR*/
#   define Uchar char
# endif /*-UNSCHAR*/
#endif /*-NOSIGNEDCHAR*/

#ifdef  UNSCHAR
# define U(c) (int)((Uchar)(c))
#else /*-UNSCHAR*/
# define U(c) ((c)&CHARMASK)
#endif  /*-UNSCHAR*/

#define Z 0
#include "e.t.h"
/* For each type there is Type and Atype.
/* use AType for array and structure member declarations,
/* and use Type everywhere else
/**/
typedef Char  Flag;             /* YES or NO */
typedef char  AFlag;            /* YES or NO */
typedef Char  Small;            /* small integer that will fit into char */
typedef char  ASmall;           /* small integer that will fit into char */
#ifdef LA_LONGFILES
typedef La_linepos Nlines;      /* number of lines in a file */
typedef La_linepos ANlines;     /* number of lines in a file */
#else
typedef La_linepos Nlines;      /* number of lines in a file */
typedef La_linepos ANlines;     /* number of lines in a file */
#endif
#ifdef LA_LONGLINES
typedef La_linesize Ncols;      /* number of columns in a line */
typedef La_linesize ANcols;     /* number of columns in a line */
#else
typedef int         Ncols;      /* number of columns in a line */
typedef La_linesize ANcols;     /* number of columns in a line */
#endif
typedef int   Fd;               /* unix file descriptor */
typedef char  AFd;              /* unix file descriptor */
typedef Char  Fn;               /* index into files we are editing */
typedef char  AFn;              /* index into files we are editing */
typedef Small Cmdret;           /* comand completion status */
typedef unsigned short Echar;

#ifdef UNIXV6
#define wait        waita       /* wait(a) waita () */
#define sgttyb      sgtty
#define sg_ispeed   sg_ispd
#define sg_ospeed   sg_ospd
#define sg_flags    sg_flag
#define ALLDELAY    (NLDELAY | TBDELAY | CRDELAY | VTDELAY)
#define B4800       12
#define st_mode     st_flags
#define S_IREAD     IREAD
#define S_IWRITE    IWRITE
#define S_IEXEC     IEXEC
#define S_IFMT      IFMT
#define S_IFDIR     IFDIR
#define S_IFCHR     IFCHR
#define S_IFBLK     IFBLK
#define S_IFREG     0
#define SIGKILL     SIGKIL
#define SIGALRM     SIGCLK
#define SIGQUIT     SIGQIT
#endif

#ifdef  NDIR
#define FNSTRLEN 255            /* max allowable filename string length */
#else   /*NDIR*/
#define FNSTRLEN 14             /* max allowable filename string length */
#endif  /*NDIR*/

#include "e.up.h"
#define feoferr(f) (feof(f) || ferror(f))
#define abs(a) ((a)<0?(-(a)):(a))
#define min(a,b) (((a) < (b))? (a): (b))
#define max(a,b) (((a) > (b))? (a): (b))
#define Goret(a) {retval= a;goto ret;}
#ifndef YES
#define YES 1
#define NO  0
#endif

# ifdef UNIXV6
# define ENAME "/bin/le" /* this will get exec'ed after forked shell */
# endif

#define RUNSAFE              /* see e.ru.c */
/*#define MESG_NO           */
#define LOGINNAME "LE"
/*#define CLEARONEXIT          /* if defined, clears screen on exit    */
# define sfree(c) cfree (c)

#define FATALEXDUMP -1          /* "exit dump" command given */
#define FATALMEM 0              /* out of memory */
#define FATALIO  1              /* fatal I/O err */
#define FATALSIG 2              /* signal caught */
#define FATALBUG 3              /* bug */
#define LAFATALBUG 4            /* bug in LA Package */
#define FATALSEC 5              /* security violation */

#define DEBUGFILE   "e.dbg"
#define CTRLCHAR   (key < 040 || 0177 == key)
#define PGMOTION (key==CCPLPAGE||key==CCPLLINE||key==CCMIPAGE||key==CCMILINE)

#define NHORIZBORDERS 2         /* number of horiz borders per window   */
#ifndef VBSTDIO
#define SMOOTHOUT               /* if defined, try to smooth output     */
				/* to terminal by doing flushing at     */
				/* line boundaries near end of buffer   */
#endif
#define TABCOL 8                /* num of cols per tab character        */

#define NENTERLINES 1           /* number of ENTER lines on screen      */
#define NINFOLINES 1            /* number of INFO lines on screen      */
#define NPARAMLINES (NENTERLINES + NINFOLINES)

#define FILEMODE 0644           /* mode of editor-created files         */
#define MAXTYP 25               /* # of modifying chars typed before    */
				/* keyfile buffer is flushed            */
#define DPLYPOL 10              /* how often to poll dintrup            */
#define SRCHPOL 50              /* how often to poll sintrup            */
#define EXECTIM 5               /* alarm time limit for Exec function   */

#define NTABS 80

/* switches for mesg() note: rightmost 3 bits are reserved for arg count */
#define TELSTRT 0010    /* Start a new message                      */
#define TELSTOP 0020    /* End of this message                      */
#define TELCLR  0040    /* Clear rest of Command Line after message */
#define TELDONE (TELSTOP | TELCLR)
#define TELALL  (TELSTRT | TELDONE)
#define TELERR  0100    /* This is an error message.                */
#define ERRSTRT (TELERR | TELSTRT)
#define ERRSTOP (TELERR | TELSTOP)  /* no more to write     */
#define ERRCLR  (TELERR | TELCLR)   /* clear rest of line   */
#define ERRDONE (ERRSTOP | ERRCLR)
#define ERRALL  (ERRSTRT | ERRDONE)


/* workspace - list per window:
/**/
typedef struct workspace {
    struct workspace *prev_wksp;          /* prev workspace             */
    struct workspace *next_wksp;          /* next workspace             */
    La_stream las;              /* lastream opened for this workspace   */
    AFn      wfile;             /* File number of file - 0 if none      */
    ASlines clin;               /* cursorline when inactive             */
    AScols   ccol;              /* curorcol when inactive               */
    ANlines wlin;               /* line no of ulhc of screen            */
    ANcols  wcol;               /* col no of column 0 of screen         */
    ANlines rngline;            /* start line of search range */
    La_stream *brnglas;         /* beginning of range, if set */
    La_stream *ernglas;         /* end of range, if set */
    ASmall wkflags;
} S_wksp;
/* S_wksp flags: */
# define RANGESET 1                /* RANGE is set */
# define WSDISP   2                /* wksp is displayed */

extern
S_wksp  *curwksp, *last_wksp, *first_wksp;


extern
La_stream *curlas;

extern Fd STDIN;
extern Fd STDOUT;
extern Fd STDERR;

#define MAXSTREAMS NOFILE
/* We need some channels for overhead stuff:
/*  0 - keyboard input
/*  1 - screen output
/*      change file
/*      (not yet: fsd file - future feature of la package)
/*      keystroke file
/*      replay input file
/*      pipe[0] --                     -- origfd
/*      pipe[1]  | for run    for save |  tempfd
/*      pipe[2] --                     --
/*      (pipe[2] not needed if RUNSAFE)
/**/
#ifdef RUNSAFE
#define WORKFILES 7
#else
#define WORKFILES 8
#endif
#define MAXOPENS (MAXSTREAMS - WORKFILES)
#define MAXFILES (MAXOPENS + 10)

extern Fd nopens;
/* There is an entry in each of these arrays for every file we have open
/* for editing.  The correct type for an array index is an Fn.
/**/
#define NULLFILE  0     /* This is handy since workspaces are calloc-ed */
#define CHGFILE   1
#define PICKFILE  2     /* file where picks go. Gets special */
			/* consideration: can't be renamed, etc.        */

#define OLDLFILE  3     /* file where closes go. Gets special */
			/* consideration: can't be renamed, etc.        */
#define NTMPFILES 2

#define FIRSTFILE PICKFILE

extern La_stream     fnlas[];       /* first La_stream open for the file    */
extern char         *tmpnames[];    /* names for tmp files */
extern char         *names[];       /* current name of the file            */
extern char         *oldnames[];    /* if != 0, orig name before renamed   */
extern S_wksp        lastlook[];
extern short         fileflags[];
#define INUSE        1              /* this array element is in use        */
#define DWRITEABLE   2              /* directory is writeable              */
#define FWRITEABLE   4              /* file is writeable                   */
#define CANMODIFY  010              /* ok to modify the file               */
#define INPLACE    020              /* to be saved in place                */
#define SAVED     0100              /* was saved during the session        */
			/* The same name can appear in more than one fn,
			/*  but only in the following combinations:
			/*  names[i] (DELETED)    == names[j] (NEW)
			/*  names[i] (DELETED)    == names[j] (RENAMED)
			/*  oldnames[j] (RENAMED) == names[j] (NEW)
			/* If (NEW | DELETED | RENAMED) == 0
			/*   file exists and we are using it               */
#define NEW       0200  /* doesn't exist yet, we want to create it         */
#define DELETED   0400  /* exists, and we'll delete it on exit             */
#define RENAMED  01000  /* exists and we will rename it on exit            */
#define UPDATE   02000  /* update this file on exit */
#define HELP     04000  /* this file is indexed help */

extern
Flag    HelpActive;

extern
Fn      curfile;

/*
 window - describes a viewing window with file
   all marg values, and ltext and ttext, are limits relative
	to (0,0) = ulhc of screen.  the other six limits are
	relative to ltext and ttext.
/**/
typedef struct window
{
    S_wksp *wksp;               /* workspace window is showing          */
    ASmall  prevwin;            /* number of the ancester win           */
				/* boundaries of text within window     */
				/*  may be same as or one inside margins*/
    ASlines ttext;              /* rel to top of full screen            */
    AScols   ltext;             /* rel to left of full screen           */
    AScols   rtext;             /*  = width - 1                         */
    ASlines btext;              /*  = height - 1                        */
    ASlines tmarg;              /*  rel to upper left of full screen    */
    AScols   lmarg;             /* margins                              */
    AScols   rmarg;
    ASlines bmarg;
    ASlines tedit;
    AScols   ledit;             /* edit window limits on screen         */
    AScols   redit;
    ASlines bedit;
    AScols  *firstcol;          /* first col containing nonblank        */
    AScols  *lastcol;           /* last col containing nonblank         */
    char   *lmchars;            /* left margin characters               */
    char   *rmchars;            /* right margin characters              */
    AFlag   winflgs;            /* flags */
} S_window;
/* S_window flags: */
#define TRACKSET 1              /* track wksp and altwksp */

#define SWINDOW (sizeof (S_window)) /* size in bytes of window */

#define MAXWINLIST 40   /* should be a linked list - not an array */
extern
S_window       *winlist[MAXWINLIST],
	       *curwin,         /* current editing win                  */
		wholescreen,    /* whole screen                         */
		infowin,        /* window for info                      */
		enterwin;       /* window for CMD and ARG               */
extern
Small   nwinlist;

#define COLMOVED    8
#define LINMOVED    4
#define WCOLMOVED   2
#define WLINMOVED   1
#define WINMOVED    (WCOLMOVED | WLINMOVED)
#define CURSMOVED   (COLMOVED | LINMOVED)

/*      savebuf - structure that describes a pick or delete buffer      */

typedef struct savebuf {
    La_stream buflas;
    ANcols  ncols;
} S_svbuf;


extern
Scols   cursorcol;              /* physical screen position of cursor   */
extern
Slines  cursorline;             /*  from (0,0)=ulhc of text in window   */

extern Small chgborders;        /* 0: don't change the borders          */
				/* 1: update them */
				/* 2: set them to inactive (dots) */

extern
unsigned numtyp;                /* number of text chars since last      */
				/* keyfile flush                        */


extern
char   *myname,
       *mypath,
       *progname;

extern
Flag    offsetflg;

extern
Flag    binary;

extern
Flag    inplace;                /* do in-place file updates?            */

extern
Flag    smoothscroll;           /* do scroll putups one line at a time */
extern
Flag    singlescroll;           /* do scrolling one line at a time */

extern ANcols *tabs;            /* array of tabstops */
extern Short   stabs;           /* number of tabs we have alloced room for */
extern Short   ntabs;           /* number of tabs set */

/* Argument to getkey is one of these: */
#define WAIT_KEY      0 /* wait for a char, ignore interrupted read calls. */
#define PEEK_KEY      1 /* peek for a char */
#define WAIT_PEEK_KEY 2 /* wait for a char, then peek at it;      */
			/* if read is interrupted, return NOCHAR. */

#define NOCHAR 0400
extern Char key;             /* last char read from tty */
extern Flag keyused;         /* last char read from tty has been used */

/* default parameters */
extern Nlines defplline,        /* default plus a line          */
	      defplpage,        /* default minus a line         */
	      defmiline,        /* default plus a page          */
	      defmipage;        /* default minus a page         */
extern Ncols  deflwin,          /* default window left          */
	      defrwin;          /* default window right         */
extern char  deffile[];         /* default filename             */
extern Fn    deffn;             /* file descriptor of default file      */

extern
Short   linewidth,              /* used by just, fill, center           */
	tmplinewidth;           /* used on return from parsing "width=xx" */

extern
char   *paramv;                 /* * globals for param read routine     */
extern
Small   paramtype;

extern
Echar   *cline;                 /* array holding current line           */
extern
Short   lcline,                 /* length of cline buffer               */
	ncline,                 /* number of chars in current line      */
	icline;                 /* increment for next expansion         */
extern
char    *deline;                /* array holding current line           */
extern
Short   ldeline,                /* length of deline buffer              */
	ideline;                /* increment for next expansion         */
extern
Flag    fcline,                 /* flag - has line been changed ?       */
	cline8,                 /* 8 bit set ? */
	ecline,                 /* line may contain ESCCHAR(s)          */
	xcline;                 /* cline was beyond end of file         */
extern
Fn      clinefn;                /* Fn of cline                          */
extern
Nlines  clineno;                /* line number in workspace of cline    */
extern
La_stream *clinelas;            /* La_stream of current line */

extern char
	prebak[],               /* text to precede/follow               */
	postbak[];              /* original name for backup file        */

extern
char *searchkey;

#ifdef  SHORTUID
extern short
#else   /*SHORTUID*/
extern char
#endif  /*SHORTUID*/
    userid,
    groupid;


extern
FILE        *keyfile;           /* channel number of keystroke file     */

extern
FILE        *inputfile;         /* channel number of command input file */

extern
Flag    intok;                  /* enable la_int ().  Normally NO, */
				/* set to YES for duration desired */
extern
Small   intrupnum;              /* how often to call intrup             */
extern
Flag    alarmed;

extern Flag windowsup;   /* screen is in use for windows */

extern Short _sizebuf;
extern Echar *_putscbuf;
extern Short _putp;
#define d_flsbuf() {d_write (_putscbuf, _putp); _putp=0;}
#define d_align() {if (_putp > 0) d_flsbuf ();}
#define d_flush() if (_putscbuf != (Echar *) NULL) {_putscbuf[_putp++] = VCCNUL; d_flsbuf ();}
#define d_put(c) {_putscbuf[_putp++] = (c); if (_putp >= _sizebuf) d_flsbuf ();}

extern
FILE      *dbgfile;

extern short revision;  /* revision number of this le */
extern short subrev;    /* sub-revision number of this le */

extern
Char evrsn;   /* the character used in the chng, strt, & keys filename   */
		/* '0', '1', ...        */

extern Flag notracks;   /* don't use or leave any strt file */
extern Flag norecover,
	    replaying,
	    recovering,
	    silent;     /* replaying silently */
extern Flag keysmoved;  /* keys file has been moved to backup */

/* these used to be in e.c only */


extern
Flag cmdmode;
extern
Flag insmode;           /* is YES when in insertmode                      */
extern
Nlines parmlines;       /* lines in numeric arg                         */
extern
Ncols parmcols;         /* columns in numeric arg e.g. 8 in "45x8"      */

extern
char *shpath;

typedef struct lookuptbl
{   char *str;
    short val;
} S_looktbl;

extern
long strttime;  /* time of start of session */

extern
Flag loginflg;  /* = 1 if this is a login process */

extern Flag ischild;    /* YES if this is a child process */

extern int zero;

/* Functions */

/* ../lib/getshort.c */
extern short getshort ();

/* ../lib/getlong.c */
extern long getlong ();

/* e.cm.c */
extern Cmdret command ();
extern Cmdret gotocmd ();
extern Cmdret doupdate ();

/* e.e.c */
extern Cmdret areacmd ();
extern Cmdret splitlines ();
/* e.f.c */
extern Flag multlinks ();
extern Flag fmultlinks ();
extern Fn hvname ();
extern Fn hvoldname ();
extern Fn hvdelname ();
/* e.la.c */
extern Flag putline ();
extern Ncols dechars ();
extern Flag extend ();
extern Nlines lincnt ();
/* e.mk.c */
extern Flag gtumark ();
extern Small exchmark ();
/* e.nm.c */
extern Cmdret name ();
extern Cmdret delete ();
extern Flag dotdot ();
/* e.p.c */
extern Small printchar ();
/* e.pa.c */
extern Small getpartype ();
extern char *getword ();
extern Cmdret scanopts ();
extern Cmdret getopteq ();
extern Cmdret doeq ();
/* e.put.c */
extern Cmdret insert ();
extern Cmdret insbuf ();
/* e.q.c */
extern Cmdret eexit ();
extern Flag saveall ();
extern Flag savestate ();
/* e.ru.c */
extern Cmdret print ();
extern Cmdret filter ();
extern Cmdret filtmark ();
extern Cmdret filtlines ();
extern Cmdret run ();
extern Cmdret runlines ();
extern Flag dowait ();
extern Flag receive ();
/* e.sb.c */
extern char *getmypath ();
extern char *gsalloc ();
extern char *salloc ();
extern char *okalloc ();
extern char *append ();
extern char *copy ();
extern char *s2i ();
extern char *itoa();
extern Flag mv ();
extern Flag okwrite ();
extern Small filecopy ();
extern int sig ();
/* e.se.c */
extern Cmdret replace ();
extern Small dsplsearch ();
extern Small strsearch ();
extern Ncols skeylen ();
/* e.sv.c */
extern Cmdret save ();
extern Flag savefile ();
extern Flag svrename ();
/* e.t.c */
extern Small vertmvwin ();
extern Small horzmvwin ();
extern Small movewin ();
extern unsigned Short getkey ();
extern Flag dintrup ();
extern Flag la_int();
extern Flag sintrup ();
/* e.tb.c */
extern Cmdret dotab ();
extern Cmdret dotabs ();
extern Small getptabs ();
extern Cmdret tabfile ();
extern Flag gettabs ();
/* e.u.c */
extern Cmdret use ();
extern Small editfile ();
extern Fn getnxfn ();
/* e.wi.c */
extern S_window *setupwindow ();
/* e.wk.c */
extern Flag swfile ();
extern int  eddeffile ();
extern Nlines botmark ();
extern void infomacro ();
extern char *getenv ();
extern void getout ();
extern Cmdret doedit();
#endif /*_E_H*/
