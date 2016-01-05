#include "sterm.h"
#ifdef sun
#include <termios.h>
#endif

extern Flag iscyr;
extern Flag qwerty;
extern S_term t_tcap;

#undef UP
#undef HO

char *AE;
char *AL;
char *AS;
char *BC;
char *CD;
char *CE;
char *CH;
char *CL;
char *CM;
char *CR;
char *CS;
char *CU;
char *CV;
char *DC;
char *DL;
char *DO;
char *GE;
char *GS;
char *HO;
char *IC;
char *KE;
char *KS;
char *LL;
char *MB;
char *MD;
char *ME;
char *MH;
char *MR;
char *ND;
char *NL;
char *RC;
char *SC;
char *SE;
char *SF;
char *SO;
char *SR;
char *TE;
char *TI;
char *UE;
char *UP;
char *US;

char PC;

extern int punt_tcap(), clr1_tcap();
extern char *tgoto ();
extern char *getenv ();
extern char *tgetstr();

#define NG  -2
#define UNKNOWN -1
#define OK  0

extern char *tcbuf;

AFlag needgraph[NSPCHR];
static char *GT;
static Short gtlen;

static
void
setgraph (my, gt)
{
	my -= FIRSTSPCL;
	if (gtlen > gt && GT[gt] != ' ') {
	    stdxlate[my] = GT[gt];
	    needgraph[my] = YES;
	}
}

