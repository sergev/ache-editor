#include "la.local.h"

/* VARARGS1 */
void
la_abort (str)
char *str;
{
    fprintf (stderr, "LA ABORT: %s\n", str);
    abort ();
    /* NOTREACHED */
}
