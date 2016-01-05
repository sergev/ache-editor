#include <stdio.h>

long
putlong (l, iop)
long l;
FILE *iop;
{
    if (fwrite ((char *) &l, sizeof (long), 1, iop) != 1)
	return EOF;
    return l;
}
