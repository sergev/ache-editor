#include "e.h"
#include "e.m.h"
#include "e.tt.h"
#include "e.help.h";
#include SIG_INCL
#ifdef UNIXV7
#include <ctype.h>
#endif

extern char *index ();
void getline ();
static Flag chkcline ();
Flag putline ();
void excline ();
void exdeline ();
/*extern void elasdump ();*/
#ifdef  CHANGECTYPE
extern Flag correctype;
#endif
static Echar checkesc();

/*
    Gets line ln of the current workspace into cline.
    If (ln < 0) then flush cline if modified
      and set clinelas to 0 so that the next call to getline is
      guaranteed to get a fresh copy of the line desired from the file.
*/
void
getline (ln)
Nlines  ln;
{
    Reg5 Echar *clp;     /* cline pointer */
    Reg6 Echar *endcl;
    La_stream *rlas;
    Flag SeeNext;
    Short LastControl;
#define L_C_NONE 0
#define L_C_BOLD 1
#define L_C_ITAL 2
#define L_C_CTRL (L_C_BOLD | L_C_ITAL)
    Uchar SavCh;
    Short GetHelpItem;
    Flag NoBin;

#ifdef DEBUGDEF
    dbgpr ("*** old %d, new %d, clinelas %x, curfile %d\n",
	   clineno, ln, clinelas, curfile);
#endif
    if (ln < 0) {
	if (fcline)
	    putline (NO);
	clinelas = (La_stream *) NULL;
	return;
    }
    if (ln >= la_lsize (curlas)) {
#ifdef DEBUGDEF
	dbgpr ("Empty file, curlas %x\n", curlas);
#endif
	if (fcline)
	    putline (NO);
	xcline = YES;         /* past end of file */
	/* make up a blank line */
	clinelas = curlas;
	clineno = ln;
	ecline = NO;
	cline8 = NO;
	fcline = NO;
	ncline = 1;
	cline[0] = '\n';
	return;
    }

    if ( clinelas &&  clinelas->la_file == curlas->la_file
	&& ln == clineno
       ) {
#ifdef DEBUGDEF
	dbgpr ("  Have it\n");
#endif
	return;
    }

    if (fcline)
	putline (NO);

    clinefn = curfile;
    clinelas = rlas = curlas;
#ifdef DEBUGDEF
    dbgpr ("About to seek to line %d\n", ln);
#endif
    (void) la_lseek (rlas, (La_linepos) ln, 0);

    ecline = NO;
    cline8 = NO;
    fcline = NO;
    ncline = 0;
    xcline = NO;

    clineno = ln;

    clp = cline;
    endcl = &cline[lcline - (TABCOL + 1)];

    SeeNext = NO;
    LastControl = L_C_CTRL;  /* Not convert first BS */
    GetHelpItem = 0;
    NoBin = !binary || HelpActive;

    for (;;) Block {
	Reg4 char *cp;
	Reg3 Small nleft;
	char *bufp;

     /* elasdump (rlas, "before getlin");       /**/
	if ((nleft = la_lpnt (rlas, &bufp)) <= 0)
	    fatal (FATALIO, "getline read la_error #%d %d",
		   la_error (), nleft);
	cp = bufp;
	while (--nleft >= 0) Block {
	    int ch;
	    Flag PrintAble;
	    Short ItemNum;

	    if (clp > endcl) Block {
		Reg1 Ncols i1;

		ncline = (i1 = clp - cline) + 1;
		excline ((Ncols) 8);          /* 8 is max needed (for tab) */
		clp = &cline[i1];
		endcl = &cline[lcline - 9];
	    }
	    ch = U(*cp++);

	    PrintAble = isprint (ch)
#ifndef  CTYPE
			||
#ifdef  CHANGECTYPE
			!correctype &&
#endif
			isrus8 (ch)
#endif
	    ;

	    if (GetHelpItem > 0) Block {
		char *ItemName;
		Short ItemLen;

		if (!isdigit (ch)) Block {
		    static char ItemErr0[] = "***NoItemNumber***";

		    GetHelpItem = 0;
		    ItemName = ItemErr0;
		    ItemLen = sizeof ItemErr0 - 1;
		    ItemNum = -1;
		    goto FoundItem;
		}
		else {
		    ItemNum = ItemNum * 10 + (ch - '0');
		    if (--GetHelpItem == 0) Block {
			Short i;

			if (ItemNum < 1 || ItemNum >= HelpItems + 1) Block {
			    static char ItemErr1[] = "***BadItemNumber***";

			    ItemNum = -1;
			    ItemName = ItemErr1;
			    ItemLen = sizeof ItemErr1 - 1;
			}
			else {
			    ItemNum--;
			    ItemName = Item[ItemNum].item_name;
			    ItemLen = Item[ItemNum].item_len;
			    AddHelpEntry (ItemNum, ln, (Ncols) (clp - cline));
			}
		FoundItem:
			if (clp + ItemLen > endcl) Block {
			    Reg1 Ncols i1;

			    ncline = (i1 = clp - cline) + 1;
			    excline ((Ncols) ItemLen);
			    clp = &cline[i1];
			    endcl = &cline[lcline - 9];
			}
			for (i = 0; i < ItemLen; i++)
			    *clp++ = U(ItemName[i]) | IA_MD;
		    }
		    if (ch != '\n')
			continue;
		}
	    }

	    if (SeeNext) {
		/* Bold */
		if (    PrintAble
		     && ch == U(SavCh) /* isspace already looked */
		     && !(LastControl & L_C_BOLD)) {
		    SeeNext = NO;
		    clp[-1] |= IA_MD;
		    LastControl |= L_C_BOLD;
		    continue;
		}
		/* Italic */
		else if (   PrintAble
			 && SavCh == '_'
			 && !(LastControl & L_C_ITAL)
			 && !isspace (ch))
		    clp--;
		else { /* Insert old BS */
		    SeeNext = NO;
		    LastControl = L_C_CTRL;
		    ecline = YES;   /* line has control char        */
		    *clp++ = ESCCHAR;
		    SavCh = '\b' | 0100;
		    if (uppercaseflg && isupper (SavCh))
			SavCh = tolower (SavCh);
		    *clp++ = U(SavCh);
		}
	    }

	    if (PrintAble) {
		Reg1 char chr;

		if (uppercaseflg && ch == BEFORECH)
		    *clp++ = BEFORECH;
		else if (uppercaseflg
			 && (chr = changechar (ch, YES)) != '\0') {
		    *clp++ = BEFORECH;
		    ch = chr;
		}
		else if (!cyrillflg) {
		    if (ch == CYRILLCH)
			*clp++ = CYRILLCH;
		    else if (isrus8 (ch)) {
			*clp++ = CYRILLCH;
			ch = tomap8 (ch);
		    }
		}
		*clp++ = ch;
		if (SeeNext) {
		    /* Italic */
		    SeeNext = NO;
		    clp[-1] |= IA_AS;
		    LastControl |= L_C_ITAL;
		}
		else
		    LastControl = L_C_NONE;
	    }
	    else if (NoBin && ch == '\b') {
		SavCh = clp[-1];
		if (    LastControl == L_C_CTRL
		     || isspace (SavCh))
		    goto ControlChar;
		SeeNext = YES;
	    }
	    else if (NoBin && ch == '\t') Block {
		Reg1 Ncols ri;

		LastControl = L_C_NONE;
		for (ri = 8 - (07 & (clp - cline)); ri--; )
		    *clp++ = ' ';
	    }
	    else if (ch == '\n') {
		if (NoBin) {
		    while (--clp >= cline && *clp == ' ')
			continue;
		    *++clp = '\n';
		}
		else {
		    ecline = YES;
		    *clp++ = ESCCHAR;
		    *clp = '\n';
		}
		ncline = clp - cline + 1;
		return;
	    }
	    else if (HelpActive && ch == '\0' && !SeeNext) {
		ItemNum = 0;
		GetHelpItem = HELPIDLEN;
	    }
	    else {
    ControlChar:
		LastControl = L_C_CTRL;
		ecline = YES;   /* line has control char        */
		*clp++ = ESCCHAR;
		if (ch == ESCCHAR)
		    *clp++ = ch;
		else if (ch < ESCCHAR) {
		    ch |= 0100;
		    if (uppercaseflg) {
			if (isupper (ch))
			    ch = tolower (ch);
			else if (ch == '\\')
			    goto Num;
		    }
		    *clp++ = ch;
		}
		else {
		    char buf[10], *s;
	    Num:
		    (void) sprintf (buf, "%3o", ch);
		    for (s = buf; *s; s++)
			*clp++ = *s;
		}
	    }
	}
    }
    /* NOTREACHED */
}

