#include <localenv.h>

#ifdef EMPTY
#ifdef NOIOCTL_H
#include <sgtty.h>
#else
#define BSD_COMP
#include <sys/ioctl.h>
#endif

#ifdef FIONREAD
empty (fd)
{
    long count;

    if (ioctl (fd, FIONREAD, &count) < 0)
	return 1;
    return count <= 0;
}
#endif /* FIONREAD */
#endif /*EMPTY*/
