/*
    initialize input (keyboard) table parsing and lookup
*/

#include "e.h"
#ifdef  KBFILE
#include "e.sg.h"
#include "e.cc.h"
#include "e.it.h"
#ifdef UNIXV7
#include <ctype.h>
#endif

extern char *salloc();
static char *getcs ();
extern Flag noctrlsflg;
extern Uint start_char;
extern Uint stop_char;
struct itable *ithead = NULLIT;
/*NOXXSTR*/
S_looktbl itsyms[] = {
    "+file",   CCPLFILE,
    "+line",   CCPLLINE,
    "+page",   CCPLPAGE,
    "+sch",    CCPLSRCH,
    "+tab",    CCTAB,
    "+word",   CCRWORD,
    "-file",   CCMIFILE,
    "-line",   CCMILINE,
    "-page",   CCMIPAGE,
    "-sch",    CCMISRCH,
    "-tab",    CCBACKTAB,
    "-word",   CCLWORD,
    "abandon", CCABANDON,
    "alt",     CCALT,
    "begs",    CCBEGS,
    "bs",      CCBACKSPACE,
    "cchar",   CCCTRLQUOTE,
    "chwin",   CCCHWINDOW,
    "close",   CCCLOSE,
    "clreol",  CCCLREOL,
    "cmd",     CCCMD,
    "dchar",   CCDELCH,
    "del",     0177,
    "down",    CCMOVEDOWN,
    "edit",    CCPLFILE,
    "ends",    CCENDS,
    "erase",   CCERASE,
    "esc",     033,
    "fcol",    CC1COLUMN,
    "goto",    CCGOTO,
    "home",    CCHOME,
    "ichar",   CCINSCH,
    "insmd",   CCINSMODE,
    "int",     CCINT,
    "join",    CCJOIN,
    "kbend",   KBEND,
    "kbinit",  KBINIT,
    "lat",     CCLAT,
    "left",    CCMOVELEFT,
    "mark",    CCMARK,
    "mend",    CCMEND,
    "mexec",   CCMEXEC,
    "mkwin",   CCMKWIN,
    "open",    CCOPEN,
    "pick",    CCPICK,
    "quote",   042,
    "redraw",  CCREDRAW,
    "replace", CCREPLACE,
    "return",  CCRETURN,
    "right",   CCMOVERIGHT,
    "rolldn",  CCROLLDOWN,
    "rollup",  CCROLLUP,
    "rus",     CCRUS,
    "space",   040,
    "split",   CCSPLIT,
    "srtab",   CCTABS,
    "tag",     CCTAG,
    "undef",   CCUNAS1,
    "up",      CCMOVEUP,
    "wleft",   CCLWINDOW,
    "wright",  CCRWINDOW,
    0, 0
 };
/*YESXSTR*/
char *kbinistr = NULL;
char *kbendstr = NULL;
int  kbinilen = 0;
int  kbendlen = 0;

int
fi_init()
{
    if(kbinilen > 0)
	fwrite(kbinistr, sizeof(char), kbinilen, stdout);
}

int
fi_end()
{
    if(kbendlen > 0)
	fwrite(kbendstr, sizeof(char), kbendlen, stdout);
}

void
getkbfile (filename, tcap)
char *filename;
Flag tcap;
{
    char line[TMPSTRLEN], string[TMPSTRLEN], value[TMPSTRLEN];
    Reg1 FILE *f;
    char *cp, *index();
    int str_len, val_len;

    if ((f = fopen (filename, "r")) == (FILE *) NULL)
OpErr:
	getout (YES, ediag("Can't open keyboard file %s",
			   "Нельзя открыть файл описания клавиатуры %s"),
			filename);
    if (!setflock (fileno (f), 0, 0)) {
	fclose (f);
	goto OpErr;
    }
    while (fgets (line, sizeof (line), f) != (char *) NULL) {
	if (cp = index (line, '\n'))
	    *cp = '\0';
	if(   line[0] == '#'
	   || line[0] == '\0')
		continue;
	itparse (line, string, &str_len, value, &val_len);
	string[str_len] = '\0';
	switch (string[0]) {
	case KBINIT:
	    kbinilen = val_len;
	    kbinistr = salloc (kbinilen, YES);
	    (void) move (value, kbinistr, kbinilen);
	    break;
	case KBEND:
	    kbendlen = val_len;
	    kbendstr = salloc (kbinilen, YES);
	    (void) move (value, kbendstr, kbendlen);
	    break;
	default: {
		Reg2 char *s = tcap ? getcs (string, &str_len) : string;

		if (s != (char *) NULL && str_len > 0)
		    itadd (s, str_len, &ithead, value, val_len, line);
	    }
	}
    }
    fclose (f);
#ifdef  DEBUG_KBFILE
    itprint (ithead, 0);
#endif
}

