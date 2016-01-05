#ifndef NOEDIAG
extern short _ediag;
# define ediag(a,b) (_ediag ? (a) : (b))
#else
# define ediag(a,b) (a)
#endif /*NOEDIAG*/
# define isrus8(c) (((c) & 0300) == 0300 && ((c) & 077) != 077)
# define ismap8(c) (((c) & 0300) == 0100 && ((c) & 077) != 077)
# define tomap8(c) ((c) ^ 0240)
# define tojcuk(c) U(qwerty_in[U(c) - 040])
extern Flag uppercaseflg;
extern Flag isuppercase;
#define BEFORECH '\\'
#define CYRILLCH '$'
extern Flag cyrillflg;
extern Flag iscyr;
extern Flag rusbit;
extern Flag qwerty;
extern char qwerty_in[];
#define NSPCHARS 5
extern char cspchars[2][NSPCHARS + 1];
extern char changechar ();
