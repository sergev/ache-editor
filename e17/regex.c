/*
	  The regular expressions allowed by ed are constructed as
	  follows:

	  The following	one-character regular expressions match	a
	  single character:

	  1.1	 An ordinary character (not one	of those discussed in
		 1.2 below) is a one-character regular expression that
		 matches itself.

	  1.2	 A backslash (\) followed by any special character is
		 a one-character regular expression that matches the
		 special character itself.  The	special	characters
		 are:

		 a.    ., *, [,	and \ (dot, star, left square bracket,
		       and backslash, respectively), which are always
		       special,	except when they appear	within square
		       brackets	([]; see 1.4 below).

		 b.    ^ (caret), which	is special at the beginning of
		       an entire regular expression (see 3.1 and 3.2
		       below), or when it immediately follows the left
		       of a pair of square brackets ([]) (see 1.4
		       below).

		 c.    $ (dollar sign),	which is special at the	end of
		       an entire regular expression (see 3.2 below).

		 d.    The character used to bound (i.e., delimit) an
		       entire regular expression, which	is special for
		       that regular expression (for example, see how
		       slash (/) is used in the	g command below).

	  1.3	 A period (.) is a one-character regular expression
		 that matches any character except newline.

	  1.4	 A nonempty string of characters enclosed in square
		 brackets ([]) is a one-character regular expression
		 that matches any one character	in that	string.	 If,
		 however, the first character of the string is a caret
		 (^), the one-character	regular	expression matches any
		 character except newline and the remaining characters
		 in the	string.	 The star (*) has this special meaning
		 only if it occurs first in the	string.	 The dash (-)
		 may be	used to	indicate a range of consecutive	ASCII
		 characters; for example, [0-9]	is equivalent to
		 [0123456789].	The dash (-) loses this	special
		 meaning if it occurs first (after an initial caret
		 (^), if any) or last in the string.  The right	square
		 bracket (]) does not terminate	such a string when it
		 is the	first character	within it (after an initial
		 caret (^), if any); e.g., []a-f] matches either a
		 right square bracket (]) or one of the	letters	``a''
		 through ``f'' inclusive.  Dot,	star, left bracket,
		 and the backslash lose	their special meaning within
		 such a	string of characters.

	  The following	rules may be used to construct regular
	  expressions from one-character regular expressions:

	  2.1	 A one-character regular expression matches whatever
		 the one-character regular expression matches.

	  2.2	 A one-character regular expression followed by	a star
		 (*) is	a regular expression that matches zero or more
		 occurrences of	the one-character regular expression.
		 If there is any choice, the longest leftmost string
		 that permits a	match is chosen.

	  2.3	 A one-character regular expression followed by	\{m\},
		 \{m,\}, or \{m,n\} is a regular expression that
		 matches a range of occurrences	of the one-character
		 regular expression.  The values of m and n must be
		 nonnegative integers less than	255; \{m\} matches
		 exactly m occurrences;	\{m,\} matches at least	m
		 occurrences; \{m,n\} matches any number of
		 occurrences between m and n, inclusive.  Whenever a
		 choice	exists,	the regular expression matches as many
		 occurrences as	possible.

	  2.4	 The concatenation of regular expressions is a regular
		 expression that matches the concatenation of the
		 strings matched by each component of the regular
		 expression.

	  2.5	 A regular expression enclosed between the character
		 sequences \( and \) is	a regular expression that
		 matches whatever the unadorned	regular	expression
		 matches.  See 2.6 below for a discussion of why this
		 is useful.

	  2.6	 The expression	\n matches the same string of
		 characters as was matched by an expression enclosed
		 between \( and	\) earlier in the same regular
		 expression.  Here n is	a digit; the subexpression
		 specified is that beginning with the n-th occurrence
		 of \( counting	from the left.	For example, the
		 expression ^\(.*\)\1$ matches a line consisting of
		 two repeated appearances of the same string.

	  Finally, an entire regular expression	may be constrained to
	  match	only an	initial	segment	or final segment of a line (or
	  both):

	  3.1	 A caret (^) at	the beginning of an entire regular
		 expression constrains that regular expression to
		 match an initial segment of a line.

	  3.2	 A dollar sign ($) at the end of an entire regular
		 expression constrains that regular expression to
		 match a final segment of a line.  The construction
		 ^entire regular expression$ constrains	the entire
		 regular expression to match the entire	line.

	  The null regular expression is equivalent to the
	  last regular expression encountered.
 */
