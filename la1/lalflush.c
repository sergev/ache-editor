#include "la.local.h"
#include <errno.h>

#ifdef UNIXV7
#include <signal.h>
#ifdef SIGVOID
#define SIGTYPE void
#else
#define SIGTYPE int
#endif
#endif

#ifdef UNIXV6
#include <sys/signals.h>
#define SIGALRM SIGCLK
#define SIGTYPE int
#endif

/* VARARGS 5 */
La_linepos
la_lflush (plas, start, nlines, chan, oktoint, timeout)
La_stream *plas;
La_linepos start;
La_linepos nlines;
int chan;
int oktoint;
Reg6 int timeout;
{
    Reg5 La_stream *tlas;
    char buf[BUFSIZ];
    La_linepos totlines;
    La_stream llas;     /* can't be in a Block */
    SIGTYPE (*pipesig) ();
    SIGTYPE (*alarmsig) ();
    extern int la_lflalarm ();

    if (nlines <= 0)
	return 0;

    if ((tlas = la_clone (plas, &llas)) == (La_stream *) NULL)
	return 0;

    pipesig = signal (SIGPIPE, SIG_IGN);
    totlines = 0;
    (void) la_lseek (tlas, start, 0);
    if (!oktoint)
	timeout = 0;
    else if (timeout) {
	(void) alarm (0);
	alarmsig = signal (SIGALRM, la_lflalarm);
    }
    while (nlines > 0) Block {
	Reg2 int lcnt;
	Reg3 int cnt;
	Reg4 char *wcp;

	cnt = 0;
	for (lcnt = 0; lcnt < nlines; ) Block {
	    Reg1 int nget;
	    int ngot;

	    if ((nget = la_lrsize (tlas)) <= BUFSIZ - cnt)
		lcnt++;
	    else
		nget = BUFSIZ - cnt;
	    if ((ngot = la_lget (tlas, &buf[cnt], nget)) != nget) {
		if (ngot != -1)
		    la_errno = LA_GETERR;
		totlines = -1;
		goto ret;
	    }
	    if ((cnt += nget) >= BUFSIZ)
		break;
	}
	wcp = buf;
	for (;;) Block {
	    Reg1 int nwr;

	    if (oktoint && la_int ()) {
		la_errno = LA_INT;
		goto ret;
	    }
	    if (timeout)
		(void) alarm (timeout);
	    nwr = write (chan, wcp, cnt);
	    if (timeout)
		(void) alarm (0);
	    if (nwr == cnt)
		break;
	    else {
		if (!timeout || errno != EINTR) {
		    la_errno = LA_WRTERR;
		    goto ret;
		}
	    }
	    if (nwr > 0) {
		cnt -= nwr;
		wcp += nwr;
	    }
	}
	nlines -= lcnt;
	totlines += lcnt;
    }
 ret:
    if (timeout)
	(void) signal (SIGALRM, alarmsig);
    (void) signal (SIGPIPE, pipesig);
    (void) la_close (tlas);

    return totlines;
}

la_lflalarm ()
{
    (void) signal (SIGALRM, la_lflalarm);
}
