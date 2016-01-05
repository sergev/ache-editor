/*   File system calls involving stat, etc. */

#include "e.h"
#include <sys/stat.h>

int  eddeffile ();

/*
    Return the S_IFMT bits of the requested file,
    or -1 if the stat call fails.
*/
int
filetype (name)
char name[];
{
    struct stat statbuf;

    if (stat (name[0] ? name : ".", &statbuf) == -1)
	return -1;
    return statbuf.st_mode & S_IFMT;
}

/*    Return the mode bits anded with 0777 for fildes. */
int
fgetpriv (fildes)
Fd fildes;
{
    struct stat statbuf;

    fstat (fildes, &statbuf);
    return statbuf.st_mode & 0777;
}

/*
    If dir != NULL, put a pointer to an alloced string containing
      the directory part into *dir.  If no '/' in name, *dir
      will be a null string, else it will be the directory name
      with a '/' as the last character.
    If file != NULL, put a pointer to the file part into *file.
    Check the directory for access, insist on writeability if
      writecheck == YES.
    Complain with calls to mesg if errors == YES.
    Truncate the file part of name to FNSTRLEN (14 characters).
    Returns file type (S_IFMT) if success or -1 if failure.
*/
int
dircheck (name, dir, file, writecheck, errors)
char  *name;
char **dir;
char **file;
Flag   writecheck;
Flag   errors;
{
    Flag slashinname = NO;
    char *tdir;
    Reg1 char *cp1;
    Reg2 char *cp2;

    for (cp1 = cp2 = name; *cp1; cp1++) {
	if (*cp1 == '/') {
	    slashinname = YES;
	    cp2 = cp1;
	}
    }
    if (slashinname) {
	*cp2 = '\0';
	tdir = append (name, "/");
	*cp2++ = '/';
    }
    else {
	tdir = append ("", "");
	cp2 = name;
    }
    if (strlen (cp2) > FNSTRLEN)
	cp2[FNSTRLEN] = '\0';
    if (file != NULL)
	*file = cp2;

    {
	register int   j;

	j = dirncheck (tdir, writecheck, errors);
	if (dir == NULL)
	    sfree (tdir);
	else
	    *dir = tdir;
	return j;
    }
}

/*    Does the work for dircheck above. */
int
dirncheck (tdir, writecheck, errors)
Reg1 char *tdir;
Flag writecheck;
Flag errors;
{
    Flag putback = NO;
    int retval;
    Reg2 int filefmt;
    Reg3 char *cp;
    Reg4 char *dirname;

    if ((cp = tdir)[0] != '\0')
	for (; ; cp++)
	    if (*cp == '\0') {
		if (*--cp == '/' && cp != tdir) {
		    *cp = '\0';
		    putback = YES;
		}
		break;
	    }

    retval = -1;
    dirname = tdir[0] != '\0' ? tdir : ".";
    if ((filefmt = filetype (dirname)) == -1) {
	if (errors) {
	    mesg (ERRSTRT + 1, ediag("Can't find", "Нельзя найти"));
	    goto ret;
	}
    }
    else if (filefmt != S_IFDIR) {
	if (errors)
	    mesg (ERRALL + 2, tdir, ediag(" is not a directory",
					  " не каталог"));
    }
    else if (access (dirname, 1) < 0) {
	if (errors) {
	    mesg (ERRSTRT + 1, ediag("Can't work in","Недоступен"));
	    goto ret;
	}
    }
    else if (writecheck && access (dirname, 2) < 0) {
	if (errors) {
	    mesg (ERRSTRT + 1, ediag("Can't write in","Нельзя писать в"));
 ret:
	    if (tdir[0] == '\0')
		mesg (ERRDONE + 1, ediag(" current directory",
					 " текущий каталог"));
	    else
		mesg (ERRDONE + 2, ediag(" dir: "," каталог: "), tdir);
	}
    }
    else
	retval = filefmt;
    if (putback)
	*cp = '/';
    return retval;
}

/*    Return YES if number of links to name is > 1 else NO. */
Flag
multlinks (name)
char name[];
{
    struct stat statbuf;

    if (stat (name[0] ? name : ".", &statbuf) == -1)
	return 0;
    return statbuf.st_nlink;
}

/*    Return YES if number of links to fildes is > 1 else NO. */
Flag
fmultlinks (fildes)
Fd fildes;
{
    struct stat statbuf;

    if (fstat (fildes, &statbuf) == -1)
	return 0;
    return statbuf.st_nlink;
}

/*    Return the Fn associated with name if we have it already, else -1. */
Fn
hvname (name)
char *name;
{
    Reg1 Fn i;

    for (i = FIRSTFILE; i < MAXFILES; ++i)
	if (   (fileflags[i] & (INUSE | DELETED)) == INUSE
	    && strcmp (name, names[i]) == 0
	   )
	    return i;
    return -1;
}

/*
    Return the Fn associated with name if we have it already
    as an old name of a renamed file, else -1.
*/
Fn
hvoldname (name)
char *name;
{
    Reg1 Fn i;

    for (i = FIRSTFILE; i < MAXFILES; ++i)
	if (   (fileflags[i] & (INUSE | RENAMED)) == (INUSE | RENAMED)
	    && strcmp (name, oldnames[i]) == 0
	   )
	    return i;
    return -1;
}

/*
    Return the Fn associated with name if we have it already
    as a deleted file, else -1.
*/
Fn
hvdelname (name)
char *name;
{
    Reg1 Fn i;

    for (i = FIRSTFILE; i < MAXFILES; ++i)
	if (   (fileflags[i] & (INUSE | DELETED)) == (INUSE | DELETED)
	    && strcmp (name, names[i]) == 0
	   )
	    return i;
    return -1;
}

/*    Edit the default file. */
int
eddeffile (puflg)
Flag puflg;
{
    int retval;
    char *cp;

    cp = append (deffile, ediag ("", "_r"));
    if ((retval = editfile (cp, (Ncols) 0, (Nlines) 0, 0, puflg, NO)) <= 0)
	mesg (ERRALL + 1, ediag("Default file gone: notify sys admin.",
	   "Отсутствует специальный файл: обратитесь к системным программистам."));
    else {
	deffn = curfile;
	fileflags[curfile] &= ~CANMODIFY;
    }
    sfree (cp);

    return retval;
}