/*
    If fcline != 0, inserts the line in cline in place of the current one.
    Returns YES is successful, NO if not.
    In future, several clines may be cached, and if allflg == YES, putline
    will flush them all out.
*/
Flag
putline (allflg)
Flag allflg;
{
    La_stream tlas;
    Nlines nlines;
    int nr;

/*  fatal (FATALBUG, "putline called"); */
#ifdef DEBUGDEF
    dbgpr ("Doing putline of line %d\n", clineno);
#endif
#ifdef lint
    if (allflg)
	allflg = allflg;
#endif

    if (!fcline)
	return YES;
#ifdef DEBUGDEF
    dbgpr ("  fcline is YES\n");
#endif
    cline[ncline - 1] = '\n';
    for (nlines = 0; chkcline (); nlines++)
	continue;
    fcline = NO;
    if (ncline < 1)
	return YES;
    nr = dechars ();
    if (!la_clone (clinelas, &tlas))
	return NO;
#ifdef DEBUGDEF
    dbgpr ("  clone ok\n");
#endif
    (void) la_lseek (&tlas, clineno, 0);
    clinelas = (La_stream *) NULL; /* force a getline next time */
    Block {
	La_linepos ln;
	La_linepos nins;

	ln = 1;
	nins = la_lreplace (&tlas, deline, nr, &ln,
			    la_lwsize (&tlas) > 1
			    ? &fnlas[OLDLFILE]
			    : (La_stream *) NULL);
	(void) la_close (&tlas);
#ifdef DEBUGDEF
	dbgpr ("  Done with putline\n");
#endif
	if (la_lsize (&tlas) < clineno)
	    fatal (FATALBUG, "lsize (%d) < clineno (%d)",
		   la_lsize (&tlas), clineno);
	if (nlines > 0)
	    redisplay (clinefn, clineno - nlines, (Nlines) 1, nlines, YES);
	if (nins < 1)
	    return NO;
	return YES;
    }
}

