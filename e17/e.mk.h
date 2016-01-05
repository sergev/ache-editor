struct markenv
{
    Nlines  mrkwinlin;
    ANcols  mrkwincol;
    ASlines mrklin;
    Scols   mrkcol;
};
extern
struct markenv *curmark,
	       *prevmark;

extern
Nlines  marklines;      /* display of num of lines marked */
extern
Ncols   markcols;       /* display of num of columns marked */

extern
char    mklinstr [],   /* string of display of num of lines marked */
	mkcolstr [];   /* string of display of num of lines marked */
extern
Small   infoarealen;    /* len of string of marked area display */

extern
Nlines  cmarktop, omarktop;
extern
Nlines  cmarkbot, omarkbot;
extern
Ncols   cmarkleft, omarkleft;
extern
Ncols   cmarkright, omarkright;

/*    Return the top line of a marked area. */
#define topmark(line) (Nlines)min(curwksp->wlin+(line),curmark->mrkwinlin+curmark->mrklin)
/*    Return the bot line of a marked area. */
#define botmark(line) (Nlines)max(curwksp->wlin+(line),curmark->mrkwinlin+curmark->mrklin)
/*    Return the leftmost column of a marked area. */
#define leftmark(col) (Ncols)min(curwksp->wcol+(col),curmark->mrkwincol+curmark->mrkcol)
/*    Return the rightmost column of a marked area. */
#define rightmark(col) (Ncols)max(curwksp->wcol+(col),curmark->mrkwincol+curmark->mrkcol)
