#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.se.h"

extern char *index ();

extern Flag needputup;
static Flag usetags = NO;
static char tagfile[] = "tags";
static char Cmain[] = "main";
void srchtag ();

Flag
imbedtag (line, col)
Nlines line;
Ncols col;
{
    Reg1 char *cp;
    int len;

    if ((cp = pkarg (curwksp, line, col, &len, PK_IDENT)) == (char *) NULL)
	return NO;

    if (len <= 0) {
	sfree (cp);
	return NO;
    }

    cp[len] = '\n';
    len = ch_dechars (cp);
    sfree (cp);
    deline[len - 1] = '\0';
    if (deline[0] == '\0')
	return NO;

    srchtag (deline);

    return YES;
}

Cmdret
dotag ()
{
    if (opstr[0] == '\0')
	return CRNEEDARG;

    if (*nxtop)
	return CRTOOMANYARGS;

    srchtag (opstr);

    return CROK;
}

void
srchtag (tag)
char *tag;
{
    Fn newfn;
    Fn savefn;
    char *cp, *tcp;
    char *file = NULL;
    Short i, j, len;
    Small delta, range, eret;
    Flag mainf, NotSame;
    char buf[256];

    if (mainf = !strcmp (tag, Cmain))
	tcp = "M\\([^ ]\\{1,\\}\\)  *\\1\\.c";
    else
	tcp = tag;
    cp = append (tcp, " ");
    tcp = append ("^", cp);
    sfree (cp);

    rex = regtest (tcp);
    sfree (tcp);
    if (rex <= 0)
	goto SrRet;

    savefn = curfile;
    if (usetags) {
	if ((newfn = hvname (tagfile)) == -1)
	    usetags = NO;
	else if (doedit (newfn, (Nlines) -1, (Ncols) -1, YES) != CROK)
	    goto ErrRet;
    }
    if (!usetags
	    && !(usetags = (eret = editfile (tagfile, (Ncols) -1, (Nlines) -1,
					    0, NO, YES)) == 1)) {
	if (eret < 0) {
	    cp = tagfile;
	    goto NoFile;
	}
	else
	    goto ErrRet;
    }

    starthere = YES;
    range = (curwksp->wkflags & RANGESET);
    curwksp->wkflags &= ~RANGESET;
    i = regsrch ((Nlines) 0, (Ncols) 0, la_lsize (curlas) - 1, 1, YES);
    curwksp->wkflags |= range;
    starthere = NO;

    switch (i) {

    case FOUND_SRCH:
	getline (srchline);
	len = ncline - 1;
	while (cline[len - 1] == ' ')
	    len--;
	for (j = i = 0; i < len && cline[i] != ' '; i++, j++)
	    ;
	if (j == 0)
	    goto BadTag;
	while (i < len && cline[i] == ' ')
	    i++;
	for (j = 0; i < len && cline[i] != ' '; i++, j++)
	    buf[j] = cline[i];
	buf[j] = '\0';
	if (!buf[0])
	    goto BadTag;
	file = append (buf, "");
	while (i < len && cline[i] == ' ')
	    i++;
	delta = (cline[i] == '?') ? -1 : 1;
	for (j = 0; i < len; ) {
	    if (j > 0 && i < len - 1
		    && index ("\\[.^*$", cline[i])
		    && (j > 1 || cline[i] != '^')
		    && (i < len - 2 || cline[i] != '$'))
		buf[j++] = '\\';
	    switch (buf[j++] = cline[i++]) {
	    case ' ':
		buf[j++] = delta < 0 ? '*' : ' ';
		buf[j++] = delta < 0 ? ' ' : '*';
		while (i < len && cline[i] == ' ')
			i++;
		break;
	    case '^':
		if (j == 2 && i < len - 1
			&& !index (" #", cline[i])) {
		    buf[j++] = ' ';
		    buf[j++] = '*';
		}
		break;
	    }
	}
	if (j < 3 || !buf[0] || !buf[1]
	      || buf[0] != buf[j - 1])
	    goto BadTag;
	buf[--j] = '\0';
	cp = buf + 1;

	if (mainf) {
	    for (tcp = cp; *tcp; tcp++)
		if (*tcp == Cmain[0] && strncmp (tcp, Cmain, 4) == 0)
		    goto Dalee;
	    goto NotMain;
	}

    Dalee:
	if ((rex = regtest (cp)) < 0)
	    goto BadTag;

	if ((eret = editfile (file, (Ncols) -1, (Nlines) -1,
			      0, NO, YES)) != 1) {
	    if (eret < 0) {
		cp = file;
		goto NoFile;
	    }
	    else
		goto FrRet;
	}

	starthere = YES;
	range = (curwksp->wkflags & RANGESET);
	curwksp->wkflags &= ~RANGESET;
	i = regsrch (
		     delta < 0 ? la_lsize (curlas) - 1 : (Nlines) 0,
		     delta < 0 ? (Ncols) -1 : (Ncols) 0,
		     delta < 0 ? (Nlines) 0 : la_lsize (curlas) - 1,
		     delta, YES);
	curwksp->wkflags |= range;
	starthere = NO;

	switch (i) {

	case FOUND_SRCH:
	    if (doedit (savefn, (Nlines) -1, (Ncols) -1, YES) != CROK)
		goto FrRet;
	    NotSame = ((newfn = hvname (file)) == -1 || newfn != savefn);
	    if ((eret = editfile (file, (Ncols) -1, (Nlines) -1,
				  0, NO, NO)) != 1) {
		if (eret < 0) {
		    cp = file;
		    goto NoFile;
		}
		else
		    goto FrRet;
	    }
	    needputup = NotSame;
	    if (file) {
		sfree (file);
		file = NULL;
	    }
	    gotomvwin (srchline, srchcol);
	    if (searchkey)
		sfree (searchkey);
	    searchkey = append (tag, "");
	    if ((rex = regtest (searchkey)) < 0) {
		sfree (searchkey);
		searchkey = NULL;
	    }
	    return;

	case NOTFOUND_SRCH:
	    mesg (ERRALL + 4, ediag ("'", "Определение '"),
			      tag,
			      ediag ("' definition not found in ",
				    "' не найдено в "),
			      file);
	    goto FrRet;

	default:
	    aborted (i == BAD_SRCH);
	    goto FrRet;

	}
	break;

    case NOTFOUND_SRCH:
NotMain:
	mesg (ERRALL + 4, ediag ("Identifier '", "Идентификатор '"),
			tag,
			ediag ("' not found in ",
				"' не найден в "),
			tagfile);
	goto FrRet;

    default:
	aborted (i == BAD_SRCH);
	goto ErrRet;
    }

NoFile:
    mesg (ERRALL + 2, ediag ("Can't find ",
			     "Нет файла "), cp);
    goto FrRet;

BadTag:
    mesg (ERRALL + 2, ediag ("Bad format in ",
			     "Плохой формат "), tagfile);
FrRet:
    if (file)
	sfree (file);

ErrRet:
    (void) doedit (savefn, (Nlines) -1, (Ncols) -1, YES);

SrRet:
    if (searchkey && (rex = regtest (searchkey)) < 0) {
	sfree (searchkey);
	searchkey = NULL;
    }
}
