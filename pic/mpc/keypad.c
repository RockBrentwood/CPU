
/*               K E Y P A D    S C A N
 * Sample typical application code for the MPC Code Development System

       The applications described herein is for illustrative
       purposes only.  Byte Craft Limited makes no representation
       or warranty that such applications will be suitable for the
       specified use without further testing or modification.


 * (c) Copyright 1993
 * Byte Craft Limited
 * Waterloo Ontario CANADA N2J 4E4.
 *
 * Scan the 3x4 keyboard and returns a code made of the column/row
 * in the a register.  The top nybble is the column, the bottom nybble
 * is the row.  If no keys were depressed, it returns 0xff.

                       �����������������������Ŀ
                       �  ���Ŀ  ���Ŀ  ���Ŀ  �     10K
          A.7 �����������Ĵ 1 ��Ĵ 2 ��Ĵ 3 �������İ����Ŀ
                       �  �����  �����  �����  �          �
                       �  ���Ŀ  ���Ŀ  ���Ŀ  �          �
          A.6 �����������Ĵ 4 ��Ĵ 5 ��Ĵ 6 �������İ����Ĵ
                       �  �����  �����  �����  �          �
                       �  ���Ŀ  ���Ŀ  ���Ŀ  �          �
          A.5 �����������Ĵ 7 ��Ĵ 8 ��Ĵ 9 �������İ����Ĵ
                       �  �����  �����  �����  �          �
                       �  ���Ŀ  ���Ŀ  ���Ŀ  �          �
          A.4 �����������Ĵ * ��Ĵ 0 ��Ĵ # �������İ����Ĵ
                       �  �����  �����  �����  �          �
          A.3 ����     �������������������������         ���
                            �      �      �               �
          A.2 ���������������      �      �
                                   �      �
          A.1 ����������������������      �
                                          �
          A.0 �����������������������������

 *
 * returns: The scan code of the key.
 *          The scan code is made up of the column in the top
 *             4 bits with the row in the bottom 4 bits.
 *
 */
#include "demo.h"

#pragma portrw porta  @ 0x00;   /* Port a data register */
#pragma portrw ddra   @ 0x04;	/* Data direction, Port A (all output) */

int       tmpa = 0x7f  ;    /* Temporary Location */
int       row  = 0x22  ;    /*  Keyboard row      */
int       column ;    /*  Keyboard column   */
char      index  ;    /* Direct access to the index (volitile)       */
registera wreg   ;    /* Direct access to the accumulator (volitile) */

int scan (void) {
	/* start with the third column */
	for (tmpa = 0x04; tmpa !=0; tmpa >>= 1)
         {                      /* check if key pressed, one col at a time */
          porta = tmpa;
          if (row = (porta & 0xf0)) /* If key is pressed, get row & column */
            {  column = porta & 7;
               for (index = 0; index < 255; index++) ;             /* delay */
	                         /* debounce by checking row with previous */
	       if ((porta & 0xf0) == row)
                 {  while (porta & 0xf0);        /* check for key released */
		    return column | row;	    /* and return the code */
		 }
               else return 0xff;           /*  not the same, return no key */

            }
	}
	return 0xff;
}

void main ()
  {
	ddra = 0x0f;           /* Port A bits 7..4 inputs bits 3..0 outputs  */
        while (1) wreg = scan();
  }
