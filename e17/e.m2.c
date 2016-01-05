#include "e.h"
#include "e.cc.h"
#include "e.e.h"
#include "e.cm.h"
#include "e.m.h"
#include "e.mk.h"

/*
    Common code for OPEN, CLOSE, and PICK keys.
    If cmdflg != 0, then do CMD OPEN, CMD CLOSE, CMD PICK.
*/
Cmdret
edkey (key, cmdflg)
Char key;
Flag cmdflg;
{
    Small opc;
    Small buf;

    key = U(key);

    switch (key) {
    case CCOPEN:
	opc = OPOPEN;
	buf = 0;
	break;

    case CCPICK:
	opc = OPPICK;
	buf = QPICK;
	break;

    case CCCLOSE:
	opc = OPCLOSE;
	buf = QCLOSE;
	break;

    case CCERASE:
	opc = OPERASE;
	buf = QERASE;
	break;
    }

    if (curmark) {
	if (!cmdflg)
	    return edmark (opc, buf);
	else
	    return NOMARKERR;
    }
    else {
	if (!cmdflg)
	    return ed (opc, buf,
		       curwksp->wlin + cursorline, (Ncols) 0,
		       (Nlines) 1, (Ncols) 0, YES);
	else {
	    switch (paramtype) {
	    case 0:
		if (key == CCOPEN)
		    return NOTIMPERR;
		return ed (OPINSERT, buf,
			   curwksp->wlin + cursorline,
			   curwksp->wcol + cursorcol,
			   (Nlines) 0, (Ncols) 0, YES);

	    case 1:
		if (parmlines <= 0)
		    return NOTPOSERR;
		return ed (opc, buf,
			   curwksp->wlin + cursorline, (Ncols) 0,
			   parmlines, (Ncols) 0, YES);

	    case 2:
		if (parmlines <= 0 || parmcols <= 0)
		    return NOTPOSERR;
		return ed (opc, buf,
			   curwksp->wlin + cursorline,
			   curwksp->wcol + cursorcol,
			   parmlines, (Ncols) parmcols, YES);
	    }
	    return NOTINTERR;
	}
    }
}

