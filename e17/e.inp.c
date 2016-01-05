#include "e.h"
#include "e.cc.h"
#include "e.mk.h"
#include "e.m.h"
#include "e.tt.h"
#include "e.mac.h"
#include "e.inf.h"
#ifdef UNIXV7
#include <ctype.h>
#endif
#include <errno.h>
#if defined(SYSSELECT)
#ifdef  M_SYSV
#include <sys/select.h>
#else
#include <sys/time.h>
#endif  /*M_SYSV*/
#elif defined(SYSPOLL)
#include <stropts.h>
#include <poll.h>
#else
#include SIG_INCL
#endif

#ifdef RDNDELAY
#include <fcntl.h>
static void xflush ();
static Flag xempty ();
#else
# define xread(a,b,c) read (a, b, c)
# ifdef EMPTY
#   define xempty(fd) empty(fd)
# else
#   define xempty(fd) 1
# endif
#endif

extern Flag atoldpos;
extern Slines  newcurline;
extern Scols   newcurcol;
extern Short   iattrs;
void clrbul ();

extern int alarmproc ();

Short mcount = 0;
static char mbuf[MACLEN];

Flag knockdown  = NO;

/*
    Read another character from the input stream.  If the last character
    wasn't used up (keyused == NO) don't read after all.
    peekflg is one of
      WAIT_KEY      wait for a character, ignore interrupted read calls.
      PEEK_KEY      peek for a character
      WAIT_PEEK_KEY wait for a character, then peek at it;
		    if read times out, return NOCHAR.
*/
#if defined(SYSSELECT) || defined(SYSPOLL)
/*VARARGS1*/
#endif
unsigned Short
getkey (peekflg
#if defined(SYSSELECT) || defined(SYSPOLL)
       , timeout
#endif
       )
