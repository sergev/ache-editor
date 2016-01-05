#include "ff.local.h"

int
ff_sync (syssync)
{
    Reg1 Ff_stream *fp;

    for (fp = ff_streams; fp < &ff_streams[NOFFFDS]; fp++)
	if (ff_flush (fp) == -1)
	    return -1;
    if (syssync)
	sync ();
    return 0;
}
