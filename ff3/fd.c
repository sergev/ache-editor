#include "ff.local.h"
#include <errno.h>

int
ff_fd(ff)
Reg1 Ff_stream *ff;
{
    if(FF_CHKF) {
	errno = EBADF;
	return -1;
    }
    return ff->f_file->fn_fd;
}