Reg2 Flag peekflg;
#if defined(SYSSELECT)
struct timeval *timeout;
#elif defined(SYSPOLL)
int   timeout;
#endif
{
    Reg1 unsigned Short rkey;
    unsigned Short getkey1 ();

    if (peekflg == WAIT_KEY && keyused == NO)
	return key;

    goto Again;

Minfo:
    infomacro (mread ? YES : NO, mread ? mread->imac : 0,
		       mread ? mread->inum : 0);
Again:
    if (mread != NULL && peekflg == WAIT_KEY) Block {
    /* Читаем из макро */
	char nameb[2];
	Reg3 S_macro *m = mtab[mread->imac];
	Reg5 S_nest *oldmread = mread;

	if (m == NULL) {
	    nameb[0] = mread->imac + 'a';
	    nameb[1] = '\0';
/*NOXXSTR*/
	    mesg (ERRALL + 3, ediag ("Macro `", "Макро `"), nameb,
/*YESXSTR*/
			      ediag ("' not exist.", "' не существует."));
	    goto Minfo;
	}
	switch (getkey1 (PEEK_KEY)) {   /* Может прерваться? */
	    case CCINT:
		nameb[0] = mread->imac + 'a';
		nameb[1] = '\0';
/*NOXXSTR*/
		mesg (ERRALL + 3, ediag ("Macro `", "Макро `"), nameb,
/*YESXSTR*/
			      ediag ("' interrupted.", "' прервано."));
		goto Minfo;
	}
	/* Получаем команду редактора */
	rkey = U(m->mbase[m->mlen - mread->icnt--]);

	if (mread->icnt == 0) {
	    if (--mread->inum > 0)
		mread->icnt = m->mlen;
	    else {
		m->mused = NO;
		mread = mread->iprevinp;
		if (oldmread)
		    sfree ((char *)oldmread);
	    }
	    infomacro (mread ? YES : NO, mread ? mread->imac : 0,
		       mread ? mread->inum : 0);
	}
    }
    else {
	/* Читаем с клавиатуры */
#if defined(SYSSELECT) || defined(SYSPOLL)
	rkey = getkey1 (peekflg, timeout);
#else
	rkey = getkey1 (peekflg);
#endif  /*SYSSELECT*/
	/* Получаем команду редактора */
	if (rusbit)
	    if (!qwerty) {
		if (rkey >= 0100 && rkey <= 0177)
		    rkey = tomap8 (rkey);
	    }
	    else {
		if (rkey >= 040 && rkey <= 0177)
		    rkey = tojcuk (rkey);
	    }
	if (knockdown && rkey < ' ') {
	    rkey |= 0100;
	    if (uppercaseflg && isupper (rkey))
		rkey = tolower (rkey);
	}

	if (setmacro && peekflg == WAIT_KEY) Block {
	/* Пишем в макро только с клавиатуры */
	    char nameb[2];

	    if (mnest || rkey != CCMEND) {
		if (rkey == CCINT) {
		    mesg (ERRSTRT);
	DelMacro:
		    if (mtab[setmacro - 1] != NULL) {
			sfree ((char *)mtab[setmacro - 1]);
			mtab[setmacro - 1] = NULL;
		    }
		    nameb[0] = setmacro - 1 + 'a';
		    nameb[1] = '\0';
/*NOXXSTR*/
		    mesg (ERRDONE + 3, ediag ("Macro `", "Макро `"),
/*YESXSTR*/
			  nameb, ediag ("' deleted.", "' отменен."));
		    setmacro = 0;
		    goto Minfo;
		}
		if (mcount >= MACLEN) {
		    mesg (ERRSTRT + 1, ediag ("Too many commands.",
					      "Слишком много команд."));
		    goto DelMacro;
		}
		else
		    /* Записываем команду в буфер макро */
		    mbuf[mcount++] = rkey;
		if (mnest && rkey == CCMEND) {
		    mnest--;
		    goto Again;
		}
	    }
	    else Block {
		/* Макро кончилось, переносим его из буфера */
		Reg6 char *s;

		if ((s = salloc (mcount, NO)) == NULL) {
		    mesg (ERRSTRT + 1, ediag ("Out of memory.", "Мало памяти."));
		    goto DelMacro;
		}
		else {
		    (void) move (mbuf, s, (Uint) mcount);
		    mtab[setmacro - 1]->mbase = s;
		    mtab[setmacro - 1]->mlen = mcount;
		    mtab[setmacro - 1]->mused = NO;
		}
		setmacro = 0;
		goto Minfo;
	    }
	}
    }
    if (peekflg != WAIT_KEY)
	return rkey;
    knockdown = rkey == CCCTRLQUOTE;
    keyused = NO;
    return key = rkey;
}

Flag entering = NO;     /* set if in param () routine. */
static Short lcnt = 0;
static Short lexrem = 0;

/*
    Return the next key from the input stream.
    Write the key to the keystroke file unless it is CCINT, which will NOT
    be written to the keyfile by this routine.  The caller will write it
    there if and only if it actually interrupted something.
    See getkey() for the function of peekflg.
    if peekflg is WAIT_PEEK_CHAR, then wait for timeout microseconds.
*/
#if defined(SYSSELECT) || defined(SYSPOLL)
/*VARARGS1*/
#endif
static unsigned Short
getkey1 (peekflg
#if defined(SYSSELECT) || defined(SYSPOLL)
	 , timeout
#endif
	)
