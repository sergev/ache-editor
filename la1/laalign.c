#include "la.local.h"

La_linepos
la_align (olas, nlas)
Reg1 La_stream *olas;
Reg2 La_stream *nlas;
{
    if (!la_verify (olas))
	return -1;
    if (olas->la_file != nlas->la_file) {
	la_errno = LA_NOTSAME;
	return -1;
    }

    (void) move ((char *) olas, (char *) nlas,
	  (unsigned) sizeof (struct la_spos));
    return nlas->la_lpos;
}
