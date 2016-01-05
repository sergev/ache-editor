#include "e.h"
#include "e.e.h"
#include "e.cc.h"
#include "e.cm.h"

Flag
begline (line, col)
Nlines line;
Ncols col;
{
    Reg1 Ncols k;
    Reg2 Uint key;
    Reg3 Short lns;

    key = CCMOVELEFT;
    getline (line);
    if ((k = fnbcol (cline, 0, ncline)) < 0)
	k = 0;
    if (k == col ||
	(lns = abs (col - k)) <= 0)
	return NO;

    if (k > col)
	key = CCMOVERIGHT;
    movecursor (cntlmotions[key], lns);

    return YES;
}

Flag
endline (line, col)
Nlines line;
Ncols col;
{
    Reg1 Ncols k;
    Reg2 Uint key;
    Reg3 Short lns;

    key = CCMOVERIGHT;
    getline (line);
    if ((k = ncline - 1) < 0) k = 0;
    if (k == col ||
	(lns = abs (col - k)) <= 0)
	return NO;

    if (k < col)
	key = CCMOVELEFT;
    movecursor (cntlmotions[key], lns);

    return YES;
}

Flag
leftword (line, col)
Nlines line;
Reg1 Ncols col;
{
    Reg2 Ncols k;

    getline (line);
    for (k = min (col - 1, ncline - 1); k >= 0; k--)
	if (cline[k] != ' '
	      && (k == 0 || cline[k - 1] == ' ')) {
	    movecursor (LT, col - k);
	    return YES;
	}

    return NO;
}


Flag
rightword (line, col)
Nlines line;
Reg1 Ncols col;
{
    Reg2 Ncols k;
    Reg3 Ncols pos;

    getline (line);

    if (col >= ncline - 1)
	return NO;

    for (k = col; k < ncline - 1; k++)
	if (cline[k] == ' ') {
	    if ((pos = fnbcol (cline, k, ncline)) > k) {
		movecursor (RT, pos - col);
		return YES;
	    }
	    else
		break;
	}

    return NO;
}

Cmdret
cleareol (line, col)
Nlines line;
Reg1 Ncols col;
{
    Reg2 Ncols k;

    getline (line);
    if ((k = ncline - 1 - col) > 0)
	return  ed (OPERASE, QERASE, line, col, (Nlines) 1, k, YES);

    return CROK;
}

void
offsetdown (line, col)
Nlines line;
Reg1 Ncols col;
{
    Reg2 Ncols k;
    Reg3 Uint ky;
    Reg4 Short lns;

    getline (line);
    if ((k = fnbcol (cline, 0, ncline)) < 0)
	k = col;
    movecursor (cntlmotions[CCMOVEDOWN], 1);
    if (k != col && (lns = abs (col - k)) > 0) {
	if (k > col)
	    ky = CCMOVERIGHT;
	else
	    ky = CCMOVELEFT;
	movecursor (cntlmotions[ky], lns);
    }
}