Small peekflg;
#if defined(SYSSELECT)
struct timeval *timeout;
#elif defined(SYSPOLL)
int    timeout;
#endif
{
#define NREAD 256
    static char chbuf[NREAD];
    static char *lp;

    if (replaying) Block {
	static Small replaydone = 0;

	if (replaydone) {
 finishreplay:
	    fclose (inputfile);
	    replaying = NO;
	    if (silent) {
		silent = NO;
		(*term.tt_ini1) (); /* not d_put(VCCICL) because fresh() */
		windowsup = YES;
		fresh ();
	    }
	    mesg (ERRALL + 1,
		  recovering
		  ? ediag("Recovery completed.","Восстановление завершено.")
		  : replaydone == 1
		    ? ediag("Replay completed.","Повторение завершено.")
		    : ediag("Replay aborted.","Повторение превращено.")
		 );
	    recovering = NO;
	    replaydone = 0;
	    goto nonreplay;
	}
	d_flush ();
	if (   !recovering
	    && !xempty (fileno (stdin)) /* any key stops replay */
	   ) {
	    lcnt = 0;
	    replaydone = 2;
	    while (xread (fileno (stdin), chbuf, sizeof (chbuf)) > 0 &&
		   !xempty (fileno (stdin)))
		continue;
	    goto endreplay;
	}
	if (lcnt <= 0) Block {
	    static char charsaved = 0;
	    static char svchar;

	    d_flush ();
	    for (;;) Block {
		Reg1 char *cp;

		cp = lp = chbuf;
		if (charsaved)
		    *cp++ = svchar;
		if ((lcnt = fread (cp, sizeof (char), sizeof (chbuf) - charsaved, inputfile))
		    >= 0) {
		    if (lcnt > 0) {
			svchar = cp[lcnt - 1];
			if (!charsaved) {
			    charsaved = 1;
			    if (--lcnt == 0)
				continue;
			}
		    }
		    break;
		}
		if (errno != EINTR)
		    fatal (FATALIO, ediag("Error reading input, errno=%d.",
					"Ошибка ввода, errno=%d."), errno);
	    }
	}
	if (lcnt == 0 || (*lp & 0xFF) == CCSTOP) {
	    replaydone = 1;
 endreplay:
	    if (!entering)
		goto finishreplay;
	    else {
		chbuf[0] = CCINT;
		lp = chbuf;
		lcnt = 1;
	    }
	}
    }
    else {
 nonreplay:
	if (lcnt < 0)
	    fatal (FATALBUG, "lcnt < 0");
	if (   lcnt - lexrem == 0 && peekflg != PEEK_KEY
	    || lcnt < sizeof (chbuf) && !xempty (fileno (stdin))
	   ) {
	    if (lcnt == 0)
		lp = chbuf;
	    else if (lp > chbuf) {
		if (lcnt == 1)
		    chbuf[0] = *lp;
		else
		    (void) move (lp, chbuf, (Uint) lcnt);
		lp = chbuf;
	    }
	    d_flush ();
	    if (peekflg == WAIT_PEEK_KEY) Block {
#if defined(SYSSELECT)
		long z = 0;
		long readmask = 1L << fileno (stdin);

		if (select (fileno (stdin) + 1, &readmask, &z, &z, timeout) <= 0)
		    return NOCHAR;
#elif defined(SYSPOLL)
		struct pollfd fds[1];
		unsigned long nfds = 1;

		fds[0].fd = fileno (stdin);
		fds[0].events = POLLIN;

		if (poll (fds, nfds, timeout) <= 0)
		    return NOCHAR;
		else if (fds[0].revents & POLLIN == 0)
		    return NOCHAR;
#else
		while (xempty (fileno (stdin))) {
		    if (alarmed)
			return NOCHAR;
		}
#endif  /*SYSSELECT*/
	    }
	    do Block {
		Reg3 int nread;

		nread = sizeof (chbuf) - lcnt;
		if ((nread = xread (fileno (stdin), &chbuf[lcnt], nread))
		    > 0) Block {
		    Reg4 char *stcp;

		    stcp = &chbuf[lcnt -= lexrem];
		    lexrem += nread;
		    if ((nread = (*kbd.kb_inlex) (stcp, &lexrem)) > 0) Block {
			Reg1 char *cp;
			Reg2 int nr;

			cp = &stcp[nread];
			do {
			    if (*--cp == CCINT)
				break;
			} while (cp > stcp);
			if ((nr = cp - stcp) > 0) {
			    lp += lcnt + nr;
			    lcnt = nread - nr + lexrem;
			}
			else
			    lcnt += nread + lexrem;
		    }
		    else
			lcnt += lexrem;
		}
		else if (nread < 0) {
		    if (errno != EINTR)
			fatal (FATALIO, ediag("Error reading input, errno=%d.",
					    "Ошибка ввода, errno=%d."), errno);
		    if (peekflg == WAIT_PEEK_KEY)
			break;
		}
		else
		    fatal (FATALIO, ediag("Unexpected EOF in key input.",
				"Неожиданный конец файла на вводе."));
	    } while (lcnt - lexrem == 0);
	}
    }

    if (   lcnt - lexrem <= 0
	&& peekflg != WAIT_KEY
       )
	return NOCHAR;

    if (peekflg != WAIT_KEY)
	return U(*lp);

    Block {
	Reg1 char rchar;

	rchar = *lp++;
	lcnt--;
	if (keyfile != (FILE *) NULL && rchar != CCINT)
	    putc (rchar, keyfile);
	return U(rchar);
    }
}

