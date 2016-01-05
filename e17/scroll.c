#include "sterm.h"

#undef UP
#undef HO

char *SR;
char *SF;
char *CS;
char *CM;
char *NL;
char *SC;
char *RC;

extern char *tgoto ();
extern int pch ();

vscset_tcap (top, bottom)
{
    if (SC && RC)
	tputs (SC, 0, pch);
    tputs (tgoto (CS, bottom, top), bottom - top, pch);
    if (SC && RC)
	tputs (RC, 0, pch);
}

vscend_tcap ()
{
    if (CS)
	vscset_tcap (0, term.tt_height - 1);
}

scrdn_tcap (num)
{
    do
	tputs(SR, 1, pch);
    while (--num);
}


scrup_tcap (num)
{
    do
	tputs(SF ? SF : (NL ? NL : "\n"), 1, pch);
    while (--num);
}

