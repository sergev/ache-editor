/*
/* file e.sg.h: sgtty stuff
/**/
#ifndef M_SYSV
#include SGTT_INCL
#ifndef NOIOCTL_H
#include <sys/ioctl.h>
#endif  /*NOIOCTL_H*/
#else   /*M_SYSV*/
#include <termio.h>
#endif

#ifdef sun
#include <sys/ttold.h>
#endif

#ifndef M_SYSV
//# ifdef TIOCGETA                 /* IPK fix */
//    extern
//   struct sgttya outstty;
//# else /*TIOCGETA*/
    extern
    struct sgttyb outstty;
//# endif /*TIOCGETA*/
  extern struct sgttyb instty;
#else   /*M_SYSV*/
  extern
  struct termio in_termio,
		out_termio;
#endif  /*M_SYSV*/

#ifndef M_SYSV
# ifdef  CBREAK
#   ifdef  TIOCGETC
      extern
      struct tchars spchars;
#   endif  /*TIOCGETC*/
#   ifdef  TIOCGLTC
      extern
      struct ltchars lspchars;
#   endif  /*TIOCGLTC*/
# endif  /*CBREAK*/
#endif  /*M_SYSV*/

extern
Flag cbreakflg;

extern
Flag istyflg,
     ostyflg;

#ifdef MESG_NO
  extern
  unsigned Short oldttmode;
#endif /*MESG_NO*/
