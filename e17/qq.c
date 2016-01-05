# 1 "e.sg.c"
# 1 "e.h" 1


# 1 "../../include/./c_env.h" 1
 



 

































 


 


 

			 




 





 
# 3 "e.h" 2

# 1 "../../include/./localenv.h" 1

 

 
 
 
 









 













# 4 "e.h" 2


# 1 "/usr/include/sys/types.h" 1 3
 











































 
# 1 "/usr/include/machine/ansi.h" 1 3
 






































 
















 










 











 













# 46 "/usr/include/sys/types.h" 2 3

# 1 "/usr/include/machine/types.h" 1 3
 







































typedef struct _physadr {
	int r[1];
} *physadr;

typedef struct label_t {
	int val[6];
} label_t;


typedef	unsigned int	vm_offset_t;
typedef	long long	vm_ooffset_t;
typedef	unsigned int	vm_pindex_t;
typedef	unsigned int	vm_size_t;

 



typedef	__signed char		   int8_t;
typedef	unsigned char		 u_int8_t;
typedef	short			  int16_t;
typedef	unsigned short		u_int16_t;
typedef	int			  int32_t;
typedef	unsigned int		u_int32_t;
typedef	long long		  int64_t;
typedef	unsigned long long	u_int64_t;

typedef	int32_t			register_t;


# 47 "/usr/include/sys/types.h" 2 3



typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		 
typedef	unsigned int	uint;		 


typedef	u_int64_t	u_quad_t;	 
typedef	int64_t		quad_t;
typedef	quad_t *	qaddr_t;

typedef	char *		caddr_t;	 
typedef	int32_t		daddr_t;	 
typedef	u_int32_t	dev_t;		 
typedef u_int32_t	fixpt_t;	 
typedef	u_int32_t	gid_t;		 
typedef	u_int32_t	ino_t;		 
typedef	long		key_t;		 
typedef	u_int16_t	mode_t;		 
typedef	u_int16_t	nlink_t;	 
typedef	long long 	off_t;		 
typedef	int 	pid_t;		 
typedef	int32_t		segsz_t;	 
typedef	int32_t		swblk_t;	 
typedef	u_int32_t	uid_t;		 

typedef	quad_t		rlim_t; 	 






 





# 1 "/usr/include/sys/cdefs.h" 1 3
 

















































 























# 100 "/usr/include/sys/cdefs.h" 3


 
























































# 90 "/usr/include/sys/types.h" 2 3

 
off_t	 lseek  (int, off_t, int)  ;
 



 









# 1 "/usr/include/machine/endian.h" 1 3
 






































 





 
































# 92 "/usr/include/machine/endian.h" 3


 


# 108 "/usr/include/machine/endian.h" 3













# 107 "/usr/include/sys/types.h" 2 3



typedef	unsigned long 	clock_t;




typedef	unsigned int 	size_t;




typedef	int 	ssize_t;




typedef	long 	time_t;






 









typedef long	fd_mask;






typedef	struct fd_set {
	fd_mask	fds_bits[((( 256  ) + ((  (sizeof(fd_mask) * 8 )  ) - 1)) / (  (sizeof(fd_mask) * 8 )  )) ];
} fd_set;







# 173 "/usr/include/sys/types.h" 3




# 6 "e.h" 2



# 1 "../la1/la.h" 1
 



# 1 "../ff3/ff.h" 1
# 1 "/usr/include/stdio.h" 1 3
 

























































 











typedef off_t fpos_t;








 





 
struct __sbuf {
	unsigned char *_base;
	int	_size;
};

 

