/*
    Check cline for a ^J before the end of the line.  If there is one,
    truncate the line and redisplay it.
*/
static
Flag
chkcline ()
{
    Reg2 Echar  *fm;
    Echar *newfm;

    if (!ecline)
	return NO;

    fm = cline;
    for (;;) Block {
	int cc;   /* current character */

	newfm = fm;
	if (U(cc = *fm) == ESCCHAR)
	    cc = checkesc (&newfm);
	else
	    newfm++;
	if (cc == '\n') {
	    *fm++ = '\n';
	    break;
	}
	else
	    fm = newfm;
    }

    if (fm - cline < ncline) Block {
	Reg3 Ncols  nsave;
	Reg4 Nlines line;

	line = clineno;
	nsave = ncline;
	ncline = fm - cline;
	fcline = YES;
	putline (NO);

	ncline = nsave - (newfm - cline);
	if (ncline < 1) {
	    clinelas = (La_stream *) NULL;
	    return NO;
	}

	if (line == la_lsize (curlas) - 1) {
	    if (!extend (1)) {
		mesg (ERRALL + 1, ediag("Can't extend the file",
					"Нельзя расширить файл"));
		return NO;
	    }
	}
	else {
	    (void) la_lseek (curlas, line + 1, 0);
	    if (la_blank (curlas, 1) != 1) {
		mesg (ERRALL + 1, ediag("Can't make file that long",
					"Нельзя сделать такой длинный файл"));
		return NO;
	    }
	}

	/* make the second line */
	move ((char *) newfm, (char *) cline,
	      (Uint) ncline * sizeof (Echar));
	fcline = YES;
	ecline = YES;
	clinelas = curlas;
	clineno = line + 1;
	clinefn = curfile;

	return YES;
    }

    return NO;
}



