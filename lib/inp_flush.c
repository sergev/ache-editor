#include <localenv.h>

#ifndef M_SYSV
#ifdef NOIOCTL_H
#include <sgtty.h>
#else
#include <sys/ioctl.h>
#endif
#ifdef VENIX
#undef TIOCFLUSH
#endif
#ifdef TIOCFLUSH
#include <sys/file.h>
#endif
#else   /*M_SYSV*/
#include <termio.h>
#endif  /*M_SYSV*/

inp_flush (fd)
{
#ifndef M_SYSV
#ifdef TIOCFLUSH
    int mode = FREAD;

    (void) ioctl (fd, TIOCFLUSH, &mode);
#else   /*TIOCFLUSH*/
    char c[256];

    while (!empty(fd))
	read (fd, c, sizeof (c));
#endif  /*TIOCFLUSH*/
#else   /*M_SYSV*/
    (void) ioctl (fd, TCFLSH, 0);
#endif  /*M_SYSV*/
}
