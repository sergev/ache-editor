/*
--------
file term/standard.c
    Standard stuff in support of
    terminal-dependent code and data declarations
*/

#ifdef T_std
extern S_kbd kb_std;
#else
#ifdef UNIXV7
#include <ctype.h>
#endif
#include "sterm.h"
#include "e.cc.h"

extern Flag knockdown;

/* То, что помечено +, может меняться */
char lexstd[32] = {
    CCCMD       ,       /*@*/
    CCTAG       ,       /*A*/   /*+*/
    CCBACKTAB   ,       /*B*/
    CCINT       ,       /*C*/
    CCMIFILE    ,       /*D*/
    CCMIPAGE    ,       /*E*/   /*+*/
    CCPLLINE    ,       /*F*/   /*+*/
    CCMILINE    ,       /*G*/   /*+*/
    CCMOVELEFT  ,       /*H*/
    CCTAB       ,       /*I*/
    CC1COLUMN   ,       /*J*/
    CCMOVEUP    ,       /*K*/   /*+*/
    CCMOVERIGHT ,       /*L*/   /*+*/
    CCRETURN    ,       /*M*/
    CCRUS       ,       /*N*/ /* RUS */
    CCLAT       ,       /*O*/ /* LAT */
    CCPICK      ,       /*P*/   /*+*/
    CCUNAS1     ,       /*Q*/ /* start char */
    CCPLPAGE    ,       /*R*/   /*+*/
    CCUNAS1     ,       /*S*/ /* stop char */
    CCMEXEC     ,       /*T*/   /*+*/
    CCINSCH     ,       /*U*/   /*+*/
    CCCLOSE     ,       /*V*/   /*+*/
    CCDELCH     ,       /*W*/   /*+*/
    CCUNAS1     ,       /*X*/ /* prefix char */
    CCMEND      ,       /*Y*/   /*+*/
    CCOPEN      ,       /*Z*/   /*+*/
    CCUNAS1     ,       /*[*/ /*ESC*/
    CCINT       ,       /*\*/
    CCMOVEDOWN  ,       /*]*/   /*+*/
    CCHOME      ,       /*^*/   /*+*/
    CCMARK      ,       /*_*/   /*+*/
};

Uint erase_char = 0177;       /* set in e.sg.c */
Uint start_char = 0xff;
Uint stop_char = 0xff;

#define CTRL(x) C(x)

unsigned
in_std (lexp, count)
char *lexp;
int *count;
{
    Reg4 int nr;
    Reg3 Uint chr;
    Reg1 char *icp;
    Reg2 char *ocp;
    Flag isknockdown;

    isknockdown = knockdown;
    icp = ocp = lexp;
    for (nr = *count; nr > 0; nr--) {
	chr = U(*icp++);
	if (isknockdown || isprint(chr)) {
	    *ocp++ = chr;
	    isknockdown = NO;
	}
	else if (chr == erase_char) *ocp++ = CCBACKSPACE;
	else if (chr == CTRL ('X')) {
	    if (nr < 2) {
		icp--;
		goto nomore;
	    }
	    nr--;
	    chr = *icp++ & 0177;
	    switch (chr) {
	    case CTRL ('A'):
		*ocp++ = CCCTRLQUOTE;
		break;
	    case CTRL ('B'):
		*ocp++ = CCSPLIT;
		break;
	    case CTRL ('D'):
		*ocp++ = CCPLFILE;
		break;
	    case CTRL ('E'):
		*ocp++ = CCERASE;
		break;
	    case CTRL ('F'):
		*ocp++ = CCABANDON;
		break;
	    case CTRL ('G'):
		*ocp++ = CCBEGS;
		break;
	    case CTRL ('H'):
		*ocp++ = CCLWINDOW;
		break;
	    case CTRL ('I'):
		*ocp++ = CCINSMODE;
		break;
	    case CTRL ('J'):
		*ocp++ = CCJOIN;
		break;
	    case CTRL ('K'):
		*ocp++ = CCENDS;
		break;
	    case CTRL ('L'):
		*ocp++ = CCRWINDOW;
		break;
	    case CTRL ('M'):
		*ocp++ = CCMISRCH;
		break;
	    case CTRL ('P'):
		*ocp++ = CCPLSRCH;
		break;
	    case CTRL ('R'):
		*ocp++ = CCREPLACE;
		break;
	    case CTRL ('T'):
		*ocp++ = CCTABS;
		break;
	    case CTRL ('U'):
		*ocp++ = CCLWORD;
		break;
	    case CTRL ('V'):
		*ocp++ = CCRWORD;
		break;
	    case CTRL ('W'):
		*ocp++ = CCCHWINDOW;
		break;
	    case CTRL ('Y'):
		*ocp++ = CCROLLUP;
		break;
	    case CTRL ('Z'):
		*ocp++ = CCROLLDOWN;
		break;
	    case CTRL (']'):
		*ocp++ = CCMKWIN;
		break;
	    case CTRL ('^'):
		*ocp++ = CCCLREOL;
		break;
	    case CTRL ('_'):
		*ocp++ = CCREDRAW;
		break;
	    default:
		*ocp++ = CCUNAS1;
		break;
	    }
	}
	else if (chr < 32) *ocp++ = lexstd[chr];
	else *ocp++ = CCUNAS1;
    }

nomore:
    Block {
	Reg5 Uint conv;

	*count = nr;
	conv = ocp - lexp;
	while (nr-- > 0)
	    *ocp++ = *icp++;
	return conv;
    }
}

S_kbd kb_std = {
/* kb_inlex */  in_std,
/* kb_init  */  nop,
/* kb_end   */  nop,
};

#endif
