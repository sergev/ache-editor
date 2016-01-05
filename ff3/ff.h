#include <stdio.h>

extern int errno;

#define FF_BSIZE   BUFSIZ       /* Size of a FF block */

#define NOFILE _NFILE           /* Number of system opens allowed */
#define NOFFFDS (NOFILE+NOFILE) /* # of active FF files */

/* file description structure.  one for each distinct file open */
typedef
struct ff_file {
    struct ff_buf *fb_qf;       /* Queue of blks associated with this file */
    char	fn_fd,		/* File Descriptor	*/
		fn_mode,	/* Open mode for file	*/
		fn_refs;	/* # of references	*/
#ifdef UNIXV7
    dev_t       fn_dev;         /* Device file is on    */
    ino_t       fn_ino;         /* Inode of file        */
#endif
#ifdef UNIXV6
    char        fn_minor,       /* Device file is on    */
		fn_major;
    short       fn_ino;         /* Inode of file        */
#endif
    long        fn_realblk;     /* Image of sys fil pos */
    long        fn_size;        /* Current file size    */
#ifdef  EUNICE
    char       *fn_memaddr;     /* address of in-core file */
#endif  /*EUNICE*/
} Ff_file;
extern Ff_file ff_files[];

/* stream structure */
typedef
struct ff_stream {
    char        f_mode,         /* Open mode for handle */
		f_count;	/* Reference count	*/
    Ff_file    *f_file; 	/* Fnode pointer	*/
    long	f_offset;	/* Current file position*/
} Ff_stream;			/*  or buffered amount	*/

/* f_mode bits */
#define F_READ	01		/* File opened for read */
#define F_WRITE 02		/* File opened for write*/
#define F_IOEOF 04              /* EOF found */
#define F_IOERR 010             /* I/O Error occurs */

#define ff_eof(f)       (((f)->f_mode & F_IOEOF) != 0)
#define ff_error(f)     (((f)->f_mode & F_IOERR) != 0)
#define ff_eoferr(f)    (((f)->f_mode & (F_IOEOF|F_IOERR)) != 0)
#define ff_clearerr(f)  ((void) ((f)->f_mode &= ~(F_IOERR|F_IOEOF)))

extern Ff_stream ff_streams[];

/* Buffer structure.  one for each buffer in cache */
typedef struct  ff_buf {
    struct ff_buf *fb_qf,       /* Q of blks associated */
	       *fb_qb,          /*  with this file      */
	       *fb_forw,        /* forw ptr */
	       *fb_back;        /* back ptr */
    Ff_file    *fb_file;	/* Fnode blk is q'd on	*/
    long        fb_bnum;        /* Block # of this blk  */
    short       fb_count,       /* Byte count of block  */
		fb_wflg;        /* Block modified flag  */
    char       *fb_buf;         /* Actual data buffer [FF_BSIZE]  */
} Ff_buf;

/* list of all buffers. there is only one of these */
typedef struct ff_rbuf {
    Ff_buf        *fb_qf,       /* not used */
		  *fb_qb,       /* not used */
		  *fb_forw,     /* first buf in chain */
		  *fb_back;     /* last buf in chain */
    short          fr_count;    /* total number of buffers */
} Ff_rbuf;
extern Ff_rbuf ff_flist;

typedef struct ff_st {
    int 	fs_seek,	/* Total seek sys calls */
		fs_read,	/*	 read  "    "	*/
		fs_write;	/*	 write "    "	*/
    int         fs_ffseek,      /* Total seek calls */
		fs_ffread,      /*       read   "   */
		fs_ffwrite;     /*       write  "   */
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
