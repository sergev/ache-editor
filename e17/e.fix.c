#include "e.h"
#include "e.sg.h"
#include "e.tt.h"
#ifdef  SIGNALS
#include SIG_INCL
#endif

#ifdef  M_SYSV
#include <fcntl.h>

extern int inioflags, outioflags;
#endif
void fixtty ();

/*
    Restore tty modes to original state.
*/
void
fixtty ()
{
    d_flush ();

    if (ostyflg) {
#ifndef M_SYSV
#  ifndef TIOCSETB                /* IPK fix */
	(void) ioctl (STDOUT, TIOCSETN, &outstty);
#  else   /*TIOCSETB*/
	(void) ioctl (STDOUT, TIOCSETB, &outstty);
#  endif  /*TIOCSETB*/
#else   /*M_SYSV*/
	(void) ioctl (STDOUT, TCSETAW, &out_termio);   /* System III */
	(void) fcntl (STDOUT, F_SETFL, outioflags);
#endif  /*M_SYSV*/

	ostyflg = NO;
#ifdef MESG_NO
	if (ttynstr != NULL)
	    chmod (ttynstr, (int) (07777 & oldttmode));
#endif /*MESG_NO*/
    }
    if (istyflg) {
#ifndef M_SYSV
	(void) ioctl (STDIN, TIOCSETN, &instty);
#else   /*M_SYSV*/
	(void) ioctl (STDIN, TCSETAF, &in_termio);     /* System III */
	(void) fcntl (STDIN, F_SETFL, inioflags);
#endif  /*M_SYSV*/

	istyflg = NO;
#ifndef M_SYSV
#  ifdef  CBREAK
	if (cbreakflg) {
#    ifdef  TIOCSETC
	    (void) ioctl (STDIN, TIOCSETC, &spchars);
#    endif  /*TIOCSETC*/
#    ifdef  TIOCSLTC
	    (void) ioctl (STDIN, TIOCSLTC, &lspchars);
#    endif  /*TIOCSLTC*/
	    cbreakflg = NO;
	}
#  endif  /*CBREAK*/
#else   /*M_SYSV*/
	cbreakflg = NO;
#endif  /*M_SYSV*/
    }

#if defined(SIGNALS) && defined(SIGTSTP)
    (void) signal (SIGTSTP, SIG_DFL);
#endif
}

#ifdef  SIGNALS
#ifdef SIGTSTP
/*
    Do the stop command.  Fix tty modes, stop, and fix them back on resume.
    Same function is used for both of these:
      do the stop command
      catch a stop signal
*/
/* void */
dostop ()
{
    (void) signal (SIGTSTP, dostop);
    if (replaying) return;
    screenexit (NO);                    /* clean up screen as if exiting */
    fixtty ();                          /* restore tty modes */
    kill (0, SIGTSTP);                  /* stop us */
    /* we reenter here after stop */
    setitty ();                         /* set tty modes */
    setotty ();                         /* set tty modes */
    fresh ();                           /* redraw the screen */
    windowsup = YES;                    /* windows are set up */
}
#endif  SIGTSTP
#endif  SIGNALS