#include "e.h"

#ifdef UNIXV7
#include <ctype.h>
#endif

#define INIT Reg1 char *sp = instring;
#define GETC() (*sp++)
#define PEEKC() (*sp)
#define UNGETC(c) (--sp)
#define RETURN(c) return 0;
#define ERROR(c) return regerr(c);
static char *regerr();
static void getrnge();

#define	CBRA	2
#define	CCHR	4
#define	CDOT	8
#define	CCL	12
#define	CDOL	20
#define	CCEOF	22
#define	CKET	24
#define	CBACK	36

#define	STAR	01
#define	RNGE	03

#define	NBRA	9

#define	PLACE(c)	ep[(c >> 3) & 037] |= bittab[c & 07]
#define	ISTHERE(c)	(ep[(c >> 3) & 037] & bittab[c & 07])

static  Echar   *braslist[NBRA];
static  Echar   *braelist[NBRA];
static  int     nbra, ebra;
static  Echar   *loc1, *loc2;
static  Echar   *locs = (Echar *) NULL;
static  int     sed;
static  int     nodelim;

static  int     circf;
static  int     dollf;
static  int     low;
static  int     size;

static  char    bittab[] = {
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128
};

/*
 * compile the regular expression argument into a dfa
 */
char *
le_comp(instring, ep, endbuf)
Reg2 char *ep;
char *instring, *endbuf;
{
	INIT	/* Dependent declarations and initializations */
	Reg3 int c;
	Reg4 int eof = '\0';
	char *lastep = instring;
	int cclcnt;
	char bracket[NBRA], *bracketp;
	int closed;
	char neg;
	int lc;
	int i, cflg;

	lastep = 0;
	if((c = GETC()) == eof || c == '\n') {
		if(c == '\n') {
			UNGETC(c);
			nodelim = 1;
		}
		if(*ep == 0 && !sed)
			ERROR(41);
		RETURN(ep);
	}
	bracketp = bracket;
	dollf = circf = closed = nbra = ebra = 0;
	if(c == '^')
		circf++;
	else
		UNGETC(c);
	while(1) {
		if(ep >= endbuf)
			ERROR(50);
		c = GETC();
		if(c != '*' && ((c != '\\') || (PEEKC() != '{')))
			lastep = ep;
		if(c == eof) {
			*ep++ = CCEOF;
			RETURN(ep);
		}
		switch(c) {

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':
			if(!sed) {
				UNGETC(c);
				*ep++ = CCEOF;
				nodelim = 1;
				RETURN(ep);
			}
			else ERROR(36);
		case '*':
			if(lastep==0 || *lastep==CBRA || *lastep==CKET)
				goto defchar;
			*lastep |= STAR;
			continue;

		case '$':
			if(PEEKC() != eof && PEEKC() != '\n')
				goto defchar;
			*ep++ = CDOL;
			dollf++;
			continue;

		case '[':
			if(&ep[33] >= endbuf)
				ERROR(50);

			*ep++ = CCL;
			lc = 0;
			for(i = 0; i < 32; i++)
				ep[i] = 0;

			neg = 0;
			if((c = GETC()) == '^') {
				neg++;
				c = GETC();
			}

			do {
				short first_c, last_c, curr_c;

				c = U(c);
				if (c == '\0' || c == '\n')
					ERROR(49);
				if (c == '-' && lc != 0) {
					if((c = GETC()) == ']') {
						PLACE('-');
						break;
					}
					first_c = Ctou (lc);
					last_c = Ctou (c);
					for (i = first_c; i < last_c; i++) {
						curr_c = Ctok (i);
						PLACE(curr_c);
					}
				}
				if(c == '\\') {
					switch(c = GETC()) {
						case 'n':
							c = '\n';
							break;
					}
					c = U(c);
				}
				lc = c;
				PLACE(c);
			} while((c = GETC()) != ']');
			if(neg) {
				for(cclcnt = 0; cclcnt < 32; cclcnt++)
					ep[cclcnt] ^= ~0;
				ep[0] &= 0376;
			}

			ep += 32;

			continue;

		case '\\':
			switch(c = GETC()) {

			case '(':
				if(nbra >= NBRA)
					ERROR(43);
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;

			case ')':
				if(bracketp <= bracket || ++ebra != nbra)
					ERROR(42);
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;

			case '{':
				if(lastep == (char *) (0))
					goto defchar;
				*lastep |= RNGE;
				cflg = 0;
			nlim:
				c = GETC();
				i = 0;
				do {
					if (isdigit(c))
						i = 10 * i + (c - '0');
					else
						ERROR(16);
				} while(((c = GETC()) != '\\') && (c != ','));
				if(i >= 255)
					ERROR(11);
				*ep++ = i;
				if(c == ',') {
					if(cflg++)
						ERROR(44);
					if((c = GETC()) == '\\')
						*ep++ = 255;
					else {
						UNGETC(c);
						goto nlim;
						/* get 2'nd number */
					}
				}
				if(GETC() != '}')
					ERROR(45);
				if(!cflg)	/* one number */
					*ep++ = i;
				else if(U(ep[-1]) < U(ep[-2]))
					ERROR(46);
				continue;

			case '\n':
				ERROR(36);

			case 'n':
				c = '\n';
				goto defchar;

			default:
				if (isdigit(c) && c != '0') {
					if((c -= '1') >= closed)
						ERROR(25);
					*ep++ = CBACK;
					*ep++ = c;
					continue;
				}
			}
	/* Drop through to default to use \ to turn off special chars */

		defchar:
		default:
			lastep = ep;
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

/*
 * match the argument string against the compiled re
 */
int
le_exec (p1, p2, pe, beg, end, delta)
Reg1   Echar   *p1;
Reg2   Uchar   *p2;
Reg3   Echar   *pe;
Flag   beg, end;
Small  delta;
{
	Reg4 Echar *pp;
	Reg5 Uchar c;
	int rv;

	if (dollf && !end || circf && !beg)
	    return 0;
	for (c = 0; c < NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	if(circf) {
		loc1 = p1;
		return advance(p1, p2);
	}
	/* fast check for first character */
	if(*p2==CCHR) {
		c = p2[1];
		for (pp = delta > 0 ? p1 : pe;
		     pp >= p1 && pp <= pe;
		     pp += delta) {
			if(U(*pp) != U(c))
				continue;
			if (rv = advance(pp, p2)) {
				loc1 = pp;
				return rv;
			}
		}
		return(0);
	}

	/* regular algorithm */

	for (pp = delta > 0 ? p1 : pe;
	     pp >= p1 && pp <= pe;
	     pp += delta)
		if(rv = advance(pp, p2)) {
			loc1 = pp;
			return rv;
		}

	return(0);
}

static
int
advance(lp, ep)
Reg1 Echar *lp;
Reg2 Uchar *ep;
{
	Reg3 Echar *curlp;
	Uchar c;
	Echar *bbeg, *bend;
	int ct, i;

	while(1)
		switch(*ep++) {

		case CCHR:
			if(U(*ep++) == U(*lp++))
				continue;
			return(0);

		case CDOT:
			if(U(*lp++) != '\0')
				continue;
			return(0);

		case CDOL:
			if(U(*lp) == '\0')
				continue;
			return(0);

		case CCEOF:
			loc2 = lp;
			return(1);

		case CCL:
			c = U(*lp++);
			if(ISTHERE(c)) {
				ep += 32;
				continue;
			}
			return(0);
		case CBRA:
			braslist[*ep++] = lp;
			continue;

		case CKET:
			braelist[*ep++] = lp;
			continue;

		case CCHR|RNGE:
			c = *ep++;
			getrnge(ep);
			while(low--)
				if(U(*lp++) != U(c))
					return(0);
			curlp = lp;
			while(size--) 
				if(U(*lp++) != U(c))
					break;
			if(size < 0)
				lp++;
			ep += 2;
			goto star;

		case CDOT|RNGE:
			getrnge(ep);
			while(low--)
				if(U(*lp++) == '\0')
					return(0);
			curlp = lp;
			while(size--)
				if(U(*lp++) == '\0')
					break;
			if(size < 0)
				lp++;
			ep += 2;
			goto star;

		case CCL|RNGE:
			getrnge(ep + 32);
			while(low--) {
				c = U(*lp++);
				if(!ISTHERE(c))
					return(0);
			}
			curlp = lp;
			while(size--) {
				c = U(*lp++);
				if(!ISTHERE(c))
					break;
			}
			if(size < 0)
				lp++;
			ep += 34;		/* 32 + 2 */
			goto star;

		case CBACK:
			bbeg = braslist[*ep];
			bend = braelist[*ep++];
			if (!bbeg || !bend)
				return -1;
			ct = bend - bbeg;
			for (i = 0; i < ct; i++)
			    if (U(bbeg[i]) != U(lp[i]))
				return 0;
			lp += ct;
			continue;

		case CBACK|STAR:
			bbeg = braslist[*ep];
			bend = braelist[*ep++];
			if (!bbeg || !bend)
				return -1;
			ct = bend - bbeg;
			curlp = lp;
			for (; ; lp += ct) {
			    for (i = 0; i < ct; i++)
				if (U(bbeg[i]) != U(lp[i]))
				    goto EndCmp;
			}
		EndCmp:
			while(lp >= curlp) {
				if(advance(lp, ep))	return(1);
				lp -= ct;
			}
			return(0);


		case CDOT|STAR:
			curlp = lp;
			while(U(*lp++) != '\0');
			goto star;

		case CCHR|STAR:
			curlp = lp;
			while(U(*lp++) == U(*ep));
			ep++;
			goto star;

		case CCL|STAR:
			curlp = lp;
			do {
				c = U(*lp++);
			} while(ISTHERE(c));
			ep += 32;
		star:
			do {
				if (--lp == locs)
					break;
				if(advance(lp, ep))
					return(1);
			} while(lp > curlp);
			return(0);

		}
}

static
void
getrnge(str)
Reg1 Uchar *str;
{
	low = U(*str++);
	size = (U(*str) == 255) ? 20000 : U(*str) - low;
}

void
le_lim(first, last)
Echar **first,**last;
{
    *first = loc1;
    *last  = loc2 - 1;
}

static
char *
regerr (err)
{
	switch (err) {
	case 11:
		return ediag ("Range endpoint too large",
			      "Слишком большой диапазон");
	case 16:
		return ediag("Bad number", "Плохое число");
	case 25:
		return ediag ("'\\digit' out of range",
			      "Слишком большое '\\число'");
	case 36:
		return ediag ("Illegal or missing delimiter",
			      "Нет ограничителя");
	case 41:
		return ediag("No remembered search string",
			     "Не было образца поиска");
	case 42:
		return ediag("\\( \\) imbalance",
			     "Несбалансированные \\( \\)");
	case 43:
		return ediag("Too many \\( \\) pairs",
			     "Слишком много пар \\( \\)");
	case 44:
		return ediag ("More then 2 numbers given in \\{ \\}",
			      "В \\{ \\} задано больше чем 2 числа");
	case 45:
		return ediag ("} expected after \\",
			      "После \\ нет }");
	case 46:
		return ediag ("First number exceeds second in \\{ \\}",
			      "Первое число в \\{ \\} больше второго");
	case 49:
		return ediag("[ ] imbalance", "Несбалансированные [ ]");
	case 50:
		return ediag("Regular expression overflow",
			 "Переполнение регулярного выражения");
	default:
		return ediag("Regular expression unknown error",
			     "Неизвестная ошибка регулярного выражения");
	}
}
