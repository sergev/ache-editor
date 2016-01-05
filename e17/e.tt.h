/*
/* file e.tt.h - terminal types header file
/*
/**/
#include "e.h"
#define C(c) ((c) & 31)

extern
char *kname;            /* name of keyboard type */
extern
char *tname;            /* name of terminal type */
extern
Small kbdtype;          /* which kind of keyboard */
extern
Small termtype;         /* which kind of terminal */
extern
Short screensize;       /* term.tt_width * term.tt_height */
extern
Flag    fast;           /* running at a fast baud rate (>= 4800) */
extern
short   ospeed;         /* tty output baud rate */
extern
Flag    visualtabs;     /* Enable tabs on borders */

extern
Echar   attributes;     /* Current attr of the screen */
extern
Small   psgraph;

#define IA_NORMAL 0
#define IA_SO (1 << CHARNBITS)
#define IA_AS (2 << CHARNBITS)
#define IA_US (4 << CHARNBITS)
#define IA_MD (010 << CHARNBITS)
#define IA_MB (020 << CHARNBITS)
#define IA_MH (040 << CHARNBITS)
#define IA_MR (0100 << CHARNBITS)

extern
S_looktbl kbdnames[];   /* names of known keyboards */
extern
S_looktbl termnames[];  /* names of known terminals */
extern
Scols ocol;
extern
Slines olin;
extern
Scols icol;
extern
Slines ilin;

extern
Flag tt_lt2;            /* 2 lefts take fewer characters than an addr */
extern
Flag tt_lt3;            /* 3 lefts take fewer characters than an addr */
extern
Flag tt_rt2;            /* 2 rights take fewer characters than an addr */
extern
Flag tt_rt3;            /* 3 rights take fewer characters than an addr */

typedef struct kbd {
/*  extern */ unsigned  (*kb_inlex  ) ();
/*  extern */ int  (*kb_init   ) ();
/*  extern */ int  (*kb_end    ) ();
} S_kbd;

typedef struct term {
/*  extern */ int  (*tt_ini0   ) ();
/*  extern */ int  (*tt_ini1   ) ();
/*  extern */ int  (*tt_end    ) ();
/*  extern */ int  (*tt_left   ) ();
/*  extern */ int  (*tt_right  ) ();
/*  extern */ int  (*tt_dn     ) ();
/*  extern */ int  (*tt_up     ) ();
/*  extern */ int  (*tt_cret   ) ();
/*  extern */ int  (*tt_nl     ) ();
/*  extern */ int  (*tt_clear  ) ();
/*  extern */ int  (*tt_home   ) ();
/*  extern */ int  (*tt_bsp    ) ();
/*  extern */ int  (*tt_addr   ) ();
/*  extern */ int  (*tt_lad    ) ();
/*  extern */ int  (*tt_cad    ) ();
/*  extern */ int  (*tt_xlate  ) ();
/*  extern */ int  (*tt_insline) ();
/*  extern */ int  (*tt_delline) ();
/*  extern */ int  (*tt_inschar) ();
/*  extern */ int  (*tt_delchar) ();
/*  extern */ int  (*tt_clrel)   ();
/*  extern */ int  (*tt_clres)   ();
/*  extern */ int  (*tt_vscset ) ();
/*  extern */ int  (*tt_vscend ) ();
/*  extern */ int  (*tt_scrup  ) ();
/*  extern */ int  (*tt_scrdn  ) ();
/*  extern */ int  (*tt_deflwin) ();
/*  extern */ int  (*tt_erase  ) ();
/*  extern */ int  (*tt_ll     ) ();
/*  extern */ int  (*tt_mexit  ) ();
/*  extern */ int  (*tt_gexit  ) ();
/*  extern */ int  (*tt_so     ) ();
/*  extern */ int  (*tt_se     ) ();
/*  extern */ int  (*tt_mb     ) ();
/*  extern */ int  (*tt_md     ) ();
/*  extern */ int  (*tt_mh     ) ();
/*  extern */ int  (*tt_mr     ) ();
/*  extern */ int  (*tt_as     ) ();
/*  extern */ int  (*tt_ae     ) ();
/*  extern */ int  (*tt_us     ) ();
/*  extern */ int  (*tt_ue     ) ();
    AFlag            tt_da;
    AFlag            tt_db;
    ASmall           tt_nleft;
    ASmall           tt_nright;
    ASmall           tt_ndn;
    ASmall           tt_nup;
    ASmall           tt_nnl;
    ASmall           tt_nbsp;
    ASmall           tt_naddr;
    ASmall           tt_nlad;
    ASmall           tt_ncad;
    ASmall           tt_wl;
    ASmall           tt_cwr;
    ASmall           tt_pwr;
    ASmall           tt_axis;
    AFlag            tt_prtok;
    short            tt_width;
    ASmall           tt_height;
} S_term;

extern S_term term;
extern S_term *tterm[];
extern S_kbd  kbd;
extern S_kbd  *tkbd[];

/* output control characters for the "standard" windowing terminal */
/**/
#define VCCNUL 000 /* move the terminal cursor to where it belongs    */
#define VCCINI 001 /* initialize                                      */
#define VCCEND 002 /* restore terminal                                */
#define VCCICL 003 /* clear physical terminal screen                  */
#define VCCARG 004 /* arg or qualification key                        */
#define VCCAAD 005 /* absolute cursor address                         */
#define VCCBEL 007 /* bell                                            */
#define VCCHOM 014 /* cursor home within window                       */
#define VCCSO  016 /* start standout */
#define VCCSE  017 /* end standout */
#define VCCAS  020
#define VCCAE  021
#define VCCUS  022
#define VCCUE  023
#define VCCMB  024
#define VCCMBE 025
#define VCCMD  026
#define VCCMDE 027
#define VCCMH  030
#define VCCMHE 031
#define VCCMR  032
#define VCCMRE 033
#define VCCNRM 034 /* clear all attributes */