/*
    Performs in-place character conversion from internal
    to external format of the characters starting at line up to a newline or
    a ^J.  May alter contents of line.  Line MUST have a newline in it
    note: replaces initial spaces with tabs; deletes trailing spaces
    returns number of characters in the converted, external representation
*/
Ncols
dechars ()
{
    Reg3 Ncols   cn;          /* col number                 */
    Echar *fm;
    Reg2 char  *to;           /* pointers for move          */
    Reg1 Ncols  lnb;          /* 1 + last non-blank col     */
    Ncols len;
    int topos;

    fm = cline;
    to = deline;
    cn = 0;
    lnb = 0;

    for (;;) Block {
	Echar cc;    /* current character          */
	Echar ChAttrs;

	if (U(cc = *fm) == ESCCHAR)
	    cc = checkesc (&fm);
	else
	    fm++;
	if (U(cc) == '\n' && !binary)
	    break;
	else if (U(cc) != ' ') {
	    if (!binary && lnb == 0)
		while (8 + (lnb & ~7) <= cn) {
		    *to++ = (lnb & 7) == 7 ? ' ' : '\t';
		    lnb &= ~7;
		    lnb += 8;
		}
	    topos = to - deline;
	    if ((len = (topos + 1) + (cn - lnb) + 6) >= ldeline) {
		exdeline (len);
		to = deline + topos;
	    }
	    while (++lnb <= cn)
		*to++ = ' ';
	    ChAttrs = cc;
	    cc = U(ChAttrs);
	    if (!cyrillflg && cc == CYRILLCH) {
		if (U(*fm) == CYRILLCH)
		    ChAttrs = *fm++;
		else {
		    if (uppercaseflg && U(*fm) == BEFORECH) {
			if (U(fm[1]) == BEFORECH) {
			    ChAttrs = fm[1];
			    cc = U(ChAttrs);
			    fm += 2;
			}
			else Block {
			    Reg3 Uchar chr;

			    if ((chr = changechar (fm[1], NO)) != '\0') {
				cc = U(chr);
				ChAttrs = fm[1];
				fm += 2;
			    }
			}
		    }
		    else if (ismap8 (U(*fm))) {
			ChAttrs = *fm++;
			cc = U(ChAttrs);
		    }

		    if (ismap8 (cc))  /* Only for KOI8 */
			cc = tomap8 (cc);
		}
	    }
	    else if (uppercaseflg && cc == BEFORECH) {
		if (U(*fm) == BEFORECH)
		    ChAttrs = *fm++;
		else Block {
		    Reg3 Uchar chr;

		    if ((chr = changechar (*fm, NO)) != '\0') {
			cc = U(chr);
			ChAttrs = *fm++;
		    }
		 }
	    }
	    if (ChAttrs & IA_AS) { /* Italic */
		*to++ = '_';
		*to++ = '\b';
	    }
	    *to++ = cc;
	    if (ChAttrs & IA_MD) { /* Bold */
		*to++ = '\b';
		*to++ = cc;
	    }
	    if (binary && to[-1] == '\n')
		return to - deline;
	}
	++cn;
    }
    *to++ = '\n';
    return to - deline;
}

Ncols
ch_dechars (line)
Reg1 char *line;
{
    Short len;
    Reg3 Ncols i;

    len = index (line, '\n') - line + 1;
    getline (-1);       /* Forget cline */
    if (len >= lcline)
	excline (len);
    for (i = 0; i < len; i++)
	cline[i] = U(line[i]);

    return dechars ();
}


/*    Expand cline to max (length, lcline + icline) */
void
excline (length)
Ncols length;
{
    Reg1 Ncols  j;

    if ((j = lcline + icline) < length)
	j = length;

    cline = (Echar *) gsalloc ((char *) cline,
			       (Uint) (j + 1) * sizeof (Echar), YES);
    icline += (j - lcline) / 2;
    lcline = j;
}


/*    Expand deline to max (length, lcline + icline) */
void
exdeline (length)
Ncols length;
{
    Reg1 Ncols  j;

    if ((j = ldeline + ideline) < length)
	j = length;

    deline = gsalloc (deline, (Uint) (j + 1), YES);

    ideline += (j - ldeline) / 2;
    ldeline = j;
}

/*
    Extend the file with ext blank lines so that
	la_lsize (curlas) = line
*/
Flag
extend (ext)
Reg1 Nlines ext;
{
    if (ext > 0) {
	(void) la_lseek (curlas, 0, 2);
	if (la_blank (curlas, ext) != ext)
	    return NO;
    }
    return YES;
}

/*
    Type: -2 and 2 - paragraphs backward and forward
    Type: -1 and 1 - lines backward and forward
    Return the actual number of lines.  End of file can cause
    the number to be less than num.
*/
Nlines
lincnt (stline, num, type)
Nlines  stline;
Nlines  num;
Flag    type;
{
    putline (YES);
    return la_lcount (curlas, stline, num, type);
}

static
Echar
checkesc (frame)
Echar **frame;
{
    Reg1 Echar cc;
    Reg2 Echar *fm = *frame;

    if (U(cc = *fm++) == ESCCHAR) {
	cc = U(*fm++);
	if (cc == ' ')
	    cc = '\n';
	if (cc == '\n' || cc == ESCCHAR)
		;
	else if (cc >= '0' && cc <= '7') Block {
	    Echar *s;
	    char *p, buf[10];
	    int i;   /* not register */

	    for (s = fm - 1, p = buf;
		s - fm < 2 && *s >= '0' && *s <= '7'; s++)
		*p++ = *s;
	    *p = '\0';
	    if (p - buf == 3
		&& sscanf (buf, "%3o", &i) == 1
		&& i <= 0377) {
		cc = i;
		fm = s;
	    }
	}
	else if (cc < ESCCHAR
		 && (isalpha (cc) || index ("@[\\]_^", cc)))
	    cc &= 037;
    }

    *frame = fm;

    return cc;
}
