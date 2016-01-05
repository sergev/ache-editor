/*    input (keyboard) table parsing and lookup */

#include "e.h"
#ifdef  KBFILE
#include "e.it.h"
#ifdef UNIXV7
#include <ctype.h>
#endif
struct itable *ithead;
extern Flag knockdown;
extern unsigned in_std();

unsigned
in_file (lexp, count)
char *lexp;
int *count;
{
    Reg1 int code;
    char *inp, *outp;
    Reg2 int i;
    Reg3 Uint converted;
    Flag isknockdown;

    isknockdown = knockdown;
    inp = outp = lexp;
    while (*count > 0) {
	i = U(*inp);
	if (isknockdown || isprint (i)) {
	    *outp++ = *inp++;
	    --*count;
	    isknockdown = NO;
	    continue;
	}

	code = itget (&inp, count, ithead, outp);
	if (code >= 0) {    /* Number of characters resulting */
	    outp += code;
	    continue;
	}
	if (code == IT_MORE)
	    break;

	converted = in_std (inp, count);
	move (inp, outp, converted);
	outp += converted;

	return outp - lexp;
    }

    move (inp, outp, *count);

    return outp - lexp;
}

/*
itget (..) matches input (at *cpp) against input table
If some prefix of the input matches the table, returns the number of
   characters in the value corresponding to the matched input, stores
   the address of the value in valp, points cpp past the matching input,
   and decrements *countp by the number of characters matched.

If no match, returns IT_NOPE.

If the input matches some proper previx of an entry in the input table,
   returns IT_MORE.

cpp and countp are not changed in the last two cases.
*/

int
itget (cpp, countp, head, valp)
char **cpp;
int *countp;
struct itable *head;
char *valp;
{
    register struct itable *it;
    register char *cp;
    int count;
    int len;

    cp = *cpp;
    count = *countp;
next:
    for (it = head; it != NULLIT; it = it->it_next) {
	if (count <= 0)
	    return IT_MORE;             /* Need more input */
#ifdef  OLD_IPK_TTY                     /* Старый телевизорный драйвер ИПК */
	/* only for bd KOI8 */
	if (cyrillflg && isalpha(U(*cp)))
	    *cp &= ~0240;
	if ((cyrillflg && isalpha(U(it->it_c))) ? (it->it_c & ~0240) : it->it_c)
		== *cp)
#else
	if (it->it_c == *cp)
#endif
	{
	    cp++;
	    --count;
	    if (it->it_leaf) {
		    *cpp = cp;
		    len = it->it_len;
		    move (it->it_val, valp, len);
		    *countp = count;
		    return len;
	    }
	    else {
		head = it->it_link;
		goto next;
	    }
	}
    }
    return IT_NOPE;
}

#endif  KBFILE
