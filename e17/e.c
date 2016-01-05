#include "e.h"
#include "e.e.h"
#include "e.tt.h"
#include "e.wi.h"
#include "e.fn.h"
#include "e.inf.h"
#include <sys/stat.h>

#ifdef TTYNAME
extern char *ttyname ();
#endif

Flag crashed;       /* предыдущий сеанс сбойнул */
extern void setitty ();
extern void getout ();
extern void setotty ();

Flag helpflg;
Flag waschoice = NO; /* dorecov уже вызывалась */
Flag chosereplay;   /* replay or recovery */
Flag usetags;
extern Nlines infosize;
extern Scols inf_avail;
extern int infofnum;

/*    Присваивает глобальному указателю 'progname' имя вызванного редактора */
void
getprogname (arg0)
char arg0[];
{
    Reg1 char *cp;
    Reg2 char lastc;

    progname = arg0;
    for (lastc = Z, cp = arg0; *cp; cp++) {
	if (lastc == '/')
	    progname = cp;
	lastc = *cp;
    }
    cp = progname;
    if (loginflg = *progname == '-')
	progname = LOGINNAME;
}

/*
    Ask the user how he wants to deal with a crashed or aborted session.
    Type is 0 for crashed, 1 for aborted.
    If the user opts for a recovery or replay, anything which was set by
    checkargs() must be set here to the way we want it for the replay.
*/
#ifndef ANSI
#define RECOVERMSG  EDIR(recovermsg)
#else
#define RECOVERMSG  EDIR("recovermsg")
#endif
void
dorecov (type)
Reg3 Flag type;
{
    waschoice = YES;
    if (norecover)
	return;
    (*kbd.kb_end) ();
    d_put (VCCEND);
    d_flush ();
    putchar ('\n');
    windowsup = NO;
    fixtty ();

    for (;;) Block {
	char line[10];
	int tmp;
	char *cp;
	Fd tfd;
	Ff_stream *ff_tmp;
/*NOXXSTR*/
	static char recovmsg[] = RECOVERMSG;
/*YESXSTR*/
	cp = append (recovmsg, ediag ("", "_r"));

	if (!cat (cp))
	    fatalpr (ediag("Please notify the system administrators\n\
that the editor couldn't read file \"%s\".\n",
"Редактор не может читать файл \"%s\",\n\
сообщите системным программистам.\n"),
cp);

	sfree (cp);
	printf(ediag("\n\
Type the number of the option you want then hit <RETURN>: ",
"\n\
Введите выбранный номер и нажмите <ВК>: "));
	fflush (stdout);
	fgets (line, sizeof(line), stdin);
	if (feof (stdin))
	    getout (type, "");
	switch (*line) {
	case '1':
	    recovering = YES;
	    silent = YES;
	    if (0) {
	case '2':
		recovering = NO;
		silent = NO;
	    }
	    replaying = YES;
	    helpflg = NO;
	    notracks = NO;
	    chosereplay = YES;
	    break;
	case '3':
	    cleanup (YES, YES);
	    break;
	case '4':
	    getout (type, "");
	default:
	    continue;
	}
	break;
    }
    setitty ();
    setotty ();
    (*kbd.kb_init) ();                  /* initialize keyboard */
    (*term.tt_ini1) ();                 /* initialize the terminal */
    windowsup = YES;
}

/*
    Initialize the #o and #c pseudo-files.
    Set up the qbuffers (pick, close, etc.).
*/
void
makescrfile ()
{
    Reg1 int j;

    for (j = FIRSTFILE; j < FIRSTFILE + NTMPFILES; j++) {
	names[j] = append (tmpnames[j - FIRSTFILE], ""); /* must be freeable*/
	/*  firstlas используется для добавления строк, потоки
	/*  дублируются в lastlook wksp чтобы позиционирование не затрагивало
	/*  позицию в firstlas[j]
	/**/
	if (!la_open ("", "t", &fnlas[j]))
	    getout (YES, ediag ("Can't open temp file (%s)",
				"Нельзя открыть временный файл (%s)"),
			 names[j]);
	if (!la_clone (&fnlas[j], &lastlook[j].las))
	    la_abort ("la_clone 1 failed in makescrfile()");
	lastlook[j].wfile = j;
	fileflags[j] = INUSE | CANMODIFY;
    }

    for (j = 0; j < NQBUFS; j++)
	if (!la_clone (&fnlas[qtmpfn[j]], &qbuf[j].buflas))
	    la_abort ("la_clone 2 failed in makescrfile()");
}

/*
    Make an initial state without reference to the state file.
    If 'nofileflg' != 0, edit the 'scratch' file.
*/
void
makestate (nofileflg)
Flag nofileflg;
{
    Reg1 S_window *window;

    freewlist ();
    stabs = NTABS;
    tabs = (ANcols *) salloc (stabs * sizeof *tabs,YES);
    ntabs = 0;
    tabevery ((Ncols) TABCOL, (Ncols) 0, (Ncols) (((NTABS / 2) - 1) * TABCOL),
	      YES);

    nwinlist = 1;
    window = winlist[0] = (S_window *) salloc (SWINDOW, YES);
    setupwindow (winlist[0], 0, 0,
		 term.tt_width - 1, term.tt_height - 1 - NPARAMLINES, 1);
    drawborders (window, WIN_ACTIVE);
    switchwindow (window);
    poscursor (0, 0);
    if (nofileflg) {
	edscrfile (NO);
	poscursor (0, 0);
    }
}

/*    Set up the displays on the info line. */
void
infoinit ()
{
    Reg1 Scols col;

    inf_insert = 0; col = 4;    /* "INS" */

    inf_inreg = col; col += 2;  /* "R" */

    inf_track = col; col += 4;  /* "TRK" */

    inf_range = col; col += 5;  /* "=RNG" */

    inf_mark = col; col += 5;   /* "MARK" */

    inf_area = col; col += 8;   /* "30x16" etc. */

    inf_macro = col; col += 4;  /* "MAC" */

    inf_mname = col;

    col = 38;
    /* inf_at = col; col += 3;     /* "At"         */
    inf_line = col; col += 13;   /* line         */
    inf_in = col; col += 7;     /* "in"         */
    inf_file = col;             /* filename     */
    inf_avail = infowin.redit - inf_file;

/**********************************************
    d_put (VCCMD);
    info (inf_at, 2, ediag("At","На"), NO);
    d_put (VCCMD);
    info (inf_in, 2, ediag("in","в"), NO);
***********************************************/
    infosize = -1;
    infoline = -1;
    infofnum = -1;
    infofile = NULLFILE;
    if (insmode) {
	insmode = NO;
	tglinsmode ();
    }
}

