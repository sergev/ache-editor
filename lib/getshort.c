#include <stdio.h>

short
getshort (iop)
FILE *iop;
{
    short i;

    if (fread ((char *) &i, sizeof (short), 1, iop) != 1)
	return EOF;

    return i;
}
