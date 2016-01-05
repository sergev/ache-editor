#include "ff.local.h"
#include <errno.h>

long ff_pos(ff)
Reg1 Ff_stream *ff;
{
    if(FF_CHKF) {
	errno = EBADF;
	return -1L;
    }
    return ff->f_offset;
}

