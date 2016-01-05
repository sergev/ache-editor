#include "e.h"
#include <sys/stat.h>
#ifdef  M_SYSV
#include <sys/utsname.h>
#include <ustat.h>
#endif

#ifdef  SECURITY
struct halfstat {   /* see stat(2) */
	dev_t   ht_dev;         /* id of device containing directory entry */
	ino_t   ht_ino;         /* inode number */
	off_t   ht_size;        /* file size in bytes */
	time_t  ht_mtime;       /* time of last data modification */
} half;

static char *file;
static dev_t dev;

Flag nomorecommand;
Flag nomoretime;
Flag nowritefile;
struct {
	char v_corr_key[16];
	int v_sys_value;
	int v_le_value;
	int v_fs_value;
} v = {
	"M\0a\1G\2i\3C",
};
#define corr_key v.v_corr_key
#define sys_value v.v_sys_value
#define le_value v.v_le_value
#define fs_value v.v_fs_value

static int val;

char *violation[2] = {
	    "Unauthorized program copy!",
	    "Нелегальная копия программы!"
};

static int bad_sys;
static int bad_le;
static int bad_fs;

static int bad_sum;

static
int
check_sum (mem, n)
Reg2 Uchar *mem;
{
    Reg3 int i;
    Reg2 int sum;
    Reg4 int sign;

    sum = 0;
    for (i = 0; i < n; i++) {
	sign = sum < 0;
	sum = ((sum << 1) ^ U(*mem++)) | sign;
    }
    return (bad_sum = sum);
}

static
Flag
stat_check ()
{
    struct stat stbuf;

    if (stat (file, &stbuf) < 0)
	return YES;
    half.ht_dev = stbuf.st_dev;
    half.ht_ino = stbuf.st_ino;
    half.ht_size = stbuf.st_size;
    half.ht_mtime = stbuf.st_mtime & ~0x1F;
    return (check_sum (&half, sizeof (half)) != val);
}

static
Flag
sys_check ()
{
#ifdef  M_SYSV
    struct utsname utsbuf;

    return (uname (&utsbuf) < 0
	    || check_sum (&utsbuf, sizeof (utsbuf)) != val
	   );
#else   /*-M_SYSV*/
    return NO;
#endif  /*-M_SYSV*/
}

static
Flag
fs_check ()
{
#if defined(M_SYSV) && defined(SPEC_FS_CHECK)
    struct ustat ubuf;

    if (ustat (dev, &ubuf) < 0) {
	bad_sum = -1;
	return YES;
    }
    ubuf.f_tfree = 0;
    ubuf.f_tinode = 0;
    return (check_sum (&ubuf, sizeof (ubuf)) != val);
#else
    return NO;
#endif
}

static
char *
UnCode (str)
Reg2 char *str;
{
    Reg1 char *s;

    for (s = str; ; s++) {
	*s ^= 0xFF;
	if (*s == '\0')
	    return str;
    }
}

Flag
ChkPnt3 ()
{
    Flag ret;

    val = fs_value;
    ret = fs_check ();
    bad_fs = bad_sum;
    return (ret || nomorecommand || nomoretime);
}

Flag
ChkPnt1 ()
{
    static char le[] =
    {'/' ^ 0xFF, 'u' ^ 0xFF, 's' ^ 0xFF, 'r' ^ 0xFF,
     '/' ^ 0xFF, 'l' ^ 0xFF, 'o' ^ 0xFF, 'c' ^ 0xFF, 'a' ^ 0xFF, 'l' ^ 0xFF,
     '/' ^ 0xFF, 'b' ^ 0xFF, 'i' ^ 0xFF, 'n' ^ 0xFF,
     '/' ^ 0xFF, 'l' ^ 0xFF, 'e' ^ 0xFF, '\0' ^ 0xFF};
    static Flag done = NO;
    Flag ret;

    file = le;
    val = le_value;
    if (!done) {
	UnCode (le);
	done = YES;
    }
    ret = stat_check ();
    bad_le = bad_sum;
    dev = half.ht_dev;
    return ret;
}

Flag
ChkPnt2 ()
{
    Flag ret;

    val = sys_value;
    ret = sys_check ();
    bad_sys = bad_sum;
    return ret;
}

int
sec_timeout ()
{
    fatal (FATALSEC, ediag (violation[0], violation[1]));
    /*NOTREACHED*/
}
#endif  /*SECURITY*/
