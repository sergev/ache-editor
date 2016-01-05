#include "e.h"
#include "e.cc.h"
#include "e.mk.h"
#include "e.cm.h"
#include "e.help.h"
#include "e.ml.h"
#include <ctype.h>

void HelpGotoMarg ();
int HelpItems = 0;
S_help *Item = (S_help *) NULL;
S_hpage *HelpPage = (S_hpage *) NULL;
static S_hpage *NextItem ();
static S_hpage *FindMarg ();

Cmdret
dohelp ()
{
    char *cp, *ncp;
    int len, num;
#ifndef ANSI
    static char helpfile[] = EDIR(help);
#else
    static char helpfile[] = EDIR("help");
#endif
    Short i;
    Short j;

    cp = append (helpfile, ediag ("", "_r"));
    if (editfile (cp, (Ncols) -1, (Nlines) -1, 0, NO, NO) <= 0) {
	sfree (cp);
	mesg (ERRALL + 1, ediag("Help file gone: notify sys admin.",
	   "Отсутствует help-файл: обратитесь к системным программистам."));
	return CROK;
    }
    sfree (cp);

    fileflags[curfile] &= ~CANMODIFY;

    if (Item == (S_help *) NULL) { /* Initilize help structures */
	if ((cp = pkarg (curwksp,
			 (Nlines) 0, (Ncols) 0,
			 &len, PK_UPTOS)) == (char *) NULL) {
	Err:
	    deffn = curfile;
	    (void) eddeffile (YES);
	    mesg (ERRALL + 1, ediag("Bad format of Help file: notify sys admin.",
	       "Неверный формат help-файла: обратитесь к системным программистам."));
	    return CROK;
	}
	if (len == 0 || s2i (cp, &HelpItems) == cp
	     || HelpItems <= 0 || HelpItems >= la_lsize (curlas) - 1) {
	    sfree (cp);
	    goto Err;
	}
	sfree (cp);
	if ((Item = (S_help *) salloc (HelpItems * sizeof (S_help), NO))
		== (S_help *) NULL)
	    goto Err;
	for (i = 0; i < HelpItems; i++) {
	    if ((cp = pkarg (curwksp,
			     (Nlines) i + 1, (Ncols) 0,
			     &len, PK_FIELD)) == (char *) NULL) {
	ErrAll:
		for (j = 0; j < i; j++)
		    sfree (Item[j].item_name);
		sfree ((char *) Item);
		goto Err;
	    }
	    if (len == 0) {
		sfree (cp);
		goto ErrAll;
	    }
	    Item[i].item_name = cp;
	    Item[i].item_len = len;
	    if ((cp = pkarg (curwksp,
			     (Nlines) i + 1, (Ncols) len + 1,
			     &len, PK_FIELD)) == (char *) NULL) {
	    ErrNum:
		sfree (Item[i].item_name);
		goto ErrAll;
	    }
	    if (len == 0
		|| (ncp = s2i (cp, &num)) == cp
		|| num <= HelpItems + 1) {
	    ErrPar:
		sfree (cp);
		goto ErrNum;
	    }
	    Item[i].def_line = num - 1;
	    while (isspace (*ncp))
		ncp++;
	    if (s2i (ncp, &num) == ncp || num < 0)
		goto ErrPar;
	    sfree (cp);
	    Item[i].def_col = num;
	}
    }   /* End of init */

    fileflags[curfile] |= HELP;
    HelpActive = YES;

    movewin ((Nlines) HelpItems + 1, (Ncols) 0, (Scols) 0, (Slines) 0, NO);
    putupwin ();
    HelpGotoMarg (NO);

    return CROK;
}

void
AddHelpEntry (num, line, col)
Ncols col;
Nlines line;
Short num;
{
    Reg1 S_hpage *hp;
    S_hpage *oldhead;
    Nlines minl, maxl;
    Ncols minc, maxc;

    oldhead = HelpPage;

    minl = curwksp->wlin;
    minc = curwksp->wcol;
    maxl = minl + curwin->btext;
    maxc = minc + curwin->rtext;

    for (hp = HelpPage; hp != (S_hpage *) NULL; hp = hp->item_next) {
	if (hp->item_num == num)
	    return;
	if (hp->item_line < minl || hp->item_col < minc
	      || hp->item_line > maxl || hp->item_col > maxc) {
    Set:
	    hp->item_line = line;
	    hp->item_col = col;
	    hp->item_num = num;
	    return;
	}
    }

    if ((hp = (S_hpage *) salloc (sizeof (S_hpage), NO)) ==
	      (S_hpage *) NULL) {
	mesg (ERRALL + 1, ediag ("No memory for help item!",
				"Мало памяти для термина help!"));
	return;
    }

    hp->item_next = oldhead;
    HelpPage = hp;
    goto Set;
}

