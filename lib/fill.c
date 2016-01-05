/*
/*   file fill.c   memory fill
/*
/**/
#include <c_env.h>
#include <localenv.h>

#ifdef VAX
char *
fill (dest, count, fillchar)
char *dest;             /*  4(ap) */
unsigned int count;     /*  8(ap) */
char fillchar;          /* 12(ap) */
{
    char *retval;

    if (count == 0)
	return dest;
    else {
	retval = &dest[count];
	for (;;) {
	    if (count <= 65535) {
		asm ("        movc5   $0,(sp),12(ap),8(ap),*4(ap)")
		break;
	    }
	    else {
		asm ("        movc5   $0,(sp),12(ap),$65535,*4(ap)")
		count -= 65535;
		dest += 65535;
	    }
	}
    }
    return retval;
}
#else   /*VAX*/
#ifdef  MEMORY
#include <memory.h>

char *
fill (dest, count, fillchar)
char *dest;
{
    if (count <= 0)
	return dest;
    (void) memset (dest, fillchar, count);
    return &dest[count];
}
#else   /*MEMORY*/
#ifdef  INT4
/*  dest should really be declared as a union, but the compilers
/*  either totally reject the idea of a union as an argument or won't put
/*  a union into a register even if it is a union of single items.
/*  Ritchie's Phototypesetter Version 7 compiler accepts dest as (char *)
/*  and generates the desired optimal code.
/**/
union bw {
    char         *b;
    short        *w;
    long         *l;
    unsigned int  i;
};

char *
fill (dest, count, fillchar)
# if defined(UNIONS_IN_REGISTERS) || defined(lint)
register union bw dest;
# else
register char *dest;
# endif
register unsigned int count;
register int fillchar;
{
    unsigned int nsave;

    if (count == 0)
	return dest.b;
    if (count < 10)
	do {
	    *dest.b++ = fillchar;
	} while (--count);
    else {
	fillchar &= CHARMASK;
	fillchar |= fillchar << CHARNBITS;
	fillchar |= fillchar << (2 * CHARNBITS);
	if (dest.i & 1) {
	    *dest.b++ = fillchar;
	    count--;
	}
	if (dest.i & 2) {
	    *dest.w++ = fillchar;
	    count -= 2;
	}
	nsave = count;
	count >>= 2;
	do {
	    *dest.l++ = fillchar;
	} while (--count);
	if (nsave & 2)
	    *dest.w++ = fillchar;
	if (nsave & 1)
	    *dest.b++ = fillchar;
    }
    return dest.b;
}
#else   /*INT4*/
/*  dest should really be declared as a union, but the compilers
/*  either totally reject the idea of a union as an argument or won't put
/*  a union into a register even if it is a union of single items.
/*  Ritchie's Phototypesetter Version 7 compiler accepts dest as (char *)
/*  and generates the desired optimal code.
/**/
union bw {
    char         *b;
    short        *w;
    unsigned int  i;
};

char *
fill (dest, count, fillchar)
# if defined(UNIONS_IN_REGISTERS) || defined(lint)
register union bw dest;
# else
register char *dest;
# endif
register unsigned int count;
register int fillchar;
{
    unsigned int nsave;

    if (count == 0)
	return dest.b;
    if (count < 10)
	do {
	    *dest.b++ = fillchar;
	} while (--count);
    else {
	fillchar &= CHARMASK;
	fillchar |= fillchar << CHARNBITS;
	if (dest.i & 1) {
	    *dest.b++ = fillchar;
	    count--;
	}
	nsave = count;
	count >>= 1;
	do {
	    *dest.w++ = fillchar;
	} while (--count);
	if (nsave & 1)
	    *dest.b++ = fillchar;
    }
    return dest.b;
}
#endif  /*INT4*/
#endif  /*MEMORY*/
#endif  /*VAX*/
