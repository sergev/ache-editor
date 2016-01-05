/* external definitions */
/*NOXXSTR*/
#include "e.h"
#include "e.e.h"
#include "e.cc.h"
#include "e.sg.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.hi.h"
#include "e.tt.h"

#ifndef NOEDIAG
short _ediag = 1;
#endif

Fd nopens;

La_stream     fnlas[MAXFILES];  /* first Lastream open for the file   */
char      *tmpnames[NTMPFILES] = {
    "#p", "#o"
};

char      *names[MAXFILES];
char      *oldnames[MAXFILES];
S_wksp     lastlook[MAXFILES];
short      fileflags[MAXFILES];


S_wksp  *curwksp, *first_wksp, *last_wksp;

La_stream *curlas;


Fn        curfile;

S_window       *winlist[MAXWINLIST],
	       *curwin,         /* current editing window               */
		wholescreen,	/* whole screen 			*/
		infowin,        /* window for info                      */
		enterwin;       /* window for CMD                       */
Small   nwinlist;

S_svbuf qbuf[NQBUFS];

AFn qtmpfn[NQBUFS] = {
    OLDLFILE,   /* QADJUST  */
    PICKFILE,   /* QPICK    */
    OLDLFILE,   /* QCLOSE   */
    OLDLFILE,   /* QERASE   */
    OLDLFILE,   /* QRUN     */
};


Scols   cursorcol;              /* physical screen position of cursor   */
Slines  cursorline;             /*  from (0,0)=ulhc of text in window   */

Small   chgborders = 1;

unsigned  numtyp;

/* table of motions for control chars
	UP  1	up
	DN  2	down		 : 4 or less change cursorline
	RN  3	carriage return
	HO  4	home
	RT  5	right
	LT  6	left
	TB  7	tab
	BT  8	backtab
	C1  9   first column
	RW  0   Word right  NOT IMPLEMENTED
	LW  0   Word left   NOT IMPLEMENTED
/**/
ASmall  cntlmotions[MAXMOTIONS] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    LT, TB, DN, HO, 0, RN, UP, C1,
    0, 0, 0, 0, 0, 0, RW, 0,
    LW, 0, 0, 0, 0, BT, 0, RT,
};

char   *myname,
       *mypath,
       *progname;

Flag    inplace = NO;
Flag    binary = NO;
Flag    HelpActive = NO;;

Flag    smoothscroll = NO;      /* do scroll putups one line at a time */
Flag    singlescroll = YES;     /* do scrolling one line at a time */

ANcols *tabs;                   /* array of tabstops */
Short   stabs = NTABS;          /* number of tabs we have alloced room for */
Short   ntabs = NTABS / 2;      /* number of set tabs */

Char    key;                /* last char read from tty */
Flag    keyused = YES;      /* last char read from tty has been used. */

/* parameters for line motion buttons */
Nlines  defplline = 10,         /* default plus a line         */
	defmiline = 10,         /* default minus a line        */
	defplpage = 1,          /* default plus a page         */
	defmipage = 1;          /* default minus a page        */
Ncols   deflwin  = 16,          /* default window left         */
	defrwin  = 16;          /* default window right        */
#ifndef ANSI
char    deffile[] = EDIR(errmsg); /* default filename            */
#else
char    deffile[] = EDIR("errmsg"); /* default filename            */
#endif
Fn      deffn     = -1;         /* file descriptor of default file */


Short   linewidth    = 75;      /* used by just, fill, center */
Short   tmplinewidth;           /* used on return from parsing "width=xx" */


char   *paramv;                 /* * globals for param read routine     */
Small   paramtype;





/* initialize cline to be empty */
Echar   *cline;                 /* array holding current line           */

Short   lcline,                 /* length of cline buffer               */
	ncline;                 /* number of chars in current line      */
Short   icline = 100;           /* initial increment for expansion */

char   *deline;                 /* array holding current line           */

Short   ldeline;                 /* length of deline buffer               */
Short   ideline = 100;           /* initial increment for expansion */

Flag    fcline,                 /* flag - has line been changed ?       */
	ecline,                 /* line contains DEL escapes            */
	cline8,                 /* line may contain chars with 8th bit set */
	xcline;                 /* cline was beyond end of file         */

Fn      clinefn;                /* Fn of cline                          */
Nlines  clineno;                /* line number in workspace of cline    */
La_stream *clinelas;            /* La_stream of current line */

char    prebak[]  = "",        /* text to precede/follow         */
	postbak[] = ".b";

char *searchkey;

#ifdef UNIXV7
short
#endif

#ifdef UNIXV6
char
#endif

    userid,
    groupid;

char *la_cfile = (char *) NULL;

FILE *keyfile = (FILE *) NULL;

FILE *inputfile = (FILE *) NULL;

Flag    intok;                  /* enable la_int () */
Small   intrupnum;              /* how often to call intrup             */

Flag	alarmed;

Short   _sizebuf = 0;
Echar   *_putscbuf = (Echar *) NULL;
Short   _putp = 0;

Flag windowsup = NO;

FILE *dbgfile = (FILE *) NULL;

Char evrsn;   /* '0', '1', ...        */


Flag notracks = NO;
Flag norecover = NO;
Flag replaying = NO;
Flag recovering = NO;
Flag silent = NO;       /* replaying silently */
Flag keysmoved = NO;    /* keys file has been moved to backup */