typedef	struct __sFILE {
	unsigned char *_p;	 
	int	_r;		 
	int	_w;		 
	short	_flags;		 
	short	_file;		 
	struct	__sbuf _bf;	 
	int	_lbfsize;	 

	 
	void	*_cookie;	 
	int	(*_close)  (void *)  ;
	int	(*_read)   (void *, char *, int)  ;
	fpos_t	(*_seek)   (void *, fpos_t, int)  ;
	int	(*_write)  (void *, const char *, int)  ;

	 
	struct	__sbuf _ub;	 
	unsigned char *_up;	 
	int	_ur;		 

	 
	unsigned char _ubuf[3];	 
	unsigned char _nbuf[1];	 

	 
	struct	__sbuf _lb;	 

	 
	int	_blksize;	 
	fpos_t	_offset;	 
} FILE;

 
extern FILE __sF[];
 





	 











 















 




				 



 




















 


 
void	 clearerr  (FILE *)  ;
int	 fclose  (FILE *)  ;
int	 feof  (FILE *)  ;
int	 ferror  (FILE *)  ;
int	 fflush  (FILE *)  ;
int	 fgetc  (FILE *)  ;
int	 fgetpos  (FILE *, fpos_t *)  ;
char	*fgets  (char *, int, FILE *)  ;
FILE	*fopen  (const char *, const char *)  ;
int	 fprintf  (FILE *, const char *, ...)  ;
int	 fputc  (int, FILE *)  ;
int	 fputs  (const char *, FILE *)  ;
size_t	 fread  (void *, size_t, size_t, FILE *)  ;
FILE	*freopen  (const char *, const char *, FILE *)  ;
int	 fscanf  (FILE *, const char *, ...)  ;
int	 fseek  (FILE *, long, int)  ;
int	 fsetpos  (FILE *, const fpos_t *)  ;
long	 ftell  (FILE *)  ;
size_t	 fwrite  (const void *, size_t, size_t, FILE *)  ;
int	 getc  (FILE *)  ;
int	 getchar  (void)  ;
char	*gets  (char *)  ;

extern const  int sys_nerr;		 
extern const  char * const  sys_errlist[];

void	 perror  (const char *)  ;
int	 printf  (const char *, ...)  ;
int	 putc  (int, FILE *)  ;
int	 putchar  (int)  ;
int	 puts  (const char *)  ;
int	 remove  (const char *)  ;
int	 rename   (const char *, const char *)  ;
void	 rewind  (FILE *)  ;
int	 scanf  (const char *, ...)  ;
void	 setbuf  (FILE *, char *)  ;
int	 setvbuf  (FILE *, char *, int, size_t)  ;
int	 sprintf  (char *, const char *, ...)  ;
int	 sscanf  (const char *, const char *, ...)  ;
FILE	*tmpfile  (void)  ;
char	*tmpnam  (char *)  ;
int	 ungetc  (int, FILE *)  ;
int	 vfprintf  (FILE *, const char *, char * )  ;
int	 vprintf  (const char *, char * )  ;
int	 vsprintf  (char *, const char *, char * )  ;
 

 



 




 
char	*ctermid  (char *)  ;
FILE	*fdopen  (int, const char *)  ;
int	 fileno  (FILE *)  ;
 


 



 
int	 asprintf  (char **, const char *, ...)  ;
char	*fgetln  (FILE *, size_t *)  ;
int	 fpurge  (FILE *)  ;
int	 getw  (FILE *)  ;
int	 pclose  (FILE *)  ;
FILE	*popen  (const char *, const char *)  ;
int	 putw  (int, FILE *)  ;
void	 setbuffer  (FILE *, char *, int)  ;
int	 setlinebuf  (FILE *)  ;
char	*tempnam  (const char *, const char *)  ;
int	 snprintf  (char *, size_t, const char *, ...)  ;
int	 vasprintf  (char **, const char *, char * )  ;
int	 vsnprintf  (char *, size_t, const char *, char * )  ;
int	 vscanf  (const char *, char * )  ;
int	 vsscanf  (const char *, const char *, char * )  ;
 

 






 


 
FILE	*funopen  (const void *,
		int (*)(void *, char *, int),
		int (*)(void *, const char *, int),
		fpos_t (*)(void *, fpos_t, int),
		int (*)(void *))  ;
 




 


 
int	__srget  (FILE *)  ;
int	__svfscanf  (FILE *, const char *, char * )  ;
int	__swbuf  (int, FILE *)  ;
 

 





static __inline int __sputc(int _c, FILE *_p) {
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__swbuf(_c, _p));
}
# 360 "/usr/include/stdio.h" 3























# 1 "../ff3/ff.h" 2


extern int errno;









 
typedef
struct ff_file {
    struct ff_buf *fb_qf;        
    char	fn_fd,		 
		fn_mode,	 
		fn_refs;	 

    dev_t       fn_dev;          
    ino_t       fn_ino;          






    long        fn_realblk;      
    long        fn_size;         



} Ff_file;
extern Ff_file ff_files[];

 
typedef
struct ff_stream {
    char        f_mode,          
		f_count;	 
    Ff_file    *f_file; 	 
    long	f_offset;	 
} Ff_stream;			 

 