void
flinput ()
{
    lexrem = lcnt = 0;
#ifdef RDNDELAY
    xflush (fileno (stdin));
#endif
    inp_flush (fileno (stdin));
}

#ifdef RDNDELAY

/*  These routines adapt the late-model read() with O_NDELAY
/*  to look like an empty call and a blocking read call.
/*  The implementation assumes that at most two fd's will be used this way.
/**/
struct rdbuf {
    int   rd_nread;
    AFlag rd_ndelay;
    char  rd_char;
    AFd   rd_fd;
 };
static struct rdbuf rd0, rdx;
static struct rdbuf *getrdb ();

/*    return YES if read on fd would block, else NO. */
static
Flag
xempty (fd)
Reg1 Fd fd;
{
    Reg2 struct rdbuf *rdb;

    rdb = getrdb (fd);

    if (rdb->rd_nread > 0)
	return NO;
    if (!rdb->rd_ndelay) {
	(void) fcntl (fd, F_SETFL, O_RDONLY | O_NDELAY);
	rdb->rd_ndelay = YES;
    }
    return (rdb->rd_nread = read (fd, &rdb->rd_char, 1)) <= 0;
}

/*    blocking read. */
static
int
xread (fd, buf, count)
Reg1 Fd fd;
char *buf;
int count;
{
    int retval;
    Reg2 struct rdbuf *rdb;

    rdb = getrdb (fd);
    if (count == 0)
	return 0;
    if (rdb->rd_nread > 0) {
	buf[0] = rdb->rd_char;
	count--;
    }
    else if (rdb->rd_ndelay) {
	(void) fcntl (fd, F_SETFL, O_RDONLY);
	rdb->rd_ndelay = NO;
    }
    retval = rdb->rd_nread + read (fd, &buf[rdb->rd_nread], count);
    rdb->rd_nread = 0;
    return retval;
}

static
void
xflush (fd)
Fd fd;
{
    Reg1 struct rdbuf *rdb;

    rdb = getrdb (fd);
    rdb->rd_nread = 0;
}

/*    Return a pointer to the appropriate rdbuf structure for the fd. */
static
struct rdbuf *
getrdb (fd)
Fd fd;
{
    Reg1 struct rdbuf *rdb;

    if (fd == fileno (stdin)) {
	rdb = &rd0;
	rd0.rd_fd = fd;
    }
    else {
	if (fd != rdx.rd_fd && rdx.rd_fd != 0)
	    fatal (FATALBUG, "getrdb");
	rdb = &rdx;
	rdx.rd_fd = fd;
    }
    return rdb;
}

#endif

static int bulsave = -1;    /* saved character from setbul */