itadd (str, str_len, headp, val, val_len, line)
char *str;              /* Character string */
struct itable **headp;  /* Pointer to head (where to start) */
char *val;              /* Value */
int str_len, val_len;
char *line;             /* For debugging */
{
    Reg1 struct itable *it;         /* Current input table entry */
    Reg2 struct itable *pt;         /* Previous table entry */

    if (str_len <= 0)
	getout (YES, ediag("kbfile invalid prefix in %s\n",
		 "Файл описания клавиатуры: неверный описатель %s\n"),
			line);
    for (it = *headp; it != NULLIT; pt = it, it = it->it_next) {
	if (it->it_c == *str) {         /* Character match? */
	    if (it->it_leaf)            /* Can't add this */
		getout (YES, ediag("kbfile duplicate string in %s\n",
	     "Файл описания клавиатуры: строка описателя %s уже была\n"),
			line);
	    else        /* Go down the tree */
		itadd (str+1, str_len-1, &it->it_link, val, val_len, line);
	    return;
	}
    }
    it = (struct itable *) salloc (sizeof *it, YES);   /* Get new node */
    if (*headp == NULLIT)
	*headp = it;
    else
	pt->it_next = it;
    it->it_c = *str++;                  /* Save current character */
    it->it_next = NULLIT;
    if (--str_len > 0) {                /* Is this a leaf? */
	it->it_leaf = 0;                /* No */
	it->it_link = NULLIT;
	itadd (str, str_len, &it->it_link, val, val_len, line);
    }
    else {
	it->it_leaf = 1;
	it->it_val = salloc (val_len, YES);
	it->it_len = val_len;
	move (val, it->it_val, val_len);
    }
}

itparse (inp, strp, str_lenp, valp, val_lenp)
register char *inp;
char *strp, *valp;      /* Pointers to string to match and value to return */
int *str_lenp, *val_lenp;
{
    Reg1 char c;
    unsigned int n;
    int i;
    int gotval = 0;
    Reg2 char *outp = strp;
    char tmpstr[50], *tp;
    char *line = inp;            /* Save for error messages */

    while ((c = *inp++) != '\0') {
	switch (c) {
	    case '\\':
		if ((c = *inp++) == '\0')
Preos:
		    getout (YES, ediag("kbfile bad line %s\n",
	      "Файл клавиатуры: неожиданный конец %s\n"),
				line);
		if (c >= '0' && c <= '7') {
		    for (i = n = 0; c >= '0' && c <= '7' && i < 3;
					c = *inp++, i++)
			n = n * 8 + (c - '0');
		    if (n > 0377)
			getout (YES, ediag(
"kbfile number 0%o too big in %s\n",
"Файл клавиатуры: число 0%o слишком большое в %s\n"),
  n, line);
		    inp--;
		    *outp = (char) n;
		    break;
		}
		else for (i = 0; i < 6; i++)
		    if (c == "trnfbE"[i]) {
			c = "\t\r\n\f\b\33"[i];
			break;
		    }
		*outp = c;
		break;
	    case '<':
		for (tp = tmpstr; (c = *inp++) != '>'; ) {
		    if (c == '\0')
		       getout (YES, ediag("kbfile mismatched '<' in %s\n",
      "Файл клавиатуры: отсутствует '<' в %s\n"),
					line);
		    *tp++ = c;
		}
		*tp = '\0';
		i = lookup (tmpstr, itsyms);
		if (i < 0)
		    getout (YES, ediag("kbfile bad symbol %s in %s\n",
	   "Файл клавиатуры: неизвестное имя %s в %s\n"),
					tmpstr, line);
		*outp = (char) itsyms[i].val;
		break;
	    case '^':
		if ((c = *inp++) == '\0')
		    goto Preos;
		if (c != '?' && (c < '@' || c > '_'))
		    getout (YES, ediag("kbfile bad char ^<%o> in %s\n",
		"Файл клавиатуры: неверный символ ^<%o> в %s\n"),
					c, line);
		*outp = c^0100;
		break;
	    case ':':
		if (gotval++)
		    goto endline;
		*str_lenp = outp - strp;
		if (*str_lenp > TMPSTRLEN)
		    goto toolong;
		outp = valp;
		continue;
	    default:
		*outp = c;
		break;
	}
	if (!gotval && !noctrlsflg)
	    noctrlsflg =
		(start_char != 0xff && U(*outp) == start_char ||
		 stop_char != 0xff && U(*outp) == stop_char);
	outp++;
    }
    getout (YES, ediag("kbfile mismatched ':' in %s\n",
		"Файл клавиатуры: отсутствует ':' в %s\n"),
			line);
endline:
    *val_lenp = outp - valp;
    if (*str_lenp > TMPSTRLEN)
toolong:
	getout (YES, ediag("kbfile line too long %s\n",
		"Файл клавиатуры: слишком длинная строка %s\n"),
			line);
}