extern Ff_stream ff_streams[];

 
typedef struct  ff_buf {
    struct ff_buf *fb_qf,        
	       *fb_qb,           
	       *fb_forw,         
	       *fb_back;         
    Ff_file    *fb_file;	 
    long        fb_bnum;         
    short       fb_count,        
		fb_wflg;         
    char       *fb_buf;          
} Ff_buf;

 
typedef struct ff_rbuf {
    Ff_buf        *fb_qf,        
		  *fb_qb,        
		  *fb_forw,      
		  *fb_back;      
    short          fr_count;     
} Ff_rbuf;
extern Ff_rbuf ff_flist;

typedef struct ff_st {
    int 	fs_seek,	 
		fs_read,	 
		fs_write;	 
    int         fs_ffseek,       
		fs_ffread,       
		fs_ffwrite;      
} Ff_stats;
extern Ff_stats ff_stats;

extern void       ff_sort ();
extern int        ff_alloc ();
extern int        ff_close ();
extern int        ff_fd ();
extern int        ff_flush ();
extern int        ff_free ();
extern int        ff_getc ();
extern int        ff_putc ();
extern int        ff_read ();
extern int        ff_use ();
extern int        ff_write ();
extern int        ff_point ();
extern int        ff_sync ();
extern long       ff_size ();
extern long       ff_grow ();
extern long       ff_pos ();
extern long       ff_seek ();
extern Ff_buf    *ff_getblk ();
extern Ff_buf    *ff_gblk ();
extern Ff_buf    *ff_haveblk ();
extern Ff_buf    *ff_putblk ();
extern Ff_stream *ff_open ();
extern Ff_stream *ff_fdopen ();
extern Ff_stream *ff_gfil ();
# 5 "../la1/la.h" 2









 

 
 













  typedef short La_linesize;     







typedef short La_linepos;        


typedef long  La_bytepos;        
typedef int La_flag;

 


typedef struct lafsd
{
    struct lafsd *fsdback;   
    struct lafsd *fsdforw;   
    long        fsdpos;      
    struct lafile
	       *fsdfile;     
			     
    short       fsdnbytes;   
    char        fsdnlines;   
			     
    char        fsdbytes[1]; 
 } La_fsd;
				 
typedef struct lafile {
    La_fsd          *la_ffsd;    
    La_fsd          *la_lfsd;    
    Ff_stream       *la_ffs;     
    struct lastream *la_fstream; 
    struct lastream *la_lstream; 
    La_linepos       la_fsrefs;  
    short            la_refs;    
    La_linesize      la_maxline; 
    La_linepos       la_nlines;  

    La_bytepos       la_nbytes;  

    char             la_mode;    
 } La_file;
 




typedef struct lastream {
    short            la_fsline;  
    short            la_fsbyte;  
    short            la_lbyte;   
    short            la_ffpos;   
    La_bytepos       la_bpos;    
    La_linepos       la_lpos;    
    La_fsd          *la_cfsd;    
    La_linepos       la_rlines;  
    La_file         *la_file;    
    struct lastream *la_sback;   
    struct lastream *la_sforw;   
    struct lastream *la_fback;   
    struct lastream *la_fforw;   
    char             la_sflags;  
 } La_stream;
 


struct la_spos {                 
    short            la_fsline;
    short            la_fsbyte;
    short            la_lbyte;
    short            la_ffpos;
    La_bytepos       la_bpos;
    La_linepos       la_lpos;
    La_fsd          *la_cfsd;
 };
struct la_fpos {                 
    short            la_fsline;
    short            la_fsbyte;
    short            la_lbyte;
    short            la_ffpos;
 };




















 











extern La_linepos   la_align (), la_blank (), la_close (), la_finsert ();
extern La_linepos   la_lcollect (), la_lcopy (), la_lcount (), la_ldelete ();
extern La_linepos   la_lflush (), la_linsert (), la_lrcollect ();
extern La_linepos   la_lreplace (), la_lseek ();
extern La_linesize  la_advance (), la_lget (), la_lpnt ();
extern La_linesize  la_lrsize (), la_lwsize ();
extern La_stream   *la_clone (), *la_open (), *la_other ();
extern int          la_tcollect ();

 


