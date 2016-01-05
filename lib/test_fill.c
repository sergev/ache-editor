#include <stdio.h>

#ifdef VAX
#define BOUND 65536
#else
#define BOUND 100
#endif

char alph[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
char test[BOUND+500] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

main (argc, argv)
int argc;
char *argv[];
{
    char fillchar;
    char *dest;
    unsigned int cnt;

    if (argc != 4){
	    printf ("%s: dest count fillchar\n", argv[0]);
	    exit (1);
    }
    move (&alph[36], &test[BOUND-10-26], 26);
    move (alph, &test[BOUND-10], 62);

    printf ("%39.39s %39.39s\n", test, &test[BOUND-10]);
    fillchar = argv[3][0];
    dest =   &test[atoi (argv[1])];
    cnt = atoi (argv[2]);
    printf ("%d\n", fill (dest, cnt, fillchar) - dest);
    printf ("%39.39s %39.39s\n", test, &test[BOUND-10]);
    return 0;
}
