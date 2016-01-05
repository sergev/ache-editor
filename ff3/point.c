#include "ff.local.h"
#include <errno.h>

#define MIN(a,b) ((a)<(b)?(a):(b))

/*
 *  We want nchars starting at addr in the file mapped into memory.
 *  Return the number of characters mapped, and store the memory address
 *  in *buf.
 */
int
ff_point (ff, addr, buf, nchars)
Ff_stream *ff;
Reg1 long addr;
char **buf;
long nchars;
{
    int offset;
    Ff_buf *ffbuf;
    long block;

    if (FF_CHKF || !(ff->f_mode & F_READ)) {
	errno = EBADF;
	return -1;
    }

    block = ldiv (addr, FF_BSIZE, &offset);

    if ((ffbuf = ff_getblk (ff->f_file, block)) == (Ff_buf *) NULL) {
	ff->f_mode |= F_IOERR;
	return -1;
    }

    *buf = &ffbuf->fb_buf[offset];

    return MIN (nchars, FF_BSIZE - offset);
}
