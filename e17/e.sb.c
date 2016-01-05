/*
    misc subroutines
*/

#include "e.h"
#include "e.fn.h"
#ifdef UNIXV7
#include <ctype.h>
#endif
#include SIG_INCL
#include <sys/stat.h>
#include <varargs.h>
#include <pwd.h>
#include <fcntl.h>

extern struct passwd *getpwuid(), *getpwnam ();
extern void getpath ();
extern void cleanup ();
extern void la_abort ();
void fatal ();
static void dofatal ();
extern void fatalpr ();
void dbgpr ();
static void dodbgpr ();
extern void flushkeys ();
void edscrfile ();
extern void monexit ();
extern char *getenv ();
extern char *index ();
extern char *getlogin ();

/*
    Find full pathname for the executable file "name".
    Must be called before path is used, and if path is used
    in a child process, getpath should be been called in the PARENT
    so that it won't have to do any work on subsequent calls.
.
    use PATH environment variable!!!
*/
void
getpath (name, path, tryagain)
register char *name;
char **path;
Flag tryagain;
{
#ifdef ENVIRON
    register char *cp1, *cp2, *pp;
#endif

    if (*path && !tryagain)
	return;
#ifdef ENVIRON
    if ((pp = getenv("PATH")) &&
	name[0] != '/' &&
	strncmp (name, "./", 2) != 0 &&
	strncmp (name, "../", 3) != 0) {
	char *buf;
	register char *dir, *eod;

	cp1 = pp = append (pp, "");
	buf = append ("/", name);
	do {
	    dir = cp1;
	    if (eod = index (cp1, ':')) {
		*eod = '\0';
		cp1 = eod + 1;
	    }
	    if (!dir[0]) dir = ".";
	    cp2 = append (dir, buf);
	    if(access (cp2, 1) == 0) {
		sfree (buf);
		sfree (pp);
		*path = cp2;
		return;
	    }
	    sfree (cp2);
	}
	while (eod != (char *) 0);
	sfree (buf);
	sfree (pp);
    }
#endif ENVIRON

    *path = append (name, "");
}

/*
    Get user's login directory.
    Store it in the global 'mypath'.
    Return mypath.
*/
char *
getmypath ()
{
    Reg1 struct passwd *pwp;

    if (mypath)
	return mypath;

    if (!(myname = getlogin ())) {
	if (!(pwp = getpwuid (userid)))
	    fatal (FATALIO, "getpwuid failed!");
	myname = pwp->pw_name;
    }
    else {
	if (!(pwp = getpwnam (myname)))
	    fatal (FATALIO, "getpwnam failed!");
    }
    myname = append (myname, "");
    mypath = append (pwp->pw_dir, "");
    endpwent ();

    return mypath;
}

/*
    Cleanup before exiting
    If filclean != 0, remove changes file.
    If rmkeysflg != 0, remove keystroke file.
*/
void
cleanup (filclean, rmkeysflg)
Flag filclean;
Flag rmkeysflg;
{
    if (filclean && la_cfile != (char *) NULL) {
	if (la_chglas != (La_stream *) NULL)
	    la_close (la_chglas);
	unlink (la_cfile);
    }

    if (keyfile != (FILE *) NULL) { /* may be keyfile not yet open */
	fclose (keyfile);
	if (rmkeysflg) {
	    unlink (keytmp);
	    unlink (bkeytmp);
	}
    }

    if (dbgfile != (FILE *) NULL)
	fclose (dbgfile);
}

