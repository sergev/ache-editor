#include "e.h"
#include "e.tt.h"
#include "e.sg.h"
#ifdef SIGNALS
#include SIG_INCL
#endif
#ifdef MESG_NO
#include <sys/stat.h>
#endif

Flag noctrlsflg = NO;
extern Uint erase_char;  /* in standard.c */
extern Uint start_char;
extern Uint stop_char;

#ifndef M_SYSV
#ifdef TIOCGETC
struct tchars tchars = {
    0xff,       /* char t_intrc;   /* interrupt */
    0xff,       /* char t_quitc;   /* quit */
    0xff,       /* char t_startc;  /* start output */
    0xff,       /* char t_stopc;   /* stop output */
    0xff,       /* char t_eofc;    /* end-of-file */
    0xff        /* char t_brkc;    /* input delimiter (like nl) */
};
#endif  /*TIOCGETC*/
#else   /*M_SYSV*/
#include <fcntl.h>

int inioflags, outioflags;
#endif  /*M_SYSV*/

void
gettchars ()
{
#ifndef M_SYSV
    if (ioctl (STDIN, TIOCGETP, &instty) < 0) {
	iscyr = NO;
	_ediag = 1;
	cbreakflg = NO;
	return;
    }
    isuppercase = !!(instty.sg_flags & LCASE);
#ifdef CYRILL
    iscyr = !!(instty.sg_flags & CYRILL);
    if (!iscyr)
	_ediag = 1;
#endif  /*CYRILL*/
#else   /*M_SYSV*/
    if (ioctl (STDIN, TCGETA, &in_termio) < 0) {
	iscyr = NO;
	_ediag = 1;
	cbreakflg = NO;
	return;
    }
    /********************
    iscyr = !!(in_termio.c_iflag & ISTRIP)
	    && (in_termio.c_cflag & CSIZE) == CS8
	    && !!(in_termio.c_cflag & PARENB);
    *********************/
    if (ioctl (STDOUT, TCGETA, &out_termio) == 0)
	isuppercase = !!(out_termio.c_oflag & OLCUC);
#endif  /*M_SYSV*/

#ifndef M_SYSV
#ifdef  CBREAK
#ifdef  TIOCGETC
    if (ioctl (STDIN, TIOCGETC, &spchars) < 0)
	cbreakflg = NO;
    else {
	tchars.t_startc = spchars.t_startc;
	tchars.t_stopc = spchars.t_stopc;
	start_char = U(tchars.t_startc);
	stop_char = U(tchars.t_stopc);
    }
#endif  /*TIOCGETC*/
#endif  /*CBREAK*/
#else   /*M_SYSV*/
    if (in_termio.c_iflag & IXON) {
	start_char = CSTART;
	stop_char = CSTOP;
    }
#endif  /*M_SYSV*/
}

/*    Set the tty modes for the input tty. */
void
setitty ()
{
#ifndef M_SYSV
    if (ioctl (STDIN, TIOCGETP, &instty) < 0)
#else   /*M_SYSV*/
    if (   ioctl (STDIN, TCGETA, &in_termio) < 0)
#endif  /*M_SYSV*/
    {
	cbreakflg = NO;
	return;
    }
    else
	cbreakflg = YES;

#ifndef M_SYSV
#ifdef  CBREAK
#ifdef  TIOCGETC
    if (cbreakflg) {
	if (noctrlsflg)
	    tchars.t_startc = tchars.t_stopc = 0xff;
	if (ioctl (STDIN, TIOCSETC, &tchars) < 0)
	    cbreakflg = NO;
    }
#endif  /*TIOCGETC*/

#ifdef  TIOCGLTC
    if (cbreakflg) Block {
	static struct ltchars ltchars = {
	    0xff,       /* char t_suspc;   /* stop process signal */
	    0xff,       /* char t_dsuspc;  /* delayed stop process signal */
	    0xff,       /* char t_rprntc;  /* reprint line */
	    0xff,       /* char t_flushc;  /* flush output (toggles) */
	    0xff,       /* char t_werasc;  /* word erase */
	    0xff        /* char t_lnextc;  /* literal next character */
	};
	if (   ioctl (STDIN, TIOCGLTC, &lspchars) < 0
	    || ioctl (STDIN, TIOCSLTC, &ltchars) < 0
	   ) {
	    (void) ioctl (STDIN, TIOCSETC, &spchars);
	    cbreakflg = NO;
	}
    }
#endif  /*TIOCGLTC*/
#endif  /*CBREAK*/

    Block {
	Reg1 int tmpflags;

	tmpflags = instty.sg_flags;
	erase_char = U(instty.sg_erase);
#ifdef  CBREAK
	if (cbreakflg)
	    instty.sg_flags = CBREAK | (instty.sg_flags & ~(ECHO | CRMOD));
	else
#endif  /*CBREAK*/
	    instty.sg_flags = RAW | (instty.sg_flags & ~(ECHO | CRMOD));
	if (ioctl (STDIN, TIOCSETP, &instty) == 0)
	    istyflg = YES;
	else
	    cbreakflg = NO;
	instty.sg_flags = tmpflags;
    }
#else   /*M_SYSV*/
    Block {
	Reg1 unsigned short tmpiflag = in_termio.c_iflag;
	Reg2 unsigned short tmplflag = in_termio.c_lflag;
	Reg3 unsigned char vtime = in_termio.c_cc[VTIME];
	Reg4 unsigned char vmin = in_termio.c_cc[VMIN];
	Reg5 unsigned char vintr = in_termio.c_cc[VINTR];
	Reg6 unsigned char vquit = in_termio.c_cc[VQUIT];

	inioflags = fcntl (STDIN, F_GETFL, NULL);
	erase_char = in_termio.c_cc[VERASE];
	if (noctrlsflg)
	    in_termio.c_iflag &= ~IXON;
	in_termio.c_iflag &= ~(ISTRIP|ICRNL|INLCR|IGNCR|IUCLC);
	in_termio.c_lflag &= ~(ICANON|XCASE|ECHO|ECHOE|ECHOK|ECHONL);
	in_termio.c_cc[VMIN] = 1;
	in_termio.c_cc[VTIME] = 0;
	in_termio.c_cc[VINTR] = CDEL;
	in_termio.c_cc[VQUIT] = CDEL;
	if (ioctl (STDIN, TCSETA, &in_termio) == 0)
	    istyflg = YES;
	else
	    cbreakflg = NO;
	in_termio.c_iflag = tmpiflag;
	in_termio.c_lflag = tmplflag;
	in_termio.c_cc[VMIN] = vmin;
	in_termio.c_cc[VTIME] = vtime;
	in_termio.c_cc[VINTR] = vintr;
	in_termio.c_cc[VQUIT] = vquit;
    }
#endif  /*M_SYSV*/

#if defined(SIGNALS) && defined(SIGTSTP)
    Block {
	extern dostop();

	(void) signal (SIGTSTP, dostop);
    }
#endif
}

