#include "e.h"
#include "e.tt.h"

extern void getout ();
#ifdef  KBFILE
extern void getkbfile ();
#endif

char *opttname;
char *optkname;     /* Keyboard name */
#ifdef TERMCAP
Flag optdtermcap;       /* YES = force use of termcap */
char *tcbuf;
#endif
#ifdef  KBFILE
/*NOXXSTR*/
#ifndef ANSI
char *kbfile = EDIR(stdkey);  /* name of input (keyboard) table file */
#else
char *kbfile = EDIR("stdkey");  /* name of input (keyboard) table file */
#endif
/*YESXSTR*/
char *optkbfile;        /* Keyboard translation file name */
#endif
Flag optvistabs;   /* YES = -vistabs, NO = -novistabs */

/*
    Get the terminal type.  If Version 7, get the TERM environment variable.
    Copy the appropriate terminal and keyboard structures from the tables
    in e.tt.c into the terminal structure to be used for the session.
*/
void
gettermtype ()
{
#ifdef ENVIRON
    extern char *getenv ();
#endif

    /* Get the selected terminal type */
    if (   !(tname = opttname)
#ifdef ENVIRON
	&& !(tname = getenv ("TERM"))
       )
	getout (YES, ediag(
"No TERM environment variable or -terminal argument",
"Нет ни переменной окружения TERM, ни аргумента флага -terminal"));
#else   ENVIRON
       )
	getout (YES, ediag("No -terminal argument",
			   "Нет аргумента флага -terminal"));
#endif  ENVIRON
    Block {
	Reg1 int ind;
	if ((ind = lookup (tname, termnames)) >= 0)
	    kbdtype = termnames[ind].val;
	else
	    kbdtype = 0;
	if (   ind >= 0
#ifdef  TERMCAP
	    && !optdtermcap
	    && tterm[termnames[ind].val] != tterm[0] /* not a termcap type */
#endif  TERMCAP
	    && tterm[termnames[ind].val] /* compiled-in code */
	   )
	    termtype = termnames[ind].val;
#ifdef  TERMCAP
	else Block {
	    char *str;
	    int res;

	    tcbuf = salloc (1024, YES); /* Отпускать можно только после
					   инициализации клавиатуры */
	    res = getcap (tname);
	    switch (res) {
	    default: {
		termtype = 0; /* termcap type is 0 */
		kbdtype = 0;
		}
		break;
	    case -1:
		str = ediag("Unknown","Неизвестный");
		goto badterm;
	    case -2:
		str = ediag("Unusable","Плохой");
	    badterm:
		getout (YES, ediag("%s terminal type: \"%s\"",
				   "%s тип терминала: \"%s\""),
			str, tname);
	    }
	}
#else   TERMCAP
	else
	    getout (YES, ediag("Unknown terminal type: \"%s\"",
				"Неизвестный тип терминала: \"%s\""),
			 tname);
#endif  TERMCAP
    }

    /* Get the selected keyboard type */
    if (   (kname = optkname)
#ifdef ENVIRON
	|| (kname = getenv ("LEKBD"))
#endif
       ) Block {
	Reg1 int ind;
	if (   (ind = lookup (kname, termnames)) >= 0
	    && tkbd[termnames[ind].val] /*  compiled-in code */
	   )
	    kbdtype = termnames[ind].val;
	else
	    getout (YES, ediag("Unknown keyboard type: \"%s\"",
			       "Неизвестный тип клавиатуры: \"%s\""),
			 kname);
    }
    else if (kbdtype)
	kname = tname;
    else
	kname = "standard";

    term = *(tterm[termtype]);
    if(termtype || kbdtype)
	kbd = *(tkbd[kbdtype]);

#ifdef  KBFILE
    /* Get the keyboard file if specified */
    Block {
	extern unsigned in_file();
	extern int fi_init(), fi_end(), kbi_tcap(), kbe_tcap();

	char *file = optkbfile ? optkbfile : getenv ("LEKBFILE");

	getkbfile (file ? file : kbfile, YES);
	kbd.kb_inlex = in_file;
	kbd.kb_init  = file ? fi_init : kbi_tcap;
	kbd.kb_end   = file ? fi_end : kbe_tcap;
    }
#endif  KBFILE
#ifdef  TERMCAP
    sfree (tcbuf);  /* Отпускаем */
#endif
    Block {
	static Echar Init[2] = {VCCINI, VCCNUL};

	d_write (Init, 2);  /* initializes display image for d_write */
    }

    /* initialize the keyboard */
    (*kbd.kb_init) ();

    Block {
	Reg1 int tmp;

	tmp = term.tt_naddr;
	tt_lt2 = term.tt_nleft  && 2 * term.tt_nleft  < tmp;
	tt_lt3 = term.tt_nleft  && 3 * term.tt_nleft  < tmp;
	tt_rt2 = term.tt_nright && 2 * term.tt_nright < tmp;
	tt_rt3 = term.tt_nright && 3 * term.tt_nright < tmp;
    }
    if (!iscyr) {
	if (!term.tt_so && !term.tt_mr)
	    cyrillflg = NO;
	else {
	    term.tt_prtok = NO;
	    cyrillflg = YES;
	}
    }
    else
	cyrillflg = YES;
    if (isuppercase) {
	if (!term.tt_so && !term.tt_md && !term.tt_mh)
	    uppercaseflg = YES;
	else {
	    term.tt_prtok = NO;
	    uppercaseflg = NO;
	}
    }
    else
	uppercaseflg = NO;
    if (optvistabs >= 0)
	visualtabs = optvistabs;
}