/************/
/* e.fn.h */
/* pathnames for standard files */
char   *tmppath;
#ifdef UNIXV7
char   *ttynstr;
#endif
#ifdef UNIXV6
char   *ttynstr   = "/dev/tty ";
#endif
char    scratch[] = "scratch";

char    tmpnstr[] = "c1";    /* The 1 may be replaced with a higher digit */
char    keystr[]  = "k1";
char    bkeystr[] = "k1b";
char    rstr[]    = "s1";

char   *keytmp,
       *bkeytmp,
       *rfile,
       *inpfname;

int     VRSCHAR;

/************/
/* e.sg.h */
#ifndef M_SYSV
# if 0 //def TIOCGETA                 /* IPK fix */
    struct sgttya outstty;
# else
    struct sgttyb outstty;
# endif
  struct sgttyb instty;
#else   /*M_SYSV*/
  struct termio in_termio,       /* System III ioctl */
		out_termio;
#endif  /*M_SYSV*/

#ifndef M_SYSV
# ifdef  CBREAK
#   ifdef  TIOCGETC
      struct tchars spchars;
#   endif  /*TIOCGETC*/
#   ifdef  TIOCGLTC
      struct ltchars lspchars;
#   endif  /*TIOCGLTC*/
    Flag cbreakflg = YES;
# else  /*CBREAK*/
    Flag cbreakflg = NO;
# endif /*CBREAK*/
#else   /*M_SYSV*/
  Flag cbreakflg = YES;
#endif  /*M_SYSV*/

Flag istyflg,
     ostyflg;

#ifdef MESG_NO
unsigned Short oldttmode;
#endif /*MESG_NO*/
/************/

Flag cmdmode;

Flag insmode;           /* is 1 when in insertmode */

Nlines parmlines;       /* lines in numeric arg */

Ncols parmcols;         /* columns in numeric arg e.g. 8 in "45x8"      */

char *shpath = "/bin/sh";

long strttime;	/* time of start of session */

Flag loginflg;  /* = 1 if this is a login process */

Flag ischild;

/************/
/* e.m.h */
char *cmdname;           /* the full name of the command given */
char *cmdopstr;
char *opstr;
char *nxtop;

struct loopflags loopflags;

struct markenv *curmark,
	       *prevmark;

char    mklinstr [6],
	mkcolstr [6];

Small   infoarealen;

/************/
/* e.inf.h */

Scols inf_insert;               /* "INS" */
Scols inf_inreg;                /* "R" */
Scols inf_track;                /* "TRACK" */
Scols inf_range;                /* "=RANGE" */
Scols inf_mark;                 /* "MARK" */
Scols inf_area;                 /* "30x16" etc. */
Scols inf_macro;                /* "MAC" */
Scols inf_mname;                /* "a" etc. */
/*Scols inf_at;                   /* "At"         */
Scols inf_line;                 /* line         */
Scols inf_in;                   /* "in"         */
Scols inf_file;                 /* filename     */
Scols inf_avail;

Nlines  infoline;       /* line number displayed */
Nlines  infosize;       /* all lines count */
Fn      infofile;       /* file number of filename displayed */
int     curfnum;
int     infofnum;

Nlines  marklines;      /* display of num of lines marked */
Ncols   markcols;       /* display of num of columns marked */
Nlines  cmarktop = -1;
Nlines  cmarkbot = -1;
Ncols   cmarkleft = -1;
Ncols   cmarkright = -1;

/************/
/* file e.ru.c */

char *filters[] = {
    "fill",
    "just",
    "center",
    "print"
};

char *filterpaths[] = {
#ifndef ANSI
    EDIR(fill),
    EDIR(just),
    EDIR(center),
    EDIR(print)
#else
    EDIR("fill"),
    EDIR("just"),
    EDIR("center"),
    EDIR("print")
#endif
};

char cspchars[2][NSPCHARS + 1] = {
    "()!^'",
    "{}|~`"
};
Flag uppercaseflg = NO;
Flag cyrillflg = NO;
Flag iscyr = NO;
Flag isuppercase = NO;
Flag rusbit = NO;

Flag offsetflg = YES;
Ncols ret_offset = 0;

Fd STDIN, STDOUT, STDERR;

Short rex;
Flag starthere;

Nlines srchline;/* line */
Ncols srchcol;  /* first */
Ncols srchlcol; /* last */

Echar CurrFont = IA_NORMAL;

Flag startupflg = NO;

S_cmd *hcmd;
S_cmd ccmd;
Short nhistory = MAXHISTORY;

Flag qwerty = YES;
char qwerty_in[] =
/* “≈–“œ√≈””œ“ … ÀœÕ–…Ã—‘œ“ Œ≈ –œŒ…Õ¡¿‘ some russian letters */
" !¸?\042:.‹()*+¬-¿\243\
0123456789ˆ÷‚=‡\263\
;ÊÈÛ˜ı·Ú˚ÔÏ‰¯Ù˝˙ÍÎ˘ÂÁÌ„\376ÓÒ»\\ﬂ,_\
`∆…”◊’¡–“€œÃƒÿ‘›⁄ ÀŸ≈«Õ√ﬁŒ—Ë|\377~\177";

#ifdef  CHANGECTYPE
Flag correctype = NO;
#endif


