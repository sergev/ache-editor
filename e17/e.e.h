#define OPPICK       1
#define OPCLOSE      2
#define OPERASE      4
#define OPCOVER    010
#define OPOVERLAY  020
#define OPUNDERLAY 040
#define OPBLOT    0100
#define OP_BLOT   0200
#define OPINSERT  0400
#define OPOPEN   01000
#define OPBOX    02000

#define OPQTO       (OPCLOSE | OPPICK | OPERASE)
#define OPQFROM     (OPCOVER|OPOVERLAY|OPUNDERLAY|OPBLOT|OP_BLOT|OPINSERT)
#define OPLENGTHEN  (OPOPEN | OPINSERT)

#define QNONE   -1
#define QADJUST  0
#define QPICK    1
#define QCLOSE   2
#define QERASE   3
#define QRUN     4
#define QBOX     5
#define NQBUFS   5
extern
S_svbuf qbuf[];
extern
AFn qtmpfn[];
