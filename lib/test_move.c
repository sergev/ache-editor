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
    char *source;
    char *dest;
    unsigned int cnt;

    if (argc != 4){
	    printf ("%s: source dest count\n", argv[0]);
	    exit (1);
    }
    move (&alph[36], &test[BOUND-10-26], 26);
    move (alph, &test[BOUND-10], 62);

    printf ("%39.39s %39.39s\n", test, &test[BOUND-10]);
    source = &test[atoi (argv[1])];
    dest =   &test[atoi (argv[2])];
    cnt = atoi (argv[3]);
    printf ("%d\n", move (source, dest, cnt) - dest);
    printf ("%39.39s %39.39s\n", test, &test[BOUND-10]);
    return 0;
}