Small
getcap(term)
char *term;
{
    char *cp;

    switch (tgetent(tcbuf, term)) {
    case -1:
    case 0:
	return UNKNOWN;
    }

    iscyr = tgetflag ("CY");
#ifdef M_XENIX
    if (!is_russian ())
	iscyr = NO;
#endif
#ifndef NOEDIAG
     if (!iscyr)
	 _ediag = 1;
#endif

     {
#ifdef TIOCGWINSZ
		struct winsize  wsz;

		if ((ioctl(0, TIOCGWINSZ, (caddr_t) &wsz) != -1) &&
			wsz.ws_col &&
			wsz.ws_row) {
			t_tcap.tt_width = wsz.ws_col;
			t_tcap.tt_height = wsz.ws_row;
		} else
#endif
    {
    t_tcap.tt_width = tgetnum("co");
    t_tcap.tt_height = tgetnum("li");
    }
    }

    if ((cp = getenv ("COLS")) != (char *) NULL)
	t_tcap.tt_width = atoi (cp);

    if ((cp = getenv ("LINES")) != (char *) NULL)
	t_tcap.tt_height = atoi (cp);

    if (t_tcap.tt_width < 3 || t_tcap.tt_height < 5)
	return NG;

    cp = salloc(256, YES);

    if ((HO = tgetstr("ho", &cp)) == NULL)
	t_tcap.tt_home = punt_tcap;

    CD = tgetstr("cd", &cp);
    if ((CL = tgetstr("cl", &cp)) == NULL) {
	if (CD == NULL)
	    return NG;
	t_tcap.tt_clear = clr1_tcap;
    }
    if (CD == NULL) t_tcap.tt_clres = (int (*) ()) 0;

    if ((CM = tgetstr("cm", &cp)) == NULL)
	    return NG;
    t_tcap.tt_naddr = tcaplen(tgoto(CM, 10, 10));

    if ((BC = tgetstr("bc", &cp)) == NULL)
	if (tgetflag("bs"))
	    BC = "\b";
	else {
	    t_tcap.tt_left = punt_tcap;
	    t_tcap.tt_nleft = t_tcap.tt_naddr;
	    t_tcap.tt_nbsp  = 2 * t_tcap.tt_naddr + 1;
	    goto endbc;
	}
    t_tcap.tt_nleft = tcaplen(BC);
    t_tcap.tt_nbsp = 2 * t_tcap.tt_nleft + 1;
 endbc:

    if ((ND = tgetstr("nd", &cp)) == NULL)
	ND = "";
    else
	t_tcap.tt_nright = tcaplen(ND);

    if ((UP = tgetstr("up", &cp)) == NULL) {
	t_tcap.tt_up = punt_tcap;
	t_tcap.tt_nup = t_tcap.tt_naddr;
    }
    else
	t_tcap.tt_nup = tcaplen(UP);

    if (tgetflag("nc")) {
	t_tcap.tt_cret = punt_tcap;
	t_tcap.tt_nl = punt_tcap;
	t_tcap.tt_nnl = t_tcap.tt_naddr;
    }
    else if ((CR = tgetstr("cr", &cp)) == NULL)
	CR = "\r";

    NL = tgetstr("nl", &cp);
    if ((DO = tgetstr("do", &cp)) == NULL && (DO = NL) == NULL)
	DO = "\n";
    t_tcap.tt_ndn = tcaplen(DO);
    t_tcap.tt_nnl = tcaplen(CR) + t_tcap.tt_ndn;

    if (   (AL = tgetstr("al", &cp)) == NULL
	|| (DL = tgetstr("dl", &cp)) == NULL
       ) {
	    t_tcap.tt_insline = (int (*) ()) 0;
	    t_tcap.tt_delline = (int (*) ()) 0;
    }

    if (   (IC = tgetstr("ic", &cp)) == NULL
	|| (DC = tgetstr("dc", &cp)) == NULL
       ) {
	    t_tcap.tt_inschar = (int (*) ()) 0;
	    t_tcap.tt_delchar = (int (*) ()) 0;
    }

    if ((CE = tgetstr("ce", &cp)) == NULL)
	t_tcap.tt_clrel = (int (*) ()) 0;

    if (tgetflag("ns")) {
	t_tcap.tt_vscset = (int (*) ()) 0;
	t_tcap.tt_vscend = (int (*) ()) 0;
	t_tcap.tt_scrup = (int (*) ()) 0;
	t_tcap.tt_scrdn = (int (*) ()) 0;
    }
    else {
	if ((CS = tgetstr("cs", &cp)) == NULL) {
	    t_tcap.tt_vscset = (int (*) ()) 0;
	    t_tcap.tt_vscend = (int (*) ()) 0;
	}

	if ((SR = tgetstr("sr", &cp)) == NULL)
	     t_tcap.tt_scrdn = (int (*) ()) 0;
	SF = tgetstr("sf", &cp);

	t_tcap.tt_da = tgetflag("da");
	t_tcap.tt_db = tgetflag("db");
    }

    SC = tgetstr("sc", &cp);
    RC = tgetstr("rc", &cp);

    if ((LL = tgetstr("ll", &cp)) == NULL)
	t_tcap.tt_ll = (int (*) ()) 0;

    t_tcap.tt_axis = 0;

    t_tcap.tt_ncad = 0;
    if ((CH = tgetstr("ch", &cp)) == NULL)
	t_tcap.tt_cad = (int (*) ()) 0;
    else {
	t_tcap.tt_axis |= 2;
	t_tcap.tt_ncad = tcaplen(CH);
    }

    t_tcap.tt_nlad = 0;
    if ((CV = tgetstr("cv", &cp)) == NULL)
	t_tcap.tt_lad = (int (*) ()) 0;
    else {
	t_tcap.tt_axis |= 1;
	t_tcap.tt_nlad = tcaplen(CV);
    }

    TI = tgetstr("ti", &cp);
    TE = tgetstr("te", &cp);
    CU = tgetstr("cu", &cp);
    KS = tgetstr("ks", &cp);
    KE = tgetstr("ke", &cp);

    GS = tgetstr("gs", &cp);
    if ((GE = tgetstr("ge", &cp)) == NULL)
	t_tcap.tt_gexit = (int (*) ()) 0;

    if ((ME = tgetstr("me", &cp)) == NULL)
	t_tcap.tt_mexit = (int (*) ()) 0;
    if ((AS = tgetstr("as", &cp)) == NULL ||
	(AE = tgetstr("ae", &cp)) == NULL) {
	t_tcap.tt_as = (int (*) ()) 0;
	t_tcap.tt_ae = (int (*) ()) 0;
    }
    if ((US = tgetstr("us", &cp)) == NULL ||
	(UE = tgetstr("ue", &cp)) == NULL) {
	t_tcap.tt_us = (int (*) ()) 0;
	t_tcap.tt_ue = (int (*) ()) 0;
    }
    if ((SO = tgetstr("so", &cp)) == NULL ||
	(SE = tgetstr("se", &cp)) == NULL) {
	t_tcap.tt_so = (int (*) ()) 0;
	t_tcap.tt_se = (int (*) ()) 0;
    }
    if ((MB = tgetstr("mb", &cp)) == NULL || ME == NULL)
	t_tcap.tt_mb = (int (*) ()) 0;
    if ((MD = tgetstr("md", &cp)) == NULL || ME == NULL)
	t_tcap.tt_md = (int (*) ()) 0;
    if ((MH = tgetstr("mh", &cp)) == NULL || ME == NULL)
	t_tcap.tt_mh = (int (*) ()) 0;
    if ((MR = tgetstr("mr", &cp)) == NULL || ME == NULL)
	t_tcap.tt_mr = (int (*) ()) 0;

    Block {
	Reg1 char *pc;

	if (pc = tgetstr("pc", &cp))
		PC = *pc;
    }

    Block {
	Reg1 char *savcp = cp;

	if ((GT = tgetstr("g2", &cp)) != NULL && (gtlen = strlen(GT)) > 0) {
	    setgraph (ELMCH, 1);
	    cp = savcp;
	}
	if ((GT = tgetstr("g5", &cp)) != NULL && (gtlen = strlen(GT)) > 0) {
	    /*setgraph (BTMCH, 9);*/
	    setgraph (MLMCH, 7);
	    setgraph (MRMCH, 6);
	    cp = savcp;
	}
	if ((GT = tgetstr("g1", &cp)) != NULL && (gtlen = strlen(GT)) > 0) {
	SetBord:
	    /*setgraph (ELMCH, 6);*/
	    setgraph (LMCH, 1);
	    setgraph (RMCH, 1);
	    setgraph (TMCH, 0);
	    setgraph (BMCH, 0);
	    setgraph (TLCMCH, 8);
	    setgraph (TRCMCH, 10);
	    setgraph (BLCMCH, 2);
	    setgraph (BRCMCH, 4);
	    /*setgraph (INMCH, 6);*/
	    /*setgraph (BTMCH, 6);*/
	    cp = savcp;
	}
	else if ((GT = tgetstr("g2", &cp)) != NULL && (gtlen = strlen(GT)) > 0)
	    goto SetBord;
	/*
	if ((GT = tgetstr("g3", &cp)) != NULL && (gtlen = strlen(GT)) > 0) {
	    cp = savcp;
	}
	if ((GT = tgetstr("g4", &cp)) != NULL && (gtlen = strlen(GT)) > 0) {
	    cp = savcp;
	} */
	if (CU == (char *) NULL
	      && (GT = tgetstr("g6", &cp)) != (char *) NULL
	      && (gtlen = strlen(GT)) > 0) {
	    if (GT[0] != ' ')
		setgraph (ESCCHAR, 0);
	    else
		setgraph (ESCCHAR, 6);
	    cp = savcp;
	}
    }

    t_tcap.tt_wl = 0;
    t_tcap.tt_pwr = tgetflag("am") ? (tgetflag("xn") ? 4 : 1) : 3; /**/
    t_tcap.tt_cwr = 0;

    if (tgetflag("JC"))
	qwerty = NO;

    return OK;
}

static tcount;

/*ARGSUSED*/
/*static*/
tcapcnt (c)
char c;
{
    tcount++;
}

int
tcaplen (s)
char *s;
{
    tcount = 0;
    if (s != NULL)
	tputs (s, 1, tcapcnt);
    return tcount;
}