/*    Set the tty modes for the output tty. */
void
setotty ()
{
#ifdef MESG_NO
    char *ttynstr;
#endif /*MESG_NO*/

#ifndef M_SYSV
#if 1 // ndef TIOCGETA
    if (ioctl (STDOUT, TIOCGETP, &outstty) < 0)
#else
    if (ioctl (STDOUT, TIOCGETA, &outstty) < 0)
#endif /*TIOCGETA*/
#else   /*M_SYSV*/
    if (ioctl (STDOUT, TCGETA, &out_termio) < 0)
#endif  /*M_SYSV*/
	fast = YES;
    else Block {
	Reg1 int i;
#ifdef TIOCGETA                 /* IPK fix */
	Reg2 int j;
#endif  /*TIOCGETA*/
#ifdef MESG_NO
	struct stat statbuf;
#endif

#ifndef M_SYSV
	i = outstty.sg_flags;
	outstty.sg_flags &= ~(CRMOD | XTABS);
#  ifdef UCASE
	if (outstty.sg_flags & LCASE)
	    outstty.sg_flags &= ~(UCASE);
#  endif  /*UCASE*/
#  if 1 //ndef TIOCGETA                /* IPK fix */
	if (ioctl (STDOUT, TIOCSETP, &outstty) == 0) {
	    ostyflg = YES;
	    outstty.sg_flags = i;
	}
#  else   /*TIOCGETA*/
	j = outstty.sg_width;
	outstty.sg_width = 0;
	if (ioctl (STDOUT, TIOCSETA, &outstty) == 0) {
	    ostyflg = YES;
	    outstty.sg_flags = i;
	    outstty.sg_width = j;
	}
#  endif  /*TIOCGETA*/
#else   /*M_SYSV*/
	Block {
	    Reg2 unsigned short tmpoflag = out_termio.c_oflag;
	    Reg3 unsigned short tmplflag = out_termio.c_lflag;

	    outioflags = fcntl (STDOUT, F_GETFL, NULL);
	    out_termio.c_oflag &= ~(OLCUC|ONLRET|ONLCR|OCRNL|ONOCR|TAB3);
	    out_termio.c_lflag &= ~XCASE;
	    if (ioctl (STDOUT, TCSETAW, &out_termio) == 0)
		ostyflg = YES;
	    out_termio.c_oflag = tmpoflag;
	    out_termio.c_lflag = tmplflag;
	}
#endif  /*M_SYSV*/

#ifdef MESG_NO
	fstat (STDOUT, &statbuf);
	oldttmode = statbuf.st_mode;
#  ifdef TTYNAME
	if ((ttynstr = ttyname (STDOUT)) != NULL)
#  endif
#  ifdef TTYN
	if ((ttynstr[strlen (ttynstr) - 1] = ttyn (STDOUT)) != NULL)
#  endif
	    chmod (ttynstr, 0600);        /* turn off messages */
#endif  /*MESG_NO*/
#ifndef M_SYSV
	fast = (ospeed = outstty.sg_ospeed) >= B4800;
#else   /*M_SYSV*/
	fast = (ospeed = (out_termio.c_cflag & CBAUD)) >= B4800;
#endif  /*M_SYSV*/
    }
    if (!fast)
	visualtabs = NO;
}
