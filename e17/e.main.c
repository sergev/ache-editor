/*    main() only */

#include "e.h"

extern void main1 ();
extern void mainloop ();

#ifdef sun
char	_sobuf[BUFSIZ];
#else
extern char _sobuf[];
#endif
#if !defined(BIGADDR) && !defined(DATA64K)
gcvt () {}      /* le не использует плавающей точки */
fcvt () {}
ecvt () {}
char    _sibuf[1];
char    _sobuf[1];

struct	_iobuf	_iob[_NFILE] = {
	{ NULL, 0, NULL, _IOREAD|_IONBF, 0},
	{ NULL, 0, NULL, _IOWRT|_IONBF, 1},
	{ NULL, 0, NULL, _IOWRT|_IONBF, 2},
};
struct	_iobuf	*_lastbuf = { &_iob[_NFILE] };
#endif

/*
    The main routine.
    First it does all of the startup stuff by calling main1, then it calls
    mainloop.  This is structured this way so that if overlays are to be
    implemented, all of the main1 stuff can be in one startup overlay that
    is discarded when it is time to call mainloop.
*/
void
main (argc, argv)
int     argc;
char   *argv[];
{
#if !defined(BIGADDR) && !defined(DATA64K)
    char    sobuf[BUFSIZ]; /* для экономии 1K данных */
#endif

    nice (-3);  /* Чуть побыстрее */

/* Обработка 3-х стандартных файлов */
#if !defined(BIGADDR) && !defined(DATA64K)
    setbuf (stdout, sobuf);
#else
    setbuf (stdout, _sobuf);
#endif
    setbuf (stdin, NULL);
    fclose (stderr);
/************************************/

    /* setuid далее (e.sup.c) */
    main1 (argc, argv);

    mainloop ();
    /* NOTREACHED */
}