#ifdef DEBUG_KBFILE
itprint (head, n)
struct itable *head;
int n;
{
    Reg1 struct itable *it;
    int i;
    char c;
    char *cp;

    for (it = head; it != NULLIT; it = it->it_next) {
	for (i = 0; i < n / 2; i++)
	    fputs ("|   ", stdout);
	c = it->it_c;
	if (isprint(U(c)))
	    printf ("|%c  |", c);
	else
	    printf ("|%3.3o|", c);
	if (it->it_leaf) {
	    printf ("=");
	    for (cp = it->it_val, i = it->it_len; i-- > 0; cp++) {
		Reg2 int j;

		for (j = 0; itsyms[j].str; j++)
			if (*cp == (char) itsyms[j].val) {
			    printf("<%s>", itsyms[j].str);
			    goto done;
			}
		printf ("<%o>", *cp);
	done:   ;
	    }
	    printf (" (len %d)\n", it->it_len);
	}
	else {
	    printf ("\n");
	    itprint (it->it_link, n+2);
	}
    }
}
#endif  DEBUG_KBFILE

static
char *
getcs (s, len)   /* разбор строки описателей termcap "xxyy.." */
Reg1 char *s;
int *len;
{
#define N_READ 40
    static char buf[N_READ];
    char *bufp; /* Not a register */
    extern char *tgetstr(), *index ();
    Reg2 Uchar c;

    *len = 0;
    for (bufp = buf; *s; s += 2, bufp--) {
	c = *s;
	if (!isalpha(c)) {
	    while (bufp < &buf[N_READ] && *s)
		*bufp++ = *s++;
	    *bufp = '\0';
	    goto Ret;
	}
	else Block {
	    Flag bad;

	    if (!s[1])
		goto BadRet;
	    c = s[2]; s[2] = '\0';
	    bad = (tgetstr (s, &bufp) == (char *)NULL);
	    s[2] = c;
	    if (bad) {
	BadRet:
		return (char *)NULL;
	    }
	}
    }
    *bufp = '\0';
    if (!noctrlsflg && *buf)
	noctrlsflg =
	    (start_char != 0xff && index (buf, start_char) != NULL ||
	     stop_char != 0xff && index (buf, stop_char) != NULL);
Ret:
    *len = bufp - buf;
    if (*len >= sizeof(buf) - 1)
	getout (YES, ediag("kbfile termcap string %s too long\n",
	  "Файл клавиатуры: слишком длинный описатель %s из termcap\n"),
	   buf);

    return buf;
}
#endif  KBFILE
