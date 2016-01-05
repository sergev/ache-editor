#include <stdio.h>

long
getlong (iop)
FILE *iop;
{
    long l;

    if (fread ((char *) &l, sizeof (long), 1, iop) != 1)
	return EOF;
    return l;
}
