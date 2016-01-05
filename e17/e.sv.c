/*    file saving routines */

#include "e.h"
#include "e.tt.h"
#include "e.m.h"
#include "e.mk.h"
#include "e.cm.h"
#include <sys/stat.h>
#include <fcntl.h>
#ifdef  SECURITY
extern Flag nomoretime, nomorecommand, nowritefile;
#endif

extern char *mktemp ();

/*    Do the "save" command.*/
/*    Если без параметров -- текущий файл */
Cmdret
save ()
{
    char *arg;

    if (curmark)
	return NOMARKERR;
    if (*nxtop)
	return CRTOOMANYARGS;
    arg = (opstr[0] == '\0') ? (char *) NULL : opstr;

    (void) savefile (arg, curfile, NO);

    return CROK;
}

/*
  There are 2 cases to deal with.
    1. Doing a save as part of exit sequence
       a. Only remove the backup file
       b. Do the saving
    2. Doing a "save" command of fn to filename
  If filename is a null pointer, then we are doing case 1, else case 2
.
    savefile (filename, fn, svinplace, rmbak)
	if filename == NULL && rmbak != 0
	    remove backup file if any
	    return
	if filename != NULL
	    if filename is open for editing (this includes the case where
	      the filename is the name of the file)
		disallow the save
		return NO;
	else filename == NULL, meaning save to this file,
	    if not EDITED
		return YES
	if svinplace and file has multiple links
	    Copy original to backup
	    if filename == NULL
		Do an la_freplace to the backup filename
	    la_lflush the file to the original file
	else
	    write out file number fn to a temp file called ,esvXXXXXX
		where XXXXX is unique.
	    rename the original file to the backup name
	    rename the temp file to the original name
	if filename == 0
	    clear EDITED and CANMODIFY
	return YES;
.
	normal return is YES; error is NO;
*/
/*VARARGS3*/
Flag
savefile (filename, fn, svinplace, rmbak)
char   *filename;
Fn      fn;
Flag svinplace;
Flag rmbak;
{
    char   *origfile,       /* origfile to be saved                      */
	   *dirname,        /* the directory for the save */
	   *filepart,       /* filename part of pathname */
	   *bakfile;
    Reg1 Short  tmp;
    Reg2 Short  j;
    Flag hasmultlinks;
    Flag TmpBak;            /* Файл уже двигали в .bak */

    putline (YES);
    bakfile = (char *) NULL;
#ifdef  SECURITY
    if (nomoretime)
	return YES;
#endif
    if (filename == (char *) NULL)
	origfile = names[fn];
    else {
	if (   hvname (filename) != -1
	    || hvdelname (filename) != -1
	    || hvoldname (filename) != -1
	   ) {
	    mesg (ERRALL + 1, ediag(
"Can't save to a file we are using",
"Нельзя записать в файл, который уже используется"));
	    return NO;
	}
	else if (   (j = filetype (origfile = filename)) != -1
		 && j != S_IFREG
		) {
	    mesg (ERRALL + 1, ediag(
"Can only save to files",
"Можно записывать только в файлы"));
	    return NO;
	}
    }
#ifdef  SECURITY
    if (nomorecommand)
	return YES;
#endif

    if (dircheck (origfile, &dirname, &filepart, YES, YES) == -1) {
	sfree (dirname);
	return NO;
    }

    TmpBak = ((fileflags[fn] & SAVED) != 0);

#ifdef  SECURITY
    if (nowritefile)
	return YES;
#endif
    if (TmpBak)
	bakfile = mktemp (append (dirname, ",esbXXXXXX"));
    else {
	/* make the backup name */
	if (prebak[0] != '\0') Block {
	    char *cp;

	    cp = append (prebak, filepart);
	    bakfile = append (dirname, cp);
	    sfree (cp);
	}
	if (postbak[0] != '\0')
	    bakfile = append (origfile, postbak);
	if (filename == (char *) NULL && rmbak) {
	    unlink (bakfile);
ok1:
	    sfree (dirname);
	    sfree (bakfile);
	    return YES;
	}
    }

    if (filename == (char *) NULL) Block {
	Fn tmp;

	if (   (tmp = hvoldname (origfile)) != -1
	    && !svrename (tmp)
	   ) {
 err2:      sfree (dirname);
	    sfree (bakfile);
	    return NO;
	}
	if (   (fileflags[tmp = fn] & RENAMED)
	    && !svrename (tmp)
	   )
	    goto err2;
	if (!la_modified (&fnlas[fn])) {
	    mesg (ERRALL + 1, ediag(
"This file is not modified sinc last update",
"Этот файл не изменялся со времени последней записи"));
	    goto ok1;
	}
    }

    hasmultlinks = (multlinks (origfile) > 1);

    if (svinplace) Block {
	Fd origfd;
	Ff_stream *ff_orig;
	int origpriv;
	Nlines ltmp;

	mesg (TELALL + 3, ediag("SAVE: ","Запись: "), origfile,
	      hasmultlinks ?
ediag(" (preserve link)", " (ссылки сохраняются)") :
ediag(" (inplace)", " (на то же место)"));
	d_flush ();
	/* copy orig to backup */
	sfree (dirname);

	(void) unlink (bakfile);
	if ((ff_orig = ff_open (origfile, 0, 0)) == (Ff_stream *) NULL) {
err1:      mesg (ERRALL + 3,
			    ediag("Unable to copy ","Нельзя переписать "),
			    origfile, ediag(" to backup", "в back-файл"));
	    sfree (bakfile);
	    return NO;
	}
	origpriv = fgetpriv (ff_fd(ff_orig));
	if ((j = filecopy ((char *) NULL, ff_orig,
			   bakfile, (Ff_stream *) NULL,
			   YES, fgetpriv (la_chan (&fnlas[fn]))))
	    == -2) {
	    mesg (ERRALL + 2, ediag("Unable to create back-file ",
			"Нельзя создать back-файл "), bakfile);
	    sfree (bakfile);
	    return NO;
	}
	else if (j < 0) {
	    if (TmpBak)
		(void) unlink (bakfile);
	    goto err1;
	}
	fileflags[fn] |= SAVED;     /* Saved to BAK */

	if (   filename == (char *) NULL
	    && !la_freplace (bakfile, &fnlas[fn])
	   ) {
	    if (TmpBak)
		(void) unlink (bakfile);
	    goto err1;
	}
#ifndef CHSIZE
	/* I wish that I could write to the file and then truncate it to the
	/* written length, rather than doing a creat on it and releasing
	/* all those blocks to the free list possibly to be gobbled up
	/* by someone else, leaving me stranded.
	/* But current unix can't truncate except by doing a creat.
	/**/
	if ((origfd = creat (origfile, origpriv)) == -1) {
err3:
	    mesg (ERRALL + 2,
ediag("Unable to create ", "Нельзя создать "), origfile);
	    if (TmpBak)
		(void) unlink (bakfile);
	    sfree (bakfile);
	    return NO;
	}
#else  /*CHSIZE*/
	if ((origfd = open (origfile, 1)) == -1) {
err3:
	    mesg (ERRALL + 2,
ediag("Unable to open for write ", "Нельзя открыть на запись "), origfile);
	    if (TmpBak)
		(void) unlink (bakfile);
	    sfree (bakfile);
	    return NO;
	}
#endif  /*CHSIZE*/
	if (!setflock (origfd, 1, 0)) {
	    close (origfd);
	    goto err3;
	}
/*  Может менять аргументы */
	ltmp = la_lflush (&fnlas[fn], (La_linepos) 0,
		       la_lsize (&fnlas[fn]), origfd, NO);
	if (TmpBak)
	    (void) unlink (bakfile);
	sfree (bakfile);

	if (ltmp != la_lsize (&fnlas[fn])) {
    WrErr:
	    mesg (ERRALL + 2,
ediag("Error to writing to ", "Ошибка записи в "), origfile);
	    close (origfd);
	    return NO;
	}
#ifdef  CHSIZE
	if (chsize (origfd, (long) la_bsize (&fnlas[fn])) == -1)
	    goto WrErr;
#endif
	(void) close (origfd);
    }
    else Block {  /* not inplace */
	Fd tempfd;
	struct stat stbuf;
	char *tempfile;
	Nlines ltmp;

	mesg (TELALL + 3, ediag("SAVE: ","Запись: "), origfile,
	      hasmultlinks ?
	   ediag(" (break link)", " (ссылки не сохранены)") : "");
	d_flush ();
	tempfile = mktemp (append (dirname, ",esvXXXXXX"));
	sfree (dirname);

	if ((tempfd = open (tempfile, O_WRONLY|O_CREAT|O_EXCL, 0600)) < 0) {
    err4:
	    mesg (ERRALL + 3, ediag(
"Unable to create tmp file: \"",
"Невозможно создать временный файл: \""), tempfile, "\"!");
retno:      sfree (tempfile);
	    return NO;               /* error */
	}
	if (!setflock (tempfd, 1, 0)) {
	    unlink (tempfile);
	    close (tempfd);
	    goto err4;
	}
/*  Может менять аргументы */
	ltmp = la_lflush (&fnlas[fn], (La_linepos) 0,
		       la_lsize (&fnlas[fn]), tempfd, NO);
	if (ltmp != la_lsize (&fnlas[fn])) {
#ifdef DEBUGDEF
	    dbgpr ("flush %s %d of %d lines\n", tempfile, ltmp,
		   la_lsize (&fnlas[fn]));
#endif
	    mesg (ERRALL + 3, ediag(
"Error saving ", "Ошибка записи "), origfile,
ediag(" to disk", " на диск"));
	    unlink (tempfile);
	    close (tempfd);
	    goto retno;              /* error */
	}
	close (tempfd);

	if ((tmp = hvdelname (origfile)) != -1) {
	    unlink (origfile);
	    fileflags[tmp] &= ~(INUSE | DELETED);
	}

	if ( !TmpBak
	  && access (origfile, 0) == 0
	  && !mv (origfile, bakfile)) {
	    mesg (ERRALL + 4,
ediag("Can't rename ","Нельзя переименовать "),origfile,
ediag(" to "," в "),bakfile);
	    sfree (bakfile);
	    unlink (tempfile);
	    sfree (tempfile);
	    return NO;
	}
	sfree (bakfile);
	fileflags[fn] |= SAVED; /* Saved to BAK */

	if (!mv (tempfile, origfile)) {
	    mesg (ERRALL + 4,
ediag("Can't rename ","Нельзя переименовать "),tempfile,
ediag(" to "," в "),origfile);
	    unlink (tempfile);
	    sfree (tempfile);
	    return NO;
	}

	if (!(fileflags[fn] & NEW)) {
	    if (userid == 0) {
		fstat (la_chan (&fnlas[fn]), &stbuf);
		chown (origfile, stbuf.st_uid, stbuf.st_gid);
		chmod (origfile, stbuf.st_mode & 0777);
	    }
	    else
		chmod (origfile, fgetpriv (la_chan (&fnlas[fn])));
	}
	sfree (tempfile);
    }

    if (filename == (char *) NULL) {
	fileflags[fn] &= ~(NEW | DWRITEABLE);
	la_unmodify (&fnlas[fn]);
    }

    return YES;
}

/*
    Do the actual renaming of the file on disk.
    Called as part of the exit saving sequence.
    If all went OK return YES, else NO.
*/
Flag
svrename (fn)
Reg1 Fn fn;
{
    Block {
	Reg2 char *old;
	Reg3 char *new;

	old = oldnames[fn];
	new = names[fn];
	mesg (TELALL + 4, ediag("RENAME: ","Переименование: "),
old, ediag(" to "," в "), new);
	if (!mv (old, new)) {
	    mesg (ERRALL + 4, ediag(
"Can't rename ","Нельзя переименовать "), old, ediag(" to "," в "), new);
	    return NO;
	}
    }
    fileflags[fn] &= ~RENAMED;

    Block {
	Reg2 Fn tmp;

	if ((tmp = hvdelname (names[fn])) != -1)
	    fileflags[tmp] &= ~(INUSE | DELETED);
    }

    return YES;
}
