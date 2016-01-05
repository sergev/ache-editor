#include "e.h"
/*
 * ������������������ ��������� ����
 *      lcmp(c1, c2) = 0  ���� c1 == c2
 *                   > 0  -//- c1 > c2
 *                   < 0  -//- c1 < c2
 *      Ctou(c)         ���-8 ---> U-���
 *      Ctok(c)         U-��� ---> ���-8
 */
/*NOXXSTR*/

int lcmp(c1, c2) {return Ctou(c1) - Ctou(c2);}

static char _utok[] = {
	0341, 0342, 0367, 0347, 0344, 0345, 0366, 0372,
	0351, 0352, 0353, 0354, 0355, 0356, 0357, 0360,
	0362, 0363, 0364, 0365, 0346, 0350, 0343, 0376,
	0373, 0375, 0377, 0371, 0370, 0374, 0340, 0361,
	0301, 0302, 0327, 0307, 0304, 0305, 0326, 0332,
	0311, 0312, 0313, 0314, 0315, 0316, 0317, 0320,
	0322, 0323, 0324, 0325, 0306, 0310, 0303, 0336,
	0333, 0335, 0337, 0331, 0330, 0334, 0300, 0321
};
static char _ktou[] = {
	0376, 0340, 0341, 0366, 0344, 0345, 0364, 0343,
	0365, 0350, 0351, 0352, 0353, 0354, 0355, 0356,
	0357, 0377, 0360, 0361, 0362, 0363, 0346, 0342,
	0374, 0373, 0347, 0370, 0375, 0371, 0367, 0372,
	0336, 0300, 0301, 0326, 0304, 0305, 0324, 0303,
	0325, 0310, 0311, 0312, 0313, 0314, 0315, 0316,
	0317, 0337, 0320, 0321, 0322, 0323, 0306, 0302,
	0334, 0333, 0307, 0330, 0335, 0331, 0327, 0332
};

int Ctok(c) register c; {return U(((c&0300)==0300)?_utok[c&077]:c);}
int Ctou(c) register c; {return U(((c&0300)==0300)?_ktou[c&077]:c);}