/*    use and editfile routines. */

#include "e.h"
#include "e.cc.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.tt.h"
#include "e.wi.h"
#include <sys/stat.h>

void commedit ();
extern int curfnum;

/*    Do the "edit" command. */
Cmdret
edit ()
{
    char *str;

    if (opstr[0] == '\0') {
	switchfile ();
	return CROK;
    }
    if (*nxtop)
	return CRTOOMANYARGS;

    str = append (opstr, "\n");
    commedit (str);

    return CROK;
}

void
commedit (str)
char *str;
{
    Reg1 Short len;

    len = ch_dechars (str);
    sfree (str);
    deline[len - 1] = '\0';
    if (editfile (deline, (Ncols) -1, (Nlines) -1, 1, YES, NO) > 0 &&
	visualtabs) {
	savecurs ();
	drawtabs (curwin, WIN_ACTIVE);
	restcurs ();
    }
}

/*
    Installs 'file' as edited file in current window, with window upper-left
      at ['line', 'col'].  Use lastlook if -1.
    If 'file' is not there
	if 'mkopt' == 2, create the file.
	else if 'mkopt' == 1, give the user the option of making it
	else return
    Returns
      -1 if user does not want to make one,
       0 if error condition,
       1 if successfull.
    Writes screen (calls putupwin ()) if puflg is YES.
    Если temp, то файл не становится альтернативным
*/
Small
editfile (file, col, line, mkopt, puflg, temp)
char   *file;
Ncols   col;
Nlines  line;
Small   mkopt;
Flag    puflg;
Flag    temp;
{
    int retval;
    static Flag toomany = NO;
    Short   j;
    Flag    dwriteable,         /* directory is writeable */
	    fwriteable,         /* file is writeable */
	    isdirectory;        /* is a directory - don's set CANMODIFY */
    Flag    hasmultlinks = NO;
    La_stream  tlas;
    La_stream *nlas;
    char *dir = (char *) NULL;
    Reg1 Fn newfn;
    int nlink;

    if (file == (char *) NULL || *file == '\0')
	Goret (-1);
    /*  That the next statement is here is sort of a kludge, so that
    /*  when editfile() is called before mainloop, only the last file
    /*  edited will be able to set the hold.
    /**/
    loopflags.hold = NO;
#ifdef DEBUGDEF
    dbgpr ("Edit %s\n", file);
#endif
    if ((newfn = hvname (file)) != -1) {
	if (newfn == curfile)
	    puflg = NO;
#ifdef DEBUGDEF
	dbgpr ("Have %s as %d\n", file, newfn);
#endif
    }
    else if (   (newfn = hvoldname (file)) != -1
	     || (newfn = hvdelname (file)) != -1
	    ) {
	dwriteable = YES;
	goto asknew;
    }
    else {
	if (toomany || nopens >= MAXOPENS) {
	    mesg (ERRALL + 1, ediag(
"Too many files -- Editor limit!",
"Слишком много открытых файлов!"));
	    Goret (0);
	}
	/* find the directory */
	if ((j = dircheck (file, &dir, (char **) 0, NO, YES)) == -1)
	    Goret (0);

	if (dir == NULL || *dir == 0)
	    dwriteable = access (".", 2) >= 0; 
	else
	    dwriteable = access (dir, 2) >= 0;

	if ((j = filetype (file)) != -1) {
	    Fn tfn;

	    /* file already exists */
	    fwriteable = access (file, 2) >= 0;
	    if (j != S_IFREG &&  j != S_IFDIR ) {
		mesg (ERRALL + 1, ediag(
"Can only edit files",
"Можно редактировать только файлы"));
		Goret (0);
	    }
	    if (access (file, 4) < 0) {
		mesg (ERRALL + 1, ediag(
"File read protected.",
"Файл защищен по чтению."));
		Goret (0);
	    }
	    isdirectory = j == S_IFDIR;
	    if ((newfn = getnxfn ()) >= MAXFILES - 1)
		toomany = YES;
	    intok = YES;
	    if ((nlas = la_open (file, "", &tlas, (La_bytepos) 0))
		== NULL) {
#ifdef DEBUGDEF
		dbgpr ("Bad la_open %s, la_errno=%d\n", file, la_errno);
#endif
		intok = NO;
		if (la_errno == LA_INT)
		    mesg (ERRALL + 1, ediag("Interrupted","Прервано"));
		else
		    mesg (ERRALL + 1, ediag(
"Can't open the file ",
"Нельзя открыть файл "), file);
		Goret (0);
	    }
	    intok = NO;
	    if (nlas->la_file == la_chglas->la_file) {
		mesg (ERRALL + 1, ediag(
"Can't edit the 'changes' file",
"Нельзя редактировать файл изменений"));
		(void) la_close (nlas);
		Goret (0);
	    }
	    Block {
		La_file   *tlaf;
		tlaf = nlas->la_file;
#ifdef DEBUGDEF
		dbgpr ("Looking up %s\n", file);
#endif
		for (tfn = FIRSTFILE + NTMPFILES; tfn < MAXFILES; tfn++)
		    if (   (fileflags[tfn] & (INUSE | NEW)) == INUSE
			&& tlaf == fnlas[tfn].la_file
		       ) {
#ifdef DEBUGDEF
			dbgpr ("Same as %s\n", names[tfn]);
#endif
			if (fileflags[tfn] & DELETED)
			    break;
			fileflags[newfn] = 0;
			newfn = tfn;
			(void) la_close (nlas);
			toomany = NO;
			mesg (TELALL + 3,
#ifdef  LABTAM_BUG
ediag("EDIT: ","\376тение: "), names[tfn],
#else
ediag("EDIT: ","Чтение: "), names[tfn],
#endif
ediag(" (linked)", " (есть ссылки)"));
			d_flush ();
			loopflags.hold = YES;
			goto editit;
		    }
	    }
	    nopens++;
#ifdef  LABTAM_BUG
	    mesg (TELSTRT + 2, ediag("EDIT: ","\376тение: "), file);
#else
	    mesg (TELSTRT + 2, ediag("EDIT: ","Чтение: "), file);
#endif
	    if (isdirectory) {
		mesg (1, ediag("  (is a directory)", " (каталог)"));
		loopflags.hold = YES;
	    }
	    else if (1 < (nlink = fmultlinks (la_chan (nlas)))) Block {
		char numstr[10];

		hasmultlinks = YES;
		sprintf (numstr, "%d", nlink);
		mesg (3, ediag("  (has "," (имеет "),
numstr, ediag(" links)"," ссылок)"));
		loopflags.hold = YES;
	    }
	    mesg (TELDONE);
	    d_flush ();
	    if (dwriteable)
		fileflags[newfn] |= DWRITEABLE;
	    if (fwriteable)
		fileflags[newfn] |= FWRITEABLE;
	    if (dwriteable && !isdirectory && (!inplace || fwriteable)) {
		fileflags[newfn] |= CANMODIFY;
		if (inplace && hasmultlinks)
		    fileflags[newfn] |= INPLACE;
	    }
	    fileflags[newfn] |= UPDATE;
	}
	else
 asknew:
	    if (mkopt == 2)
		goto createit;
	else if (mkopt == 1) {
	    mesg (TELSTRT|TELCLR + 3, ediag(
"Do you want to create ","Создать "), file, "? ");
	    keyused = YES;
	    getkey (WAIT_KEY);
	    keyused = YES;
	    mesg (TELDONE);
	    if (key != CCPLFILE && key != CCMIFILE && !index("YyДд\n\r", key))
		Goret (-1);
 createit:
	    if (!dwriteable) {
		dirncheck (dir, YES, YES);
		Goret (0);
	    }
	    if ((newfn = getnxfn ()) >= MAXFILES - 1)
		toomany = YES;
	    fileflags[newfn] |= UPDATE | DWRITEABLE | FWRITEABLE
			     | CANMODIFY | NEW;
	    /* a NEW file can NOT be INPLACE */
	    nlas = la_open (file, "n", &tlas);
	}
	else
	    Goret (-1);
	names[newfn] = append (file, "");
#ifdef DEBUGDEF
	dbgpr ("%s: las=%x, lines=%d\n", file, nlas, la_lsize(nlas));
#endif
	if (   !la_clone (nlas, &fnlas[newfn])
	    || !la_clone (nlas, &lastlook[newfn].las)
	   )
	    la_abort ("la_clone failed in editfile()");
	(void) la_close (nlas);
    /*  elasdump (&fnlas[newfn], "open");        /**/
    }

editit:
    if (doedit (newfn, line, col, temp) != CROK)
	Goret(-1);
    curwin->winflgs &= ~TRACKSET;
    infotrack (NO);
    if (puflg) {
	getline (-1);
	putupwin ();
	limitcursor ();
	poscursor (curwksp->ccol, curwksp->clin);
	if (HelpActive)
	    HelpGotoMarg (NO);
    }
    retval = 1;
ret:
    if (dir)
	sfree (dir);

    return retval;
}