#ifdef  SIGNALS
/*
    Catch a signal, and call fatal.
*/
#ifdef SIGARG
int
sig (num)
Uint num;
{
/*NOXXSTR*/
#define SSNAMES 26
    static char *signames[SSNAMES][2] = {
	{0, 0},
	{"Hangup", "Обрыв связи"},
	{"Interrupt", "Прерывание"},
	{"Quit", "Выход"},
	{"Illegal instruction", "Неверная команда"},
	{"Trace/BPT trap", "Прерывание по trace/BPT"},
	{"IOT trap", "Прерывание IOT"},
	{"EMT trap", "Прерывание EMT"},
	{"Floating exception", "Ошибка плавающей арифметики"},
	{"Killed", "Убит"},
	{"Bus error", "Ошибка шины"},
	{"Segmentation fault", "Нарушение защиты памяти"},
	{"Bad system call", "Плохой системный вызов"},
	{"Broken pipe", "Оборвалась труба"},
	{"Alarm clock", "Прерывание по таймеру"},
	{"Terminated", "Завершение"},
	{"Signal 16", "16 сигнал"},
	{"Stopped (signal)", "Остановлен (по сигналу)"},
	{"Stopped", "Остановлен"},
	{"Continued", "Продолжен"},
	{"Child exited", "Изенилось состояние подпроцесса"},
	{"Stopped (tty input)", "Остановлен (ждет ввода с терминала)"},
	{"Stopped (tty output)", "Остановлен (ждет вывод на терминал)"},
	{"Tty input interrupt",  "Прерван ввод с терминала"},
	{"Cputime limit exceeded", "Исчерпан лимит процессорного времени"},
	{"Filesize limit exceeded", "Исчерпан лимит дискового пространства"}
    };
/*YESXSTR*/
    if (num > NSIG)
	num = 0;
    if (   num >= SSNAMES
	|| signames[num][0] == 0
       )
	fatal (FATALSIG, ediag("Signal %d.","Сигнал %d."), num);
    else
	fatal (FATALSIG, ediag("Caught signal: %s.",
			"Пришел сигнал: %s."),
			 ediag(signames[num][0], signames[num][1]));
}
#else
int
sig ()
{
    fatal (FATALSIG, ediag("Fatal Trap","Фатальный сигнал"));
}
#endif
#endif  SIGNALS

/*
    Called by the LA Package on fatal trouble.
    Pass the information on to a call to fatal().
*/
/* VARARGS1 */
void
la_abort (va_alist)
va_dcl
{
    Reg1 va_list fmt;

    va_start (fmt);
    dofatal (LAFATALBUG, fmt);
    va_end (fmt);
}

static char *bugwas;

/* VARARGS2 */
void
fatal (type, va_alist)
Flag type;
va_dcl
{
    Reg1 va_list fmt;

    va_start (fmt);
    dofatal (type, fmt);
    va_end (fmt);
}

static
void
dofatal (type, fmt)
Reg1 Flag    type;
Reg2 va_list fmt;
{
    char bugstr[256];
    Reg3 char *format;

    if (ischild)
	exit (-1);

    ischild = YES;      /* never fatal loops */

    if (type == FATALSEC)
	cleanup (YES, YES);

    if (windowsup) {
	screenexit (YES);
	windowsup = NO;
    }
    fixtty ();
/*NOXXSTR*/
    fatalpr (ediag(
"\7=*==*==*==*==*==*=\n\
The editor just crashed because:\n    ",
"\7=*==*==*==*==*==*=\n\
Авост редактора:\n    "));
/*YESXSTR*/

    switch (type) {
    case FATALEXDUMP:
	fatalpr (bugwas = ediag(
		 "you gave the command to exit with the \"dump\" option.\n",
		 "вы указали флаг \"dump\" в команде exit.\n"));
	break;

    case FATALMEM:
	fatalpr (bugwas = ediag(
"you ran out of memory space.\n",
"кончилась свободная память.\n"));
	break;

    case FATALIO:
/*NOXXSTR*/
	strcpy (bugstr, ediag("I/O error:\n", "ошибка ввода/вывода:\n"));
/*YESXSTR*/
	format = va_arg (fmt, char *);
	(void) vsprintf (&bugstr[strlen (bugstr)], format, fmt);
	strcat (bugstr, " ***\n");
	fatalpr (bugwas = bugstr);
	break;

    case FATALSIG:
    case FATALBUG:
    case LAFATALBUG:
    case FATALSEC:
/*NOXXSTR*/
	strcpy (bugstr, ediag("the editor just caught a ",
			"редактор поймал "));
	strcat (&bugstr[strlen (bugstr)],
#ifdef SIGNALS
		type == FATALSIG ? ediag("signal.\n","сигнал.\n") :
#endif SIGNALS
		(type == FATALBUG || type == FATALSEC) ?
				     ediag("bug in itself.\n",
					   "внутреннюю ошибку.\n") :
		ediag("bug in itself (LA Package).\n",
		      "внутреннюю ошибку (пакет LA).\n"));
/*YESXSTR*/
	format = va_arg (fmt, char *);
	(void) vsprintf (&bugstr[strlen (bugstr)], format, fmt);
	strcat (bugstr, "\n");
	fatalpr (bugwas = bugstr);
	break;
    }
    putchar ('\n');
    fflush (stdout);

    /* Выводим Crashdoc на терминал */
    if (type != FATALSEC)
    Block {
/*NOXXSTR*/
	/* Здесь НЕЛЬЗЯ использовать append */
#ifndef ANSI
	static char crashdoc_e[] = EDIR(Crashdoc);
	static char crashdoc_r[] = EDIR(Crashdoc_r);
#else
	static char crashdoc_e[] = EDIR("Crashdoc");
	static char crashdoc_r[] = EDIR("Crashdoc_r");
#endif
/*YESXSTR*/
	char *cp;

	cp = ediag (crashdoc_e, crashdoc_r);
	close (MAXSTREAMS - 1);
	close (MAXSTREAMS - 2);
	if (!cat (cp))
	    fatalpr (ediag(
"Please notify the system administrators\n\
that the editor couldn't read file %s.\n",
"Нельзя прочитать файл %s,\n\
сообщите системным программистам."),
cp);
    }

    if (keyfile != (FILE *) NULL)
	fclose (keyfile);
    if (dbgfile != (FILE *) NULL)
	fclose (dbgfile);
    if (la_chglas != (La_stream *) NULL)
	la_close (la_chglas);
    if (replaying && keysmoved && strcmp (inpfname, bkeytmp) == 0)
	mv (bkeytmp, keytmp);
    fflush (stdout);

    if (loginflg) {
	(void) signal (SIGINT, SIG_DFL);
	(void) signal (SIGQUIT, SIG_DFL);
	sleep (60);
    }

    /* save disk space by truncating changes file */
    if (type != FATALEXDUMP)
	close (creat (la_cfile, 0600));

    switch (type) {
    case FATALSIG:
    case FATALBUG:
    case FATALMEM:
    case FATALIO:
	fatalpr (ediag("\n\
(The following message comes to you from the shell:)\n\n",
"\n\
(Следующее сообщение придет от вашего интерпретатора команд:)\n\n"));
	fflush (stdout);

	cleanup ();	/* Было _cleanup(). Что это - я не знаю. */
#ifdef SIGNALS
	(void) signal (ABORT_SIG, SIG_DFL);
#endif SIGNALS
	abort();
    }

#ifdef PROFILE
    monexit (-1);
    /* NOTREACHED */
#else
    exit (-1);
#endif
}

