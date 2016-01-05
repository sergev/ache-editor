#include "e.h"
#include "e.inf.h"
#include "e.tt.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.mac.h"

#define LMAC ('z' - 'a' + 1)

S_macro *mtab[LMAC];
S_nest *mread = NULL;
Small setmacro = 0;
Small mnest = 0;

void
macerr (mr)
Reg1 S_nest *mr;
{
    Reg2 S_nest *nextmread;
    Reg3 Short i;

    if (mr == NULL)
	return;
    while (mr != NULL) {
	nextmread = mr->iprevinp;
	sfree ((char *)mr);
	mr = nextmread;
    }
    mread = NULL;
    for (i = 0; i < LMAC; i++)
	if (mtab[i] != NULL)
	    mtab[i]->mused = NO;
    if (!replaying)
	flinput ();
}

Cmdret
mexec (nmac, num)
Reg1 Small nmac;
Reg2 Short num;
{
    Reg3 S_nest *oldmread = mread;
    char nameb[2];

    nameb[0] = nmac + 'a';
    nameb[1] = '\0';
    if (mtab[nmac] == NULL) {
/*NOXXSTR*/
	mesg (ERRALL + 3, ediag ("Macro `", "Макро `"),
/*YESXSTR*/
	      nameb, ediag ("' undefined.", "' неопределен."));
    }
    else if (mtab[nmac]->mused) {
/*NOXXSTR*/
	mesg (ERRALL + 3, ediag ("Macro `", "Макро `"),
/*YESXSTR*/
	      nameb, ediag ("' loop.", "': зацикливание."));
    }
    else {
	if ((mread = (S_nest *) salloc (sizeof (S_nest), NO)) == NULL) {
	    if (oldmread != NULL)
		macerr (oldmread);
	    return NOMEMERR;
	}
	mread->iprevinp = oldmread;
	mread->icnt = mtab[nmac]->mlen;
	mread->imac = nmac;
	mread->inum = num;
	mtab[nmac]->mused = YES;
	infomacro (YES, nmac, num);
    }
    return CROK;
}

static
void
mmsg (c)
Reg1 Small c;
{
    char nameb[2];

    nameb[0] = c + 'a';
    nameb[1] = '\0';
    mesg (TELSTRT + 3, ediag ("Macro `", "Макро `"), nameb, "': ");
    loopflags.hold = YES;
}

Cmdret
domacro (set)
Reg1 Flag set;
{
    Reg2 Small c;

    if (opstr[0] == '\0')
	return CRNEEDARG;
    if (*nxtop)
	return CRTOOMANYARGS;
    c = (opstr[0] & 0177) | 040;
    if (c < 'a' || c > 'z' || opstr[1] != '\0')
	return CRBADARG;
    if (setmacro)
	mnest++;
    if (mnest) {
	mmsg (c - 'a');
	mesg (TELDONE + 1, ediag ("nested.", "вложенный."));
	return CROK;
    }
    c -= 'a';
    if (set && mtab[c] != NULL) {
	mmsg (c);
	mesg (TELDONE + 1, ediag ("already defined.","уже определен."));
    }
    else if (!set && mtab[c] == NULL) {
	mmsg (c);
	mesg (TELDONE + 1, ediag ("undefined.","неопределен."));
    }
    else if (!set) {
	mmsg (c);
	mesg (TELDONE + 1, ediag ("delete.","отменен."));
	if (mtab[c]->mbase != NULL)
		sfree (mtab[c]->mbase);
	sfree ((char *)mtab[c]);
	mtab[c] = NULL;
    }
    else {
	if ((mtab[c] = (S_macro *) salloc (sizeof (S_macro), NO)) == NULL) {
	    mmsg (c);
	    mesg (TELDONE + 1, ediag ("no memory.", "нет памяти."));
	}
	else {
	    mtab[c]->mbase = NULL;
	    mtab[c]->mlen = 0;
	    mtab[c]->mused = NO;
	    mcount = 0;
	    setmacro = c + 1;
	    infomacro (YES, c, 0);
	}
    }
    return CROK;
}

void
infomacro (on, num, cnt)
Reg1 Small num;
Reg2 Flag on;
Reg3 Short cnt;
{
    char nameb[2], buf[10];

    nameb[0] = num + 'a';
    nameb[1] = '\0';
    if (cnt)
	sprintf (buf, "%d", cnt);
    info (inf_macro, 4, on && cnt ? buf : "    ", NO);
    if (on)
	d_put (VCCMD);
    info (inf_mname, 1, on ? nameb : " ", NO);
    d_flush ();
}
