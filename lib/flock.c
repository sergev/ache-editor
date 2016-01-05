#include <fcntl.h>
#ifdef sun
#include <errno.h>
#endif


/* Sets lock from whence to EOF */
int
setflock (fd, wr, whence)
short whence;
{
    static struct flock lock = {
	F_UNLCK,
	0,
	(long) 0,
	(long) 0,
    };

    switch (wr) {
	case 0:
	    lock.l_type = F_RDLCK;
	    break;
	case 1:
	    lock.l_type = F_WRLCK;
	    break;
	case 2:
	    lock.l_type = F_UNLCK;
	    break;
	default:
	    return 0;
    }
    lock.l_whence = whence;

    return ((fcntl (fd, F_SETLK, &lock) != -1)
#ifdef sun
	   || (errno == EINVAL)
#endif
    );
}

int
getflock (fd)
{
    static struct flock lock = {
	F_UNLCK,
	0,
	(long) 0,
	(long) 0,
    };

    return (fcntl (fd, F_GETLK, &lock) != -1 && lock.l_type != F_UNLCK);
}