#define NUMADRS 1500

/*
    'Grow salloc' - Use the new V7 realloc call.
    Allocs a new size block of memory for the item and copies the item
    from its old position to the new position.
    Returns the new position or NULL if salloc returned NULL and fatalflg
    is NO.
*/
char *
gsalloc (oldp, newsize, fatalflg)
char *oldp;
Uint newsize;
Flag fatalflg;
{
    Reg1 char *newp;
    extern char *realloc ();

    if ((newp = realloc (oldp, newsize)) == (char *) NULL)
	if (fatalflg)
	    fatal (FATALMEM, (char *) NULL);
	else
	    mesg (ERRALL + 1, ediag(
"You have run out of memory. Get out NOW!",
"Кончилась свободная память. Выходите ПОБЫСТРЕЕ!"));

    return newp;
}

/*
    Do a calloc().
*/
char *
okalloc (n)
Uint n;
{
    extern char *calloc ();

    return calloc (n, 1);
}

/*
    Do a calloc().
    Call fatal if calloc() returned NULL and fatalflg != 0.
    Else return the pointer to the alloced memory.
*/
char *
salloc (n, fatalflg)
Uint n;
Flag fatalflg;
{
    Reg1 char  *cp;
    extern char *calloc ();

    if ((cp = calloc (n, 1)) == (char *) NULL)
	if (fatalflg)
	    fatal (FATALMEM, (char *) NULL);
	else
	    mesg (ERRALL + 1, ediag(
"You have run out of memory. Get out NOW!",
"Кончилась свободная память. Выходите ПОБЫСТРЕЕ!"));

    return cp;
}


/*
    allocs enough space to hold the catenation of the strings name and ext.
    Returns pointer to new, alloced string.
*/
char *
append (name, ext)
char   *name;
char   *ext;
{
    int lname;
    register char  *c,
                   *d,
                   *newname;

    for (lname = 0, c = name; *c++; ++lname)
	continue;
    for (c = ext; *c++; ++lname)
	continue;
    for (newname = c = salloc (lname + 1, YES), d = name; *d; *c++ = *d++)
	continue;
    for (d = ext; *c++ = *d++;)
	continue;
    return newname;
}

/*
    Copy s1 to s2 and return address of '\0' at end of new s2.
*/
char *
copy (s1, s2)
register char *s1;
register char *s2;
{
    while (*s2++ = *s1++)
	continue;
    return --s2;
}