/*
    Now that we have determined that everything is OK, go ahead and
    install the file as element 'fn' in the list of files we are editing.
    If 'line' != -1 make it the line number of the current workspace;
    Similarly for 'col'.
    Если temp, то файл не становится альтернативным
*/
Cmdret
doedit (fn, line, col, temp)
Fn fn;
Nlines  line;
Ncols   col;
Flag temp;
{
    Reg1 S_wksp *lwksp;
    Reg2 S_wksp *cwksp;

    if (curwksp != (S_wksp *) NULL)
	getline (-1);

    if (temp && curwksp != (S_wksp *) NULL)
	lwksp = curwksp; /* Zeroed curwksp feather */
    else
	temp = NO;

    if ((cwksp = (S_wksp *) salloc (sizeof (S_wksp), NO)) == (S_wksp *) NULL)
	return NOMEMERR;
    insertwlist (cwksp, curwksp);
    curwin->wksp = cwksp;
    exchgwksp ();

    if (temp && !wkused (lwksp))
	freewk (lwksp);

    lwksp = &lastlook[fn];
    if (line != -1)
	lwksp->wlin = line;       /* line no of upper-left of screen */
    if (col != -1)
	lwksp->wcol = col;        /* col no of column 0 of screen */
    cwksp->wlin = lwksp->wlin;
    cwksp->wcol = lwksp->wcol;
    cwksp->ccol = lwksp->ccol;
    cwksp->clin = lwksp->clin;
    curlas = &cwksp->las;
    if (!la_clone (&fnlas[fn], curlas))
	la_abort ("la_clone failed in doedit()");
    cwksp->wfile = curfile = fn;
    curwksp = cwksp;
    HelpActive = ((fileflags[curfile] & HELP) != 0);
    curfnum = getnumbywk (curwksp);

    return CROK;
}

/*    Return the next file number that is not INUSE. */
static
Fn
getnxfn ()
{
    Reg1 Fn fn;

    for (fn = FIRSTFILE + NTMPFILES; fileflags[fn] & INUSE; )
	if (++fn >= MAXFILES)
	    fatal (FATALBUG, ediag("Too many open files",
				   "Слишком много открытых файлов"));
    fileflags[fn] = INUSE;

    return fn;
}
