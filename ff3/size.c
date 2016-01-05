#include "ff.local.h"
#include <errno.h>

long
ff_size(ff)
Reg1 Ff_stream *ff;
{
    if(FF_CHKF) {
	errno = EBADF;
	return -1L;
    }
    return ff->f_file->fn_size;
}
