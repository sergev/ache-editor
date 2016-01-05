#include <stdio.h>

short
putshort (i, iop)
short i;
FILE *iop;
{
    if (fwrite ((char *) &i, sizeof (short), 1, iop) != 1)
	return EOF;
    return i;
}
