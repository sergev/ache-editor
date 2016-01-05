/* input control character assignments */

#define CCCMD           000 /*enter parameter       */
#define CCLWINDOW       001 /*window left           */
#define CCMIFILE        002 /* назад по списку файлов */
#define CCINT           003 /*interrupt             */
#define CCOPEN          004 /*insert                */
#define CCMISRCH        005 /*minus search          */
#define CCCLOSE         006 /*delete                */
#define CCMARK          007 /*mark                  */
#define CCMOVELEFT      010 /*backspace             */
#define CCTAB           011 /*tab                   */
#define CCMOVEDOWN      012 /*move down 1 line      */
#define CCHOME          013 /*home cursor           */
#define CCPICK          014 /*pick                  */
#define CCRETURN        015 /*return                */
#define CCMOVEUP        016 /*move up 1 lin         */
#define CC1COLUMN       017 /*go to first column    */
#define CCREPLACE       020 /*replace               */
#define CCMIPAGE        021 /*minus a page          */
#define CCPLSRCH        022 /*plus search           */
#define CCRWINDOW       023 /*window right          */
#define CCPLLINE        024 /*minus a line          */
#define CCDELCH         025 /*character delete      */
#define CCRWORD         026 /*move right one word   */
#define CCMILINE        027 /*plus a line           */
#define CCLWORD         030 /*move left one word    */
#define CCPLPAGE        031 /*plus a page           */
#define CCCHWINDOW      032 /*change window         */
#define CCTABS          033 /*set tabs              */
#define CCCTRLQUOTE     034 /*knockdown next char   */
#define CCBACKTAB       035 /*tab left              */
#define CCBACKSPACE     036 /*backspace and erase   */
#define CCMOVERIGHT     037 /*forward move          */
#define CCDEL          0177 /* <del> */
#define CCSTOP         0200 /*  stop replay           */
#define CCERASE        0201 /*  erase                 */
#define CCUNAS1        0202 /*  -- not assigned --    */
#define CCSPLIT        0203 /*  split                 */
#define CCJOIN         0204 /*  join                  */
#define CCGOTO         0205 /* goto                     */
#define CCBEGS         0206 /* beg of str               */
#define CCENDS         0207 /* end of str               */
#define CCREDRAW       0210 /* redraw                   */
#define CCINSCH        0211 /* insert char              */
#define CCMKWIN        0212 /* make window              */
#define CCMEXEC        0213 /* macro execution          */
#define CCMEND         0214 /* end of macro             */
#define CCROLLUP       0215 /* rolling up               */
#define CCROLLDOWN     0216 /* rolling down             */
#define CCINSMODE      0217 /* insert mode              */
#define CCCLREOL       0220 /* clear end of line        */
#define CCRUS          0221 /* cyrill input             */
#define CCLAT          0222 /* latin input              */
#define CCTAG          0223 /* tag <ident>              */
#define CCPLFILE       0224 /* вперед по списку файлов   */
#define CCABANDON      0225 /* удалить текущий файл из списка */
#define CCALT          0226 /* RUS/LAT case switch */

#define MAXMOTIONS 32
extern
ASmall cntlmotions[MAXMOTIONS];
#define UP 1    /* Up           */
#define DN 2    /* Down         */
#define RN 3    /* Return       */
#define HO 4    /* Home         */
#define RW 0    /* Word right   */
#define LW 0    /* Word left    */
#define RT 7    /* Right        */
#define LT 8    /* Left         */
#define TB 9    /* Tab          */
#define BT 10   /* Backtab      */
#define C1 11   /* First column */