extern void   la_abort();        
extern int    la_int();          
				 
				 
 
extern int         la_maxchans;  
extern char       *la_cfile;     
extern La_linesize la_maxline;   
				 

 
extern La_stream *la_chglas;     
extern Ff_stream *la_chgffs;     
extern La_flag    la_chgopen;    
extern int        la_chans;      
extern int        la_nbufs;      
extern La_stream *la_firststream; 
extern La_stream *la_laststream; 
extern int        la_errno;      
extern La_flag    la_colstate;
 




















# 9 "e.h" 2










 































# 1 "e.t.h" 1
 




typedef int  Scols;             
typedef short AScols;            
typedef int  Slines;             
typedef char ASlines;            



 





















# 51 "e.h" 2

 



typedef int   Flag;              
typedef char  AFlag;             
typedef int   Small;             
typedef char  ASmall;            




typedef La_linepos Nlines;       
typedef La_linepos ANlines;      





typedef int         Ncols;       
typedef La_linesize ANcols;      

typedef int   Fd;                
typedef char  AFd;               
typedef int   Fn;                
typedef char  AFn;               
typedef Small Cmdret;            
typedef unsigned short Echar;

# 101 "e.h"








# 1 "e.up.h" 1

extern short _ediag;








extern Flag uppercaseflg;
extern Flag isuppercase;


extern Flag cyrillflg;
extern Flag iscyr;
extern Flag rusbit;
extern Flag qwerty;
extern char qwerty_in[];

extern char cspchars[2][5  + 1];
extern char changechar ();
# 109 "e.h" 2
















 

 

















				 
				 









				 






 













 

typedef struct workspace {
    struct workspace *prev_wksp;           
    struct workspace *next_wksp;           
    La_stream las;               
    AFn      wfile;              
    ASlines clin;                
    AScols   ccol;               
    ANlines wlin;                
    ANcols  wcol;                
    ANlines rngline;             
    La_stream *brnglas;          
    La_stream *ernglas;          
    ASmall wkflags;
} S_wksp;
 



extern
S_wksp  *curwksp, *last_wksp, *first_wksp;


extern
La_stream *curlas;

extern Fd STDIN;
extern Fd STDOUT;
extern Fd STDERR;


 



















extern Fd nopens;
 





			 


			 




extern La_stream     fnlas[];        
extern char         *tmpnames[];     
extern char         *names[];        
extern char         *oldnames[];     
extern S_wksp        lastlook[];
extern short         fileflags[];






			 












extern
Flag    HelpActive;

extern
Fn      curfile;

 





typedef struct window
{
    S_wksp *wksp;                
    ASmall  prevwin;             
				 
				 
    ASlines ttext;               
    AScols   ltext;              
    AScols   rtext;              
    ASlines btext;               
    ASlines tmarg;               
    AScols   lmarg;              
    AScols   rmarg;
    ASlines bmarg;
    ASlines tedit;
    AScols   ledit;              
    AScols   redit;
    ASlines bedit;
    AScols  *firstcol;           
    AScols  *lastcol;            
    char   *lmchars;             
    char   *rmchars;             
    AFlag   winflgs;             
} S_window;
 





extern
S_window       *winlist[40 ],
	       *curwin,          
		wholescreen,     
		infowin,         
		enterwin;        
extern
Small   nwinlist;








 

typedef struct savebuf {
    La_stream buflas;
    ANcols  ncols;
} S_svbuf;


extern
Scols   cursorcol;               
extern
Slines  cursorline;              

extern Small chgborders;         
				 
				 

extern
unsigned numtyp;                 
				 


extern
char   *myname,
       *mypath,
       *progname;

extern
Flag    offsetflg;

extern
Flag    binary;

extern
Flag    inplace;                 

extern
Flag    smoothscroll;            
extern
Flag    singlescroll;            

extern ANcols *tabs;             
extern int    stabs;            
extern int    ntabs;            

 



			 


extern int  key;              
extern Flag keyused;          

 
extern Nlines defplline,         
	      defplpage,         
	      defmiline,         
	      defmipage;         
extern Ncols  deflwin,           
	      defrwin;           
extern char  deffile[];          
extern Fn    deffn;              

extern
int    linewidth,               
	tmplinewidth;            

extern
char   *paramv;                  
extern
Small   paramtype;