/*
    Set a bullet at the current cursor postion.
    If 'wt' is non-0, set the 1-second alarm to hold the bullet in place.
*/
void
setbul (wt)
Flag wt;
{
    if (bulsave != -1)
	clrbul ();

    if (curfile != NULLFILE) Block {
	Reg1 Ncols charpos;

	getline (cursorline + curwksp->wlin);
	bulsave = (charpos = cursorcol + curwksp->wcol) < ncline - 1
		  ? cline[charpos] : ' ';
    }
    else
	bulsave = ' ';

    if (!term.tt_mr)
	putch (U(BULCHAR), YES);
    else {
	Short satt;

	d_align ();
	satt = iattrs;
	d_put ((satt & IA_MR) ? VCCMRE : VCCMR);
	putch (bulsave, YES);
	d_put ((satt & IA_MR) ? VCCMR : VCCMRE);
    }
    poscursor (cursorcol - 1, cursorline);
    /*movecursor (LT, 1);*/
    d_flush ();

    if (wt) {
	/* Если выполняется макро, не делаем задержки */
#if defined(SYSSELECT)
	if (!mread) Block {
	    static struct timeval onesec = { 1L, 0L };

	    getkey (WAIT_PEEK_KEY, &onesec);
#elif defined(SYSPOLL)
	if (!mread) Block {
	    getkey (WAIT_PEEK_KEY, 1 * 1000 /* milliseconds */);
#else
	if (!mread && getkey (PEEK_KEY) == NOCHAR) Block {
	    Reg2 int (*alarmsig) ();

	    alarmed = NO;
	    alarmsig = signal (SIGALRM, alarmproc);
	    alarm (1);
	    getkey (WAIT_PEEK_KEY);
	    alarm (0);
	    (void) signal (SIGALRM, alarmsig);
	    alarmed = NO;
#endif
	}
	clrbul ();
    }
}

/*    Clear the bullet that was placed at the current cursor position. */
void
clrbul ()
{
    if (bulsave == -1)
	return;
    if (curmark && cursorcol < rightmark (cursorcol) - curwksp->wcol)
	d_put (VCCMR);
    putch (bulsave, NO);
    if (curmark)
	d_put (VCCMRE);
    poscursor (cursorcol - 1, cursorline);
    /*movecursor (LT, 1);*/
    bulsave = -1;
}

/*    Write code1, then str, then code2 to the keys file. */
void
writekeys (code1, str, code2)
Char    code1;
char   *str;
Char    code2;
{
    if (keyfile != (FILE *) NULL) {
	putc (code1, keyfile);
	fputs (str, keyfile);
	putc (code2, keyfile);
    }
}

/*
    Look ahead for certain keys which will interrupt a putup.
    Return YES if such a key is found, else NO.
*/
Flag
dintrup ()
{
    intrupnum = 0;

    switch (getkey (PEEK_KEY)) {
	case CCINT:
	    if (!replaying)
		flinput ();
	    return YES;

	case CCOPEN:
	case CCCLOSE:
	    return atoldpos;

	case CCLWINDOW:
	case CCRWINDOW:
	case CCMIPAGE:
	case CCPLPAGE:
	case CCMILINE:
	case CCPLLINE:
	    return YES;

	case CCMOVEUP:
	    if (newcurline <= curwin->tedit + defmiline / 3 + 1)
		return YES;
	    break;

	case CCMOVEDOWN:
	    if (newcurline >= curwin->bedit - defplline / 3 - 1)
		return YES;
	    break;

	case CCMOVELEFT:
	    if (newcurcol <= curwin->ledit + deflwin / 3 + 1)
		return YES;
	    break;

	case CCMOVERIGHT:
	    if (newcurcol >= curwin->redit - defrwin / 3 - 1)
		return YES;
	    break;
    }
    return NO;
}

/*
    Look ahead for certain keys which will interrupt a search or any
    subprogram, e.g. a "run", "fill", etc.
    Return YES if such a key is found, else NO.
*/
Flag
sintrup ()
{
    intrupnum = 0;       /* reset the counter */
    if (getkey (PEEK_KEY) == CCINT) {
	putc (CCINT, keyfile);
	keyused = YES;
	getkey (WAIT_KEY);
	keyused = YES;
	if (!replaying)
	    flinput ();
	return YES;
    }
    return NO;
}

/*    If intok == 0, return NO, else return sintrup (). */
Flag
la_int()
{
    if (intok)
	return sintrup ();
    else
	return NO;
}

void
infoinreg (onoff)
Reg1 Flag onoff;
{
    static Flag wason = NO;

    if ((onoff = onoff ? YES : NO) ^ wason) {
	d_put (VCCMD);
	info (inf_inreg, 1, (wason = onoff) ? "R" : "", NO);
    }
}
