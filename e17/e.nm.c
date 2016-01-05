/*    name and delete commands */

#include "e.h"
#include "e.inf.h"
#include "e.tt.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.fn.h"
#include <sys/stat.h>

static void dlwkfn ();
extern int curfnum;
extern int infofnum;
void dlcurwk ();
void dlcurfn ();

/*    Do the name command. */
Cmdret
name ()
{
    Flag renamed,
	 new;
    char *name;

    if (opstr[0] == '\0')
	return CRNEEDARG;
    if (*nxtop)
	return CRTOOMANYARGS;

    name = names[curfile];

    if (   (renamed = fileflags[curfile] & RENAMED ? YES : NO)
	&& strcmp (oldnames[curfile], opstr) == 0
       ) {
	sfree (oldnames[curfile]);
	oldnames[curfile] = NULL;
	fileflags[curfile] &= ~RENAMED;
	goto ret;
    }

    if (   dotdot (name)
	|| curfile < FIRSTFILE + NTMPFILES
       ) {
	mesg (ERRALL + 2, ediag("Can't rename ", "Нельзя переименовать "),
					name);
	return CROK;
    }

    if ((fileflags[curfile] & DWRITEABLE) == 0) {
	dircheck (name, (char **) 0, (char **) 0, YES, YES);
	return CROK;
    }

    if (hvname (opstr) != -1)
	goto exists;

    if (dircheck (opstr, (char **) 0, (char **) 0, YES, YES) == -1)
	return CROK;

    if (   hvdelname (opstr) == -1
	&& hvoldname (opstr) == -1
	&& access (opstr, 0) >= 0
       ) {
 exists:
	mesg (ERRALL + 1, ediag("That name exists already",
				"Такое имя уже есть"));
	return CROK;
    }

    if (!(new = (fileflags[curfile] & NEW))) Block {
	char *d1, *d2;
	struct stat s1, s2;

	dircheck ((renamed ? oldnames : names)[curfile],
		  &d1, (char **) 0, NO, NO);
	dircheck (opstr, &d2, (char **) 0, NO, NO);

	stat (d1, &s1);
	stat (d2, &s2);

#ifdef UNIXV7
	if (s1.st_dev != s2.st_dev) {
#else
#ifdef UNIXV6
	if (   s1.st_minor != s2.st_minor
	    || s1.st_major != s2.st_major
	   ) {
#else
ERROR:  must define one or the other
#endif
#endif
	    mesg (ERRALL + 1, ediag("Can't rename to there, have to copy",
		      "Нельзя переименовать туда, попробуйте скопировать"));
	    return CROK;
	}
    }
    if (new || renamed)
	sfree (name);
    else {
	oldnames[curfile] = name;
	fileflags[curfile] |= RENAMED;
    }
 ret:
    d_put (VCCAS);
    info (inf_file, chanchcnt (names[infofile]), opstr, YES);
    names[curfile] = append (opstr, "");
    return CROK;
}

/*    Do the delete command. */
Cmdret
delete ()
{
    Reg1 Short flags;
    Reg2 char *name;
    int j;
    Small len;

    if (opstr[0] != '\0')
	return CRTOOMANYARGS;

    name = names[curfile];
    flags = fileflags[curfile];

    if (curfile < FIRSTFILE + NTMPFILES) {
	mesg (ERRALL + 2, ediag("Can't delete ","Нельзя удалить "), name);
	return CROK;
    }

    if ((flags & DWRITEABLE) == 0) {
	dircheck (name, (char **) NULL, (char **) NULL, YES, YES);
	return CROK;
    }

    if (   !(flags & NEW)
	&& (j = filetype (name)) != -1
	&& j == S_IFDIR
       ) {
	mesg (ERRALL + 1, ediag("Can't delete directories",
				"Нельзя удалять каталоги"));
	return CROK;
    }

    if (flags & (NEW | RENAMED)) {
	len = chanchcnt (name);
	sfree (name);
    }
    if (flags & NEW)
	flags = 0;
    else {
	if (flags & RENAMED) {
	    names[curfile] = oldnames[curfile];
	    oldnames[curfile] = NULL;
	}
	else
	    len = chanchcnt (name);
	flags &= ~(SAVED | RENAMED | CANMODIFY);
	la_unmodify (curlas);
	flags |= DELETED;
    }
    (void) la_close (&fnlas[curfile]);
    (void) la_close (&lastlook[curfile].las);
    fileflags[curfile] = flags;
    curwin->winflgs &= ~TRACKSET;
    infotrack (NO);
    dlcurfn ();
    d_put (VCCAS);
    info (inf_file, len, names[curfile], YES);

    return CROK;
}

/*    Return YES if any element of the pathname 'name' is "..". */
Flag
dotdot (name)
char *name;
{
    Reg1 char *cp;

    for (cp = name; *cp; ) {
	if (   (cp == name || *cp++ == '/')
	    && *cp++ == '.'
	    && (   *cp == '\0'
		|| *cp++ == '.' && *cp == '\0'
	       )
	   )
	    return YES;
    }
    return NO;
}

/*    Remove the file from any workspaces in any windows where it occurs. */
void
dlcurfn ()
{
	dlwkfn ((S_wksp *) NULL, curfile);
}

/*    Remove workspace in any windows where it occurs. */
void
dlcurwk ()
{
	dlwkfn (curwksp, curfile);
}

static
void
dlwkfn (wsp, fn)
S_wksp *wsp;
Fn fn;
{
    Reg1 S_wksp *wk;
    Reg2 Small  ind;
    S_window *oldwin;
    Flag we;

    oldwin = curwin;
    we = (strcmp (names[fn], scratch) != 0);

Again:
    for (ind = Z; ind < nwinlist; ind++) {
	if (   (wk = winlist[ind]->wksp) != (S_wksp *) NULL
	    && (wsp == (S_wksp *) NULL || wsp == wk)
	    && wk->wfile == fn
	   ) Block {
	    Flag docurs;

	    if (docurs = (oldwin != winlist[ind]))
		savecurs ();
	    switchwindow (winlist[ind]);
	    if (docurs)
		chgborders = 2;
	    if (!swfile (NO)) {
		if (docurs)
		    chgborders = 2;
		if (we)
		    edscrfile (YES);
		else
		    eddeffile (YES);
	    }
	    switchwindow (oldwin);
	    if (!wkused (wk))
		freewk (wk);
	    if (docurs) {
		chgborders = 1;
		restcurs ();
	    }
	    goto Again;
	}
    }
There:
    for (wk = first_wksp; wk != (S_wksp *) NULL; wk = wk->next_wksp)
	if ((wsp == (S_wksp *) NULL || wsp == wk)
	    && wk->wfile == fn
	    && !wkused (wk)
	   ) {
		freewk (wk);
		goto There;
	}
    curfnum = getnumbywk (curwksp);
    infofnum = -1;
}
