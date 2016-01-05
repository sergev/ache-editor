#include <c_env.h>
#include <localenv.h>

#ifdef UNIXV7
#include <sys/types.h>
#endif

/* #define DEBUG        /* for bug squashing code */

#define FF_CHKF (ff < &ff_streams[0] || ff >= &ff_streams[NOFFFDS] || ff->f_file == 0)

#define CHKBLK  (!(fb = fp->fb_qf) || fb->fb_bnum != block || (ff_flist.fb_forw != fb && ff_flist.fb_forw->fb_forw != fb))
/* (   !(fb = fp->fb_qf)
/*  || fb->fb_bnum != block
/*  || (   ff_flist.fb_forw != fb
/*      && ff_flist.fb_forw->fb_forw != fb
/*     )
/* )
/**/
#ifndef LDIV
#define LDIV
#define ldiv(a,b,c) ((*(c) = (a) % (b)), ((long)(a)) / (b))
#else
extern long ldiv();
#endif /*LDIV*/

/* the following include must appear in the right order */
#include "ff.h"

#ifndef NULL
#define NULL ((char *)0)
#endif

#define Block
