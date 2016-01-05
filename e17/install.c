#include "e.h"
#include <sys/stat.h>
#ifdef  M_SYSV
#include <sys/utsname.h>
#include <ustat.h>
#endif

struct halfstat {   /* see stat(2) */
	dev_t   ht_dev;         /* id of device containing directory entry */
	ino_t   ht_ino;         /* inode number */
	off_t   ht_size;        /* file size in bytes */
	time_t  ht_mtime;       /* time of last data modification */
} half;

static char *file;
static dev_t dev;

struct {
	char v_corr_key[16];
	int v_sys_value;
	int v_le_value;
	int v_fs_value;
} v = {
	"M\0a\1G\2i\3C",
};
#define corr_key v.v_corr_key
#define bad_sys v.v_sys_value
#define bad_le v.v_le_value
#define bad_fs v.v_fs_value

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
stat_check ()
{
    struct stat stbuf;

    if (stat (file, &stbuf) < 0) {
	perror (file);
	return;
    }
    half.ht_dev = stbuf.st_dev;
    half.ht_ino = stbuf.st_ino;
    half.ht_size = stbuf.st_size;
    half.ht_mtime = stbuf.st_mtime & ~0x1F;
    check_sum (&half, sizeof (half));
}

static
sys_check ()
{
#ifdef  M_SYSV
    struct utsname utsbuf;

    if (uname (&utsbuf) < 0) {
	perror ("uname");
	return;
    }
    check_sum (&utsbuf, sizeof (utsbuf));
#else   /*-M_SYSV*/
    return;
#endif  /*-M_SYSV*/
}

static
fs_check ()
{
#ifdef M_SYSV
    struct ustat ubuf;

    if (ustat (dev, &ubuf) < 0) {
	perror ("ustat");
	bad_sum = -1;
	return;
    }
    ubuf.f_tfree = 0;
    ubuf.f_tinode = 0;
    check_sum (&ubuf, sizeof (ubuf));
#else
    return;
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

ChkPnt3 ()
{
    fs_check ();
    bad_fs = bad_sum;
}

ChkPnt1 ()
{
    static char le[] =
    {'/' ^ 0xFF, 'u' ^ 0xFF, 's' ^ 0xFF, 'r' ^ 0xFF,
     '/' ^ 0xFF, 'l' ^ 0xFF, 'o' ^ 0xFF, 'c' ^ 0xFF, 'a' ^ 0xFF, 'l' ^ 0xFF,
     '/' ^ 0xFF, 'b' ^ 0xFF, 'i' ^ 0xFF, 'n' ^ 0xFF,
     '/' ^ 0xFF, 'l' ^ 0xFF, 'e' ^ 0xFF, '\0' ^ 0xFF};
    static Flag done = NO;

    file = le;
    if (!done) {
	UnCode (le);
	done = YES;
    }
    stat_check ();
    bad_le = bad_sum;
    dev = half.ht_dev;
}

ChkPnt2 ()
{
    sys_check ();
    bad_sys = bad_sum;
}


main ()
{
    FILE *le;
    int c,state;

    ChkPnt1();
    ChkPnt2();
    ChkPnt3();
    if ((le = fopen ("le", "r+")) == NULL) {
	perror("le");
	return;
    }
    state = 0;
    while ((c = getc (le)) != EOF) {
	if (c == corr_key[state])
		state++;
	else
		state = 0;
	if (state == sizeof(corr_key)) {
	    if (fseek (le, 0L, 1)) {
		perror("fseek");
		return;
	    }
	    if (fwrite ((char *)&bad_sys, sizeof(bad_sys), 1, le) != 1) {
		perror("fwrite 1");
		return;
	    }
	    if (fwrite ((char *)&bad_le, sizeof(bad_le), 1, le) != 1) {
		perror("fwrite 2");
		return;
	    }
	    if (fwrite ((char *)&bad_fs, sizeof(bad_fs), 1, le) != 1) {
		perror("fwrite 3");
		return;
	    }
	    fclose(le);
	    exit(0);
	}
    }
    perror("key");
    exit(1);
}
