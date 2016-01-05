/*
/* file e.m.h - some specialized include stuff
/**/

extern char *cmdname;   /* the full name of the command given */
extern char *cmdopstr;  /* command operand string - points to the rest of
			/* paramv after the command */
extern char *opstr;     /* first word in cmdopstr from a call to getword () */
extern char *nxtop;     /* next word after opstr - use for further getwords */

/* commands: <arg> command <ret> */
#define CMD_MARK        0
#define CMDCHWINDOW     1
#define CMDEXIT         2
#define CMDLOGOUT       3
#define CMD_COMMAND     4
#define CMDCENTER       5
#define CMDMARK         6
#define CMDWINDOW       7
#define CMD_WINDOW      8
#define CMDRUN          9
#define CMDSAVE         10
#define CMDSTOP         11
#define CMDFILL         12
#define CMDJUST         13
#define CMDEDIT         14
#define CMDREDRAW       15
#define CMDGOTO         16
#define CMDSPLIT        17
#define CMD_JOIN        17
#define CMDJOIN         18
#define CMD_SPLIT       18
#define CMDPRINT        19
#define CMDCOMMAND      20
#define CMDPICK         21
#define CMDCLOSE        22
#define CMDERASE        23
#define CMDOPEN         24
#define CMDBOX          25
#define CMD_PICK        26
#define CMD_CLOSE       27
#define CMD_ERASE       28
#define CMDTAB          29
#define CMD_TAB         30
#define CMDTABS         31
#define CMD_TABS        32
#define CMDTABFILE      33
#define CMD_TABFILE     34
#define CMDFEED         35
#define CMDHELP         36
#define CMDDELETE       37
#define CMDCLEAR        38
#define CMDNAME         39
#define CMDSHELL        40
#define CMDCALL         41
#define CMDMACRO        42
#define CMD_MACRO       43
#define CMDENDM         44
#define CMDSEARCH       45
#define CMD_SEARCH      46
#define CMDREPLACE      47
#define CMD_REPLACE     48
#define CMDUPDATE       49
#define CMD_UPDATE      50
#define CMDCOVER        51
#define CMDOVERLAY      52
#define CMDUNDERLAY     53
#define CMDBLOT         54
#define CMD_BLOT        55
#define CMDINSERT       56
#define CMDUNDO         57
#ifdef DEBUGALLOC
#define CMDVERALLOC     58
#endif
#define CMDTRACK        59
#define CMD_TRACK       60
#define CMDRANGE        61
#define CMD_RANGE       62
#define CMDQRANGE       63
#define CMDDIFF         64
#define CMD_DIFF        65
#define CMDHORWIN       66
#define CMDVERWIN       67
#define CMDOFFSET       68
#define CMD_OFFSET      69
/*#define CMDBULLETS      70*/
/*#define CMD_BULLETS     71*/
#define CMDVISTABS      72
#define CMD_VISTABS     73
#define CMDINSMODE      74
#define CMD_INSMODE     75
#define CMDTAG          76
#define CMDINPLACE      77
#define CMD_INPLACE     78
#define CMDFONT         79
#define CMDBINARY       80
#define CMD_BINARY      81
#define CMDABANDON      82

#include "e.ml.h"

extern char   *copy ();