/*
    Converts string s to int and returns value in i.
    Returns pointer to the first char encountered that is not part
    of a valid decimal number.
    If the returned pointer == s, then no number was converted.
*/
char *
s2i (s, i)
char   *s;
int    *i;
{
    Reg1 char   lc;
    Reg2 int    c;
    Reg3 Short  val;
    Flag    maxi;
    Short   sign;

    maxi = NO;
    sign = 1;
    val = 0;
    lc = Z;
    for (c = *s; ; lc = c, c = *++s) {
	c = U(c);
	if (isdigit (c)) {
	    if (maxi)
		continue;
	    if ((val = 10 * val + c - '0') < 0 && sign > 0)
		maxi = YES;
	    continue;
	}
	else if (c == '-' && lc == Z) {
	    sign = -1;
	    continue;
	}
	else if (lc == '-')
	    s--;
	break;
    }
    if (maxi)
	*i = MAXINT;
    else
	*i = val * sign;
    return s;
}

/*
    Rename file name1 to file name2 with link() and unlind().
    Return YES if all went OK, else NO.
*/
Flag
mv (name1, name2)
char name1[];
char name2[];
{
    unlink (name2);
    return     link (name1, name2) != -1
	  && unlink (name1) != -1;
}

/*
    Like a printf, but the output also goes out to the debug output file,
    if any.
*/
/* VARARGS1 */
void
fatalpr (va_alist)
va_dcl
{
    Reg1 va_list fmt;
    char *format;

    va_start (fmt);
    dodbgpr (fmt);
    va_end (fmt);

    va_start (fmt);
    format = va_arg (fmt, char *);
    (void) vprintf (format, fmt);
    va_end (fmt);
}

/* VARARGS1 */
void
dbgpr (va_alist)
va_dcl
{
    Reg1 va_list fmt;

    va_start (fmt);
    dodbgpr (fmt);
    va_end (fmt);
}

static
void
dodbgpr (fmt)
va_list fmt;
{
    char *format;

    if (dbgfile != (FILE *) NULL) {
	format = va_arg (fmt, char *);
	(void) vfprintf (dbgfile, format, fmt);
	fflush (dbgfile);
    }
}

/*
    Flush the keys to the keystroke file.
*/
void
flushkeys ()
{
    if (fflush (keyfile) == EOF)
	fatal (FATALIO, ediag(
"ERROR WRITING keys FILE.",
"Ошибка записи файла протокола."));
    numtyp = 0;
}

/*
    Return YES if it is ok to modify curfile, else NO.
*/
Flag
okwrite ()
{
    return !!(fileflags[curfile] & CANMODIFY);
}

/*
    Copy one file to another.
    If 'file1' is non-NULL, try to open it for reading,
    else assume 'fd1' is already open.
    If 'file2' is non-NULL, try to creat it for writing,
    else assume 'fd2' is already open for writing.
    If 'doutime' is non-0, set times of file2 to times of file1 (only
    possible if 'file2' is non-null.
    Returns:
      0  All went ok
     -1  Can't open 'file1'
     -2  Can't creat 'file2'
     -3  Read error
     -4  Write error
*/
/*VARARGS4*/
Small
filecopy (file1, ff1, file2, ff2, doutime, crmode)
Ff_stream *ff1;
Ff_stream *ff2;
char *file1;
char *file2;
Flag doutime;
int crmode;
{
    register int j;
    Small retval;
    time_t times[2];

    if (   file1
	&& (ff1 = ff_open (file1, 0, 0)) == (Ff_stream *) NULL
       ) {
	if (!file2)
	    ff_close (ff2);
	return -1;
    }

    if (file2) {
	if ((j = open (file2, O_RDWR|O_TRUNC|O_CREAT, crmode)) < 0
	    || (ff2 = ff_fdopen(j, 1, 0)) == (Ff_stream *) NULL) {
	    (void) close(j);
	    if (!file1)
		ff_close (ff1);
	    return -2;
	}
    }
    else
	doutime = NO;

    if (doutime) Block {
	struct stat stbuf;

	fstat (ff_fd(ff1), &stbuf);
	times[0] = stbuf.st_atime;
	times[1] = stbuf.st_mtime;
    }

    for (;;) {
	if ((j = ff_getc (ff1)) == EOF) {
	    if (errno == 0) {
		retval = 0;
		break;
	    }
	    else
		retval = -3;
	}
	else if (ff_putc (j, ff2) == EOF)
	    retval = -4;
	else
	    continue;
	unlink (file2);
	break;
    }
    ff_close (ff1);
    ff_close (ff2);
    if (doutime)
	utime (file2, times);
    return retval;
}

