typedef struct {
    short mlen;
    AFlag mused;
    char *mbase;
} S_macro;

typedef struct nest {
    struct nest *iprevinp;
    short icnt;
    short inum;
    ASmall imac;
} S_nest;

#define MACLEN 512

extern S_nest *mread;
extern S_macro *mtab[];
extern Small setmacro;
extern Short mnest;
extern Short mcount;