extern
Echar   *cline;                  
extern
int    lcline,                  
	ncline,                  
	icline;                  
extern
char    *deline;                 
extern
int    ldeline,                 
	ideline;                 
extern
Flag    fcline,                  
	cline8,                  
	ecline,                  
	xcline;                  
extern
Fn      clinefn;                 
extern
Nlines  clineno;                 
extern
La_stream *clinelas;             

extern char
	prebak[],                
	postbak[];               

extern
char *searchkey;


extern short



    userid,
    groupid;


extern
FILE        *keyfile;            

extern
FILE        *inputfile;          

extern
Flag    intok;                   
				 
extern
Small   intrupnum;               
extern
Flag    alarmed;

extern Flag windowsup;    

extern int  _sizebuf;
extern Echar *_putscbuf;
extern int  _putp;





extern
FILE      *dbgfile;

extern short revision;   
extern short subrev;     

extern
int  evrsn;    
		 

extern Flag notracks;    
extern Flag norecover,
	    replaying,
	    recovering,
	    silent;      
extern Flag keysmoved;   

 


extern
Flag cmdmode;
extern
Flag insmode;            
extern
Nlines parmlines;        
extern
Ncols parmcols;          

extern
char *shpath;

typedef struct lookuptbl
{   char *str;
    short val;
} S_looktbl;

extern
long strttime;   

extern
Flag loginflg;   

extern Flag ischild;     

extern int zero;

 

 
extern short getshort ();

 
extern long getlong ();

 
extern Cmdret command ();
extern Cmdret gotocmd ();
extern Cmdret doupdate ();

 
extern Cmdret areacmd ();
extern Cmdret splitlines ();
 
extern Flag multlinks ();
extern Flag fmultlinks ();
extern Fn hvname ();
extern Fn hvoldname ();
extern Fn hvdelname ();
 
extern Flag putline ();
extern Ncols dechars ();
extern Flag extend ();
extern Nlines lincnt ();
 
extern Flag gtumark ();
extern Small exchmark ();
 
extern Cmdret name ();
extern Cmdret delete ();
extern Flag dotdot ();
 
extern Small printchar ();
 
extern Small getpartype ();
extern char *getword ();
extern Cmdret scanopts ();
extern Cmdret getopteq ();
extern Cmdret doeq ();
 
extern Cmdret insert ();
extern Cmdret insbuf ();
 
extern Cmdret eexit ();
extern Flag saveall ();
extern Flag savestate ();
 
extern Cmdret print ();
extern Cmdret filter ();
extern Cmdret filtmark ();
extern Cmdret filtlines ();
extern Cmdret run ();
extern Cmdret runlines ();
extern Flag dowait ();
extern Flag receive ();
 
extern char *getmypath ();
extern char *gsalloc ();
extern char *salloc ();
extern char *okalloc ();
extern char *append ();
extern char *copy ();
extern char *s2i ();
extern char *itoa();
extern Flag mv ();
extern Flag okwrite ();
extern Small filecopy ();
extern int sig ();
 
extern Cmdret replace ();
extern Small dsplsearch ();
extern Small strsearch ();
extern Ncols skeylen ();
 
extern Cmdret save ();
extern Flag savefile ();
extern Flag svrename ();
 
extern Small vertmvwin ();
extern Small horzmvwin ();
extern Small movewin ();
extern unsigned int  getkey ();
extern Flag dintrup ();
extern Flag la_int();
extern Flag sintrup ();
 
extern Cmdret dotab ();
extern Cmdret dotabs ();
extern Small getptabs ();
extern Cmdret tabfile ();
extern Flag gettabs ();
 
extern Cmdret use ();
extern Small editfile ();
extern Fn getnxfn ();
 
extern S_window *setupwindow ();
 
extern Flag swfile ();
extern int  eddeffile ();
extern Nlines botmark ();
extern void infomacro ();
extern char *getenv ();
extern void getout ();
extern Cmdret doedit();

# 1 "e.sg.c" 2

# 1 "e.tt.h" 1
 






extern
char *kname;             
extern
char *tname;             
extern
Small kbdtype;           
extern
Small termtype;          
extern
int  screensize;        
extern
Flag    fast;            
extern
short   ospeed;          
extern
Flag    visualtabs;      

extern
Echar   attributes;      
extern
Small   psgraph;










