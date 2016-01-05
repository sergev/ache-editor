/* these came from the top of e.m.c */
extern
struct loopflags {
    AFlag clear;        /* clear enterwin  */
    AFlag hold;         /* hold cmd line until next key input */
    AFlag beep;         /* beep on clearing cmd line */
    AFlag flash;        /* bullet at cursor position is to be temporarily */
 };
extern
struct loopflags loopflags;

/* Режимы для pkarg */
extern char *pkarg ();

#define PK_WHOLE 0
#define PK_IDENT 1
#define PK_UPTOS 2
#define PK_FIELD 3
