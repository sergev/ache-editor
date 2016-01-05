#include "sterm.h"

extern char *GS, *ME, *CU;
extern pch ();
extern void set_attributes ();

xlate_tcap (chr)
Echar chr;
{
    extern AFlag needgraph[];
    Reg1 Short i;

    i = U(chr) - FIRSTSPCL;
    if (needgraph[i]) {
	if (!psgraph) {
	    if (GS) {
	/* if your termcap entry for GS not contain ME... */
		if (ME)
		    tputs (ME, 0, pch);
		tputs (GS, 0, pch);
		attributes = IA_NORMAL;
	    }
	    psgraph = YES;
	}
    }
    else {
	if (psgraph)
	   psexit ();
	if (CU && i <= BULCHAR - FIRSTSPCL) {
	   if (attributes != (chr &~ CHARMASK))
	       set_attributes (chr &~ CHARMASK);
	   tputs (CU, 0, pch);
	   return;
	}
    }
    if (attributes != (chr &~ CHARMASK))
	set_attributes (chr &~ CHARMASK);
    P (stdxlate[i]);
}