extern
S_looktbl kbdnames[];    
extern
S_looktbl termnames[];   
extern
Scols ocol;
extern
Slines olin;
extern
Scols icol;
extern
Slines ilin;

extern
Flag tt_lt2;             
extern
Flag tt_lt3;             
extern
Flag tt_rt2;             
extern
Flag tt_rt3;             

typedef struct kbd {
  unsigned  (*kb_inlex  ) ();
  int  (*kb_init   ) ();
  int  (*kb_end    ) ();
} S_kbd;

typedef struct term {
  int  (*tt_ini0   ) ();
  int  (*tt_ini1   ) ();
  int  (*tt_end    ) ();
  int  (*tt_left   ) ();
  int  (*tt_right  ) ();
  int  (*tt_dn     ) ();
  int  (*tt_up     ) ();
  int  (*tt_cret   ) ();
  int  (*tt_nl     ) ();
  int  (*tt_clear  ) ();
  int  (*tt_home   ) ();
  int  (*tt_bsp    ) ();
  int  (*tt_addr   ) ();
  int  (*tt_lad    ) ();
  int  (*tt_cad    ) ();
  int  (*tt_xlate  ) ();
  int  (*tt_insline) ();
  int  (*tt_delline) ();
  int  (*tt_inschar) ();
  int  (*tt_delchar) ();
  int  (*tt_clrel)   ();
  int  (*tt_clres)   ();
  int  (*tt_vscset ) ();
  int  (*tt_vscend ) ();
  int  (*tt_scrup  ) ();
  int  (*tt_scrdn  ) ();
  int  (*tt_deflwin) ();
  int  (*tt_erase  ) ();
  int  (*tt_ll     ) ();
  int  (*tt_mexit  ) ();
  int  (*tt_gexit  ) ();
  int  (*tt_so     ) ();
  int  (*tt_se     ) ();
  int  (*tt_mb     ) ();
  int  (*tt_md     ) ();
  int  (*tt_mh     ) ();
  int  (*tt_mr     ) ();
  int  (*tt_as     ) ();
  int  (*tt_ae     ) ();
  int  (*tt_us     ) ();
  int  (*tt_ue     ) ();
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

 
 























# 2 "e.sg.c" 2

# 1 "e.sg.h" 1
 



# 1 "/usr/include/sgtty.h" 1 3
 





































# 1 "/usr/include/sys/ioctl.h" 1 3
 











































# 1 "/usr/include/sys/ttycom.h" 1 3
 











































# 1 "/usr/include/sys/ioccom.h" 1 3
 






































 





















 






 
int	ioctl  (int, unsigned long, ...)  ;
 




# 45 "/usr/include/sys/ttycom.h" 2 3


 




 



struct winsize {
	unsigned short	ws_row;		 
	unsigned short	ws_col;		 
	unsigned short	ws_xpixel;	 
	unsigned short	ws_ypixel;	 
};














						 


						 

						 






						 






						 














































# 45 "/usr/include/sys/ioctl.h" 2 3


 




struct ttysize {
	unsigned short	ts_lines;
	unsigned short	ts_cols;
	unsigned short	ts_xxx;
	unsigned short	ts_yyy;
};





# 1 "/usr/include/sys/filio.h" 1 3
 













































 









# 63 "/usr/include/sys/ioctl.h" 2 3

# 1 "/usr/include/sys/sockio.h" 1 3
 








































 










































# 64 "/usr/include/sys/ioctl.h" 2 3




 







# 1 "/usr/include/sys/ioctl_compat.h" 1 3
 











































# 1 "/usr/include/sys/ttychars.h" 1 3
 






































 





struct ttychars {
	char	tc_erase;	 
	char	tc_kill;	 
	char	tc_intrc;	 
	char	tc_quitc;	 
	char	tc_startc;	 
	char	tc_stopc;	 
	char	tc_eofc;	 
	char	tc_brkc;	 
	char	tc_suspc;	 
	char	tc_dsuspc;	 
	char	tc_rprntc;	 
	char	tc_flushc;	 
	char	tc_werasc;	 
	char	tc_lnextc;	 
};

# 1 "/usr/include/sys/ttydefaults.h" 1 3
 








































 





 








 





















 




 


 










# 63 "/usr/include/sys/ttychars.h" 2 3



# 45 "/usr/include/sys/ioctl_compat.h" 2 3

# 1 "/usr/include/sys/ttydev.h" 1 3
 



































 


























# 46 "/usr/include/sys/ioctl_compat.h" 2 3


struct tchars {
	char	t_intrc;	 
	char	t_quitc;	 
	char	t_startc;	 
	char	t_stopc;	 
	char	t_eofc;		 
	char	t_brkc;		 
};

struct ltchars {
	char	t_suspc;	 
	char	t_dsuspc;	 
	char	t_rprntc;	 
	char	t_flushc;	 
	char	t_werasc;	 
	char	t_lnextc;	 
};

 




struct sgttyb {
	char	sg_ispeed;		 
	char	sg_ospeed;		 
	char	sg_erase;		 
	char	sg_kill;		 
	short	sg_flags;		 
};



























































































# 76 "/usr/include/sys/ioctl.h" 2 3


# 39 "/usr/include/sgtty.h" 2 3

# 5 "e.sg.h" 2


# 1 "/usr/include/sys/ioctl.h" 1 3
 








































# 66 "/usr/include/sys/ioctl.h" 3


 









# 7 "e.sg.h" 2







 
 
 
 
    extern
    struct sgttyb outstty;
 
  extern struct sgttyb instty;









      extern
      struct tchars spchars;


      extern
      struct ltchars lspchars;




extern
Flag cbreakflg;

extern
Flag istyflg,
     ostyflg;





# 3 "e.sg.c" 2


# 1 "/usr/include/signal.h" 1 3
 






































# 1 "/usr/include/sys/signal.h" 1 3
 












































# 1 "/usr/include/machine/signal.h" 1 3
 






































 



typedef int sig_atomic_t;



# 1 "/usr/include/machine/trap.h" 1 3
 









































 


























 

 






 








 





 



# 48 "/usr/include/machine/signal.h" 2 3


 






struct	sigcontext {
	int	sc_onstack;		 
	int	sc_mask;		 
	int	sc_esp;			 
	int	sc_ebp;
	int	sc_isp;
	int	sc_eip;
	int	sc_efl;
	int	sc_es;
	int	sc_ds;
	int	sc_cs;
	int	sc_ss;
	int	sc_edi;
	int	sc_esi;
	int	sc_ebx;
	int	sc_edx;
	int	sc_ecx;
	int	sc_eax;




};




# 46 "/usr/include/sys/signal.h" 2 3



















































 















typedef void __sighandler_t  (int)  ;






typedef unsigned int sigset_t;

 


struct	sigaction {
	__sighandler_t *sa_handler;	 
	sigset_t sa_mask;		 
	int	sa_flags;		 
};











 







typedef	__sighandler_t	*sig_t;	 

 


struct	sigaltstack {
	char	*ss_sp;			 
	int	ss_size;		 
	int	ss_flags;		 
};





 



struct	sigvec {
	__sighandler_t *sv_handler;	 
	int	sv_mask;		 
	int	sv_flags;		 
};








 


struct	sigstack {
	char	*ss_sp;			 
	int	ss_onstack;		 
};

 










 



 
__sighandler_t *signal  (int, __sighandler_t *)  ;
 


# 40 "/usr/include/signal.h" 2 3




extern const  char * const  sys_signame[32 ];
extern const  char * const  sys_siglist[32 ];


 
int	raise  (int)  ;

int	kill  (int , int)  ;
int	sigaction  (int, const struct sigaction *, struct sigaction *)  ;
int	sigaddset  (sigset_t *, int)  ;
int	sigdelset  (sigset_t *, int)  ;
int	sigemptyset  (sigset_t *)  ;
int	sigfillset  (sigset_t *)  ;
int	sigismember  (const sigset_t *, int)  ;
int	sigpending  (sigset_t *)  ;
int	sigprocmask  (int, const sigset_t *, sigset_t *)  ;
int	sigsuspend  (const sigset_t *)  ;

int	killpg  (int , int)  ;
int	sigaltstack  (const struct sigaltstack *, struct sigaltstack *)  ; 
int	sigblock  (int)  ;
int	siginterrupt  (int, int)  ;
int	sigpause  (int)  ;
int	sigreturn  (struct sigcontext *)  ;
int	sigsetmask  (int)  ;
int	sigstack  (const struct sigstack *, struct sigstack *)  ;
int	sigvec  (int, struct sigvec *, struct sigvec *)  ;
void	psignal  (unsigned int, const char *)  ;


 


 








# 5 "e.sg.c" 2






Flag noctrlsflg = 0 ;
extern unsigned int  erase_char;   
extern unsigned int  start_char;
extern unsigned int  stop_char;



struct tchars tchars = {
    0xff,        
    0xff,        
    0xff,        
    0xff,        
    0xff,        
    0xff         
};







void
gettchars ()
{

    if (ioctl (STDIN, ( 0x40000000   | ((  sizeof( struct sgttyb )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  (  8 ) ))   , &instty) < 0) {
	iscyr = 0 ;
	_ediag = 1;
	cbreakflg = 0 ;
	return;
    }
    isuppercase = !!(instty.sg_flags & 0x00000004 );





# 63 "e.sg.c"





    if (ioctl (STDIN, ( 0x40000000   | ((  sizeof( struct tchars )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  ( 18 ) ))   , &spchars) < 0)
	cbreakflg = 0 ;
    else {
	tchars.t_startc = spchars.t_startc;
	tchars.t_stopc = spchars.t_stopc;
	start_char = (int)((unsigned char )( tchars.t_startc )) ;
	stop_char = (int)((unsigned char )( tchars.t_stopc )) ;
    }








}

 
void
setitty ()
{

    if (ioctl (STDIN, ( 0x40000000   | ((  sizeof( struct sgttyb )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  (  8 ) ))   , &instty) < 0)



    {
	cbreakflg = 0 ;
	return;
    }
    else
	cbreakflg = 1 ;




    if (cbreakflg) {
	if (noctrlsflg)
	    tchars.t_startc = tchars.t_stopc = 0xff;
	if (ioctl (STDIN, ( 0x80000000   | ((  sizeof( struct tchars )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  ( 17 ) ))   , &tchars) < 0)
	    cbreakflg = 0 ;
    }



    if (cbreakflg)   {
	static struct ltchars ltchars = {
	    0xff,        
	    0xff,        
	    0xff,        
	    0xff,        
	    0xff,        
	    0xff         
	};
	if (   ioctl (STDIN, ( 0x40000000   | ((  sizeof( struct ltchars )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  ( 116 ) ))   , &lspchars) < 0
	    || ioctl (STDIN, ( 0x80000000   | ((  sizeof( struct ltchars )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  ( 117 ) ))   , &ltchars) < 0
	   ) {
	    (void) ioctl (STDIN, ( 0x80000000   | ((  sizeof( struct tchars )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  ( 17 ) ))   , &spchars);
	    cbreakflg = 0 ;
	}
    }



      {
	register  int tmpflags;

	tmpflags = instty.sg_flags;
	erase_char = (int)((unsigned char )( instty.sg_erase )) ;

	if (cbreakflg)
	    instty.sg_flags = 0x00000002  | (instty.sg_flags & ~(0x00000008  | 0x00000010 ));
	else

	    instty.sg_flags = 0x00000020  | (instty.sg_flags & ~(0x00000008  | 0x00000010 ));
	if (ioctl (STDIN, ( 0x80000000   | ((  sizeof( struct sgttyb )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  (  9 ) ))   , &instty) == 0)
	    istyflg = 1 ;
	else
	    cbreakflg = 0 ;
	instty.sg_flags = tmpflags;
    }
# 180 "e.sg.c"



      {
	extern dostop();

	(void) signal (18 , dostop);
    }

}

 
void
setotty ()
{








    if (ioctl (STDOUT, ( 0x40000000   | ((  sizeof(  struct termios )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  (  19 ) ))   , &outstty) < 0)




	fast = 1 ;
    else   {
	register  int i;

	register  int j;






	i = outstty.sg_flags;
	outstty.sg_flags &= ~(0x00000010  | 0x00000c00 );





	if (ioctl (STDOUT, ( 0x80000000   | ((  sizeof( struct sgttyb )  & 0x1fff ) << 16) | (( 	( 't' ) ) << 8) | (  (  9 ) ))   , &outstty) == 0) {
	    ostyflg = 1 ;
	    outstty.sg_flags = i;
	}
# 238 "e.sg.c"

# 252 "e.sg.c"


# 264 "e.sg.c"


	fast = (ospeed = outstty.sg_ospeed) >= 12 ;



    }
    if (!fast)
	visualtabs = 0 ;
}
