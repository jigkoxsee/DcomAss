#define COM1BASE 0x3F8
#define COM2BASE 0X2F8
#define COM3BASE 0X3E8
#define TXDATA COM2BASE
#define LCR (COM2BASE+3) /*0x3F8 line control*/
#define LSR (COM2BASE+5) /*0x3FD line status */
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dcom.c"
#include "control.c"
#include "error.c"
#include "file.c"
#include "frame.c"
#include "time.c"


int main( void)
{
	//sframe sfr;
	//iframe ifr;
	char* sf=NULL;
	sf=sframe_new(0,1,0);
	printf("SF : %x ",sf);
	return 0;
}
