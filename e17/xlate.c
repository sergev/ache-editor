#include "sterm.h"
#include "e.sg.h"

bad ()
{
    fatal (FATALBUG, ediag("Impossible cursor address",
			   "Неверная адресация курсора"));
}

nop () {}

/* Special characters */
char stdxlate[NSPCHR] = {
    '@',     /* BULCHAR bullet character */
    '@',     /* ESCCHAR escape character */
    '!',     /* LMCH    left border */
    '!',     /* RMCH    right border */
    '<',     /* MLMCH   more left border */
    '>',     /* MRMCH   more right border */
    '-',     /* TMCH    top border */
    '-',     /* BMCH    bottom border */
    '+',     /* TLCMCH  top left corner border */
    '+',     /* TRCMCH  top right corner border */
    '+',     /* BLCMCH  bottom left corner border */
    '+',     /* BRCMCH  bottom right corner border */
    '+',     /* BTMCH   bottom tab border */
    ';',     /* ELMCH   empty left border */
    ':',     /* INVMCH  inactive border */
    '.'      /* INHMCH  inactive border */
};

xlate (chr)
Uchar chr;
{
    chr = U(chr);
    P (stdxlate[chr - FIRSTSPCL]);
}

kini_nocbreak()
{
#if defined(M_SYSV) || defined(CBREAK)
    cbreakflg = NO;
#endif
}


