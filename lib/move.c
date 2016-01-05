/*
/*   file move.c   memory move
/**/
#include <c_env.h>
#include <localenv.h>

#ifdef VAX
char *
move (source, dest, count)
char *source, *dest;
unsigned int count;
{                                       /* MUST BE:     */
    register char         *rsource;     /* r11          */
    register char         *rdest;       /* r10          */
    register unsigned int  rcount;      /* r9           */
    unsigned int dist;
    unsigned int nsave;

    if ((rcount = count) == 0)
	return dest;
    if (rcount <= 65535) {
	asm ("        movc3   r9,*4(ap),*8(ap)");
    }
    else if (   source > dest
	     || &source[rcount] <= dest
	    ) {
	rdest = dest;
	rsource = source;
	for (;;) {
	    if (rcount <= 65535) {
		asm ("        movc3   r9,(r11),(r10)");
		break;
	    }
	    else {
		asm ("        movc3   $65535,(r11),(r10)");
		rcount -= 65535;
		rsource += 65535;
		rdest += 65535;
	    }
	}
    }
    else {
	rdest = &dest[rcount];
	rsource = &source[rcount];
	if ((dist = rdest - rsource) > 16) {
	    unsigned int nmove, resid;

	    nmove = rcount / dist;
	    resid = rcount % dist;
	    while (nmove--) {
		move (rsource -= dist, rdest -= dist, dist);
	    }
	    if (resid)
		move (rsource - resid, rdest - resid, resid);
	}
	else {
	    if ((dist & 3) == 0) {
		if ((long) rsource & 1) {
		    *--rdest = *--rsource;
		    --rcount;
		}
		if ((long) rsource & 2) {
		/*  *--((short *) rdest) = *--((short *) rsource);  */
		    asm ("        movw    -(r11),-(r10)");
		    rcount -= 2;
		}
		nsave = rcount;
		rcount >>= 2;
		do {
		/*  *--((long *) rdest) = *--((long *) rsource);    */
		    asm ("        movl    -(r11),-(r10)");
		} while (--rcount);
		if (nsave & 2) {
		/*  *--((short *) rdest) = *--((short *) rsource);  */
		    asm ("        movw    -(r11),-(r10)");
		}
		if (nsave & 1)
		    *--rdest = *--rsource;
	    }
	    else if ((dist & 1) == 0) {
		if ((long) rsource & 1) {
		    *--rdest = *--rsource;
		    --rcount;
		}
		nsave = rcount;
		rcount >>= 1;
		do {
		/*  *--((short *) rdest) = *--((short *) rsource);  */
		    asm ("        movw    -(r11),-(r10)");
		} while (--rcount);
		if (nsave & 1)
		    *--rdest = *--rsource;
	    }
	    else
		do {
		    *--rdest = *--rsource;
		} while (--rcount);
	}
    }
    return &dest[count];
}

#else   /*VAX*/
#ifdef  MEMORY
#include <memory.h>
#endif
char *
move (source, dest, count)
register char *source, *dest;
unsigned int count;
{
    register unsigned int rcount;
    unsigned int nsave;

    if ((rcount = count) == 0)
	return dest;
    /* Non-Overlapping move */
    if ( source > dest || &source[rcount] <= dest ) {
#ifdef  MEMORY
	(void) memcpy (dest, source, rcount);
	return &dest[rcount];
#else
	do {
		*dest++ = *source++;
	} while (--rcount);
	return dest;
#endif  /*MEMORY*/
    } else {
    /* Overlapping move */
	dest = &dest[rcount];
	source = &source[rcount];
	do {
		*--dest = *--source;
	} while (--rcount);
	return &dest[count];
    }
}
#endif /*VAX*/
