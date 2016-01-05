#include "ff.local.h"
#include <errno.h>

int
ff_close (ff)
Reg3 Ff_stream *ff;
{
    int clret = 0;

    if (FF_CHKF) {
	errno = EBADF;
	return -1;
    }

    if (--ff->f_count == 0) Block {
	Reg2 Ff_file *fp;

	ff->f_mode = 0;
	fp = ff->f_file;
	ff->f_file = 0;
	if (--fp->fn_refs == 0) Block {
	    Reg1 Ff_buf *fb;

	    if (fp->fn_mode & F_WRITE)
		ff_sort (fp);
	    while (fb = fp->fb_qf)
		if (ff_putblk (fb, 1) == (Ff_buf *) NULL)
		    return -1;
	    clret = close (fp->fn_fd);
	    fp->fn_fd = -1;
#ifdef  EUNICE
	    fp->fn_memaddr = (char *) NULL;
#endif  EUNICE
	}
    }
    return clret < 0 ? -1 : 0;
}
