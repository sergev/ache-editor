#include "la.local.h"

int
la_freplace (filename, plas)
char *filename;
Reg1 La_stream *plas;
{
    Reg2 Ff_stream *tffs;

    if ((tffs = ff_open (filename, 0, 0)) == (Ff_stream *) NULL) {
	la_errno = LA_NOOPN;
	return NO;
    }

    if (ff_close (plas->la_file->la_ffs) < 0) {
	la_errno = LA_FFERR;
	return NO;
    }

    plas->la_file->la_ffs = tffs;
    return YES;
}
