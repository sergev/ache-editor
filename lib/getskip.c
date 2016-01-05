#include <stdio.h>

/*
    Skip 'num' bytes in 'file'.  'Num' must be > 0.
*/
getskip (num, file)
register int num;
register FILE *file;
{
    if (num == 0)
	return;
    do
	getc (file);
    while (--num);
}
