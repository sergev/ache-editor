/*
/* file e.t.h - header file that is terminal dependent
/*
/**/

typedef Short Scols;            /* number of columns on the screen */
typedef short AScols;           /* number of columns on the screen */
typedef Char Slines;            /* number of lines on the screen */
typedef char ASlines;           /* number of lines on the screen */

#define MAXWIDTH ((Scols) 999)

/* margin characters and others */
#define FIRSTSPCL  127
#define ESCCHAR    127  /* escape character */
#define BULCHAR    128  /* bullet character */
#define FIRSTMCH   129
#define LMCH       129  /* left */
#define RMCH       130  /* right */
#define MLMCH      131  /* more left */
#define MRMCH      132  /* more right */
#define TMCH       133  /* top */
#define BMCH       134  /* bottom */
#define TLCMCH     135  /* top left corner */
#define TRCMCH     136  /* top right corner */
#define BLCMCH     137  /* bottom left corner */
#define BRCMCH     138  /* bottom right corner */
#define BTMCH      139  /* bottom tab */
#define ELMCH      140  /* empty left */
#define INVMCH     141  /* inactive */
#define INHMCH     142  /* inactive */

#define NSPCHR      16  /* number of special characters */
#define NMCH        14  /* number of margin characters */