static
S_hpage *
NextItem (line, col, vert, forw)
Nlines line;
Ncols col;
Flag vert;
Flag forw;
{
    Nlines minl, maxl;
    Ncols minc, maxc;
    Reg1 S_hpage *hp;
    Reg2 S_hpage *nearhp;

    minl = curwksp->wlin;
    minc = curwksp->wcol;
    maxl = minl + curwin->btext;
    maxc = minc + curwin->rtext;

    nearhp = (S_hpage *) NULL;

    for (hp = HelpPage; hp != (S_hpage *) NULL; hp = hp->item_next) {
	if (hp->item_line < minl || hp->item_col < minc
	      || hp->item_line > maxl || hp->item_col > maxc)
	    continue;

	if (vert && hp->item_col == col
	     && hp->item_line == (forw ? line + 1 : line - 1))
	    return hp;

	if (forw) {
	    if ((hp->item_line > line
		 || hp->item_line == line
		    && hp->item_col > col) /* Valid hp */
		&& (nearhp == (S_hpage *) NULL
		    || nearhp->item_line > hp->item_line     /* or */
		    || nearhp->item_line == hp->item_line
		       && nearhp->item_col >= hp->item_col)) { /* Hp better */
		nearhp = hp;
	    }
	}
	else {
	    if ((hp->item_line < line
		 || hp->item_line == line
		    && hp->item_col < col) /* Valid hp */
		&& (nearhp == (S_hpage *) NULL
		    || nearhp->item_line < hp->item_line     /* or */
		    || nearhp->item_line == hp->item_line
		       && nearhp->item_col <= hp->item_col)) { /* Hp better */
		nearhp = hp;
	    }
	}
    }

    return nearhp;
}

static
Short
FindItem (line, col)
Nlines line;
Ncols col;
{
    Reg1 S_hpage *hp;

    for (hp = HelpPage; hp != (S_hpage *) NULL; hp = hp->item_next)
	if (hp->item_line == line && hp->item_col == col)
	    return hp->item_num;
    return -1;
}

static
S_hpage *
FindMarg (forw)
Flag forw;
{
    Reg1 S_hpage *hp;
    Reg2 S_hpage *mosthp;
    Nlines minl, maxl;
    Ncols minc, maxc;

    minl = curwksp->wlin;
    minc = curwksp->wcol;
    maxl = minl + curwin->btext;
    maxc = minc + curwin->rtext;

    mosthp = (S_hpage *) NULL;
    for (hp = HelpPage; hp != (S_hpage *) NULL; hp = hp->item_next) {
	if (hp->item_line < minl || hp->item_col < minc
	      || hp->item_line > maxl || hp->item_col > maxc)
	    continue;
	if (mosthp == (S_hpage *) NULL)
	    mosthp = hp;
	else if (!forw) {
	    if (hp->item_line < mosthp->item_line
		|| mosthp->item_line == hp->item_line
		    && hp->item_col < mosthp->item_col)
		mosthp = hp;
	}
	else {
	    if (hp->item_line > mosthp->item_line
		|| mosthp->item_line == hp->item_line
		    && hp->item_col > mosthp->item_col)
		mosthp = hp;
	}
    }
    return mosthp;
}

Flag
HelpMove (cm)
Small cm;
{
    Flag vert;
    Flag forw;
    S_hpage *hp;
    Flag first;
    Short Num;
    Nlines lin;
    Ncols col;

    switch (cm) {
	case LT:
	case BT:
	    vert = NO;
	    forw = NO;
	    break;
	case RT:
	case TB:
	    vert = NO;
	    forw = YES;
	    break;
	case UP:
	    vert = YES;
	    forw = NO;
	    break;
	case DN:
	Down:
	    vert = YES;
	    forw = YES;
	    break;
	case RN:
	    if ((Num = FindItem (curwksp->wlin + cursorline,
				 curwksp->wcol + cursorcol)) < 0)
		goto Down;

	    lin = Item[Num].def_line;
	    col = Item[Num].def_col;
	    if (editfile (names[curfile], (Ncols) -1, (Nlines) -1, 0, NO, NO) <= 0) {
		mesg (ERRALL + 1, ediag("Help file gone: notify sys admin.",
		   "Отсутствует help-файл: обратитесь к системным программистам."));
		return NO;
	    }
	    markrect (lin, col, lin, Item[Num].item_len + col);

	    return YES;

	default:
	    return NO;
    }

Next:
    if ((hp = NextItem (curwksp->wlin + cursorline,
			curwksp->wcol + cursorcol,
			vert, forw)) == (S_hpage *) NULL) {
	if (first) {
	    if (curmark)
		unmark ();
	    if (!vert)
		horzmvwin ((Nlines) (forw ? defrwin : -deflwin));
	    else
		rollmvwin ((Nlines) (forw ? 1 : -1));
	    first = NO;
	    goto Next;
	}
	else if ((hp = FindMarg (forw)) == (S_hpage *) NULL)
	    return YES;
    }

    lin = hp->item_line;
    col = hp->item_col;
    markrect (lin, col, lin, Item[hp->item_num].item_len + col);

    return YES;
}

void
HelpGotoMarg (forw)
Flag forw;
{
    Reg1 S_hpage *hp;
    Nlines lin;
    Ncols col;

    if ((hp = FindMarg (forw)) == (S_hpage *) NULL)
	return;
    lin = hp->item_line;
    col = hp->item_col;
    markrect (lin, col, lin, Item[hp->item_num].item_len + col);
}