/* Здесь НЕЛЬЗЯ использовать malloc */
Flag
cat (name)
char *name;
{
    Reg1 int c;
    Reg2 FILE *f;
    char buf[BUFSIZ];

    if ((f = fopen (name, "r")) == (FILE *) NULL)
	return NO;

    if (!setflock (fileno (f), 0, 0)) {
	fclose (f);
	return NO;
    }

    setbuf (f, buf);

    while ((c = getc (f)) != EOF)
	putchar (c);

    if (ferror (f)) {
	fclose (f);
	return NO;
    }
    fclose (f);

    return YES;
}

/*
    Edit the 'scratch' file.
    If that fails, edit the 'default' file.
*/
void
edscrfile (puflg)
Flag puflg;
{
    if (editfile (scratch, (Ncols) -1, (Nlines) -1, 2, puflg, NO) != 1)
	(void) eddeffile (puflg);
}

#ifdef PROFILE

Flag profiling;

/*
    call monitor(0), move mon.out to /usr/tmp/e/, and exit.
*/
void
monexit (status)
int status;
{
    static char template[] = "/usr/tmp/e/pXXXXXX";
    char tempfile[sizeof template];
    extern char *mktemp ();

    if (profiling) {
	strcpy (tempfile, template);
	mktemp (tempfile);
	monout (tempfile);
	profil ((char *) 0, 0, 0, 0);
    }
    _cleanup ();
    _exit (status);
}

#ifdef VAX_MONITOR
/* /usr/src/libc/mon.c modified to take a filename other than mon.out */
static int *sbuf, ssiz;

monout(name)
char *name;
{
	register o;

	profil(0, 0, 0, 0);
	o = creat(name, 0666);
	write(o, (char *) sbuf, ssiz);
	close(o);
}

monitor(lowpc, highpc, buf, bufsiz, cntsiz)
char *lowpc, *highpc;
int *buf, bufsiz;
{
	register o;
	struct phdr {
		int *lpc;
		int *hpc;
		int ncnt;
	};
	struct cnt {
		int *pc;
		long ncall;
	};

	if (lowpc == 0) {
		monout ("mon.out");
		return;
	}
	sbuf = buf;
	ssiz = bufsiz;
	buf[0] = (int)lowpc;
	buf[1] = (int)highpc;
	buf[2] = cntsiz;
	o = sizeof(struct phdr) + cntsiz*sizeof(struct cnt);
	buf = (int *) (((int)buf) + o);
	bufsiz -= o;
	if (bufsiz<=0)
		return;
	o = ((highpc - lowpc)>>1);
	if(bufsiz < o)
		o = ((float) bufsiz / o) * 32768;
	else
		o = 0177777;
	profil(buf, bufsiz, lowpc, o);
}
#else
static int *sbuf, ssiz;

monout(name)
char *name;
{
	register o;

	profil(0, 0, 0, 0);
	o = creat(name, 0666);
	write(o, (char *) sbuf, ssiz<<1);
	close(o);
}

monitor(lowpc, highpc, buf, bufsiz, cntsiz)
char *lowpc, *highpc;
int *buf, bufsiz;
{
	register o;

	if (lowpc == 0) {
		monout("mon.out");
		return;
	}
	ssiz = bufsiz;
	buf[0] = (int)lowpc;
	buf[1] = (int)highpc;
	buf[2] = cntsiz;
	sbuf = buf;
	buf += 3*(cntsiz+1);
	bufsiz -= 3*(cntsiz+1);
	if (bufsiz<=0)
		return;
	o = ((highpc - lowpc)>>1) & 077777;
	if(bufsiz < o)
		o = ((long)bufsiz<<15) / o;
	else
		o = 077777;
	profil(buf, bufsiz<<1, lowpc, o<<1);
}
#endif

#endif

#ifdef NOVPRINTF
int
vprintf (format, fmt)
char *format;
va_list fmt;
{
    return vfprintf (stdout, format, fmt);
}

int
vsprintf (str, format, fmt)
char *str, *format;
va_list fmt;
{
    FILE _strout;

    _strout._base = _strout._ptr = str;
    _strout._cnt = MAXSHORT;
    _strout._file = -1;
    _strout._flag = _IOWRT;
#ifdef _IOSTRG
    _strout._flag |= _IOSTRG;
#endif
    _doprnt (format, fmt, &_strout);
    putc ('\0', &_strout);
    return 0;
}

int
vfprintf (iop, fmtptr, fmt)
FILE *iop;
Reg2 char *fmtptr;
Reg1 va_list fmt;
{
    _doprnt (format, fmt, iop);
    return 0;
}
#endif /*NOVPRINTF*/
