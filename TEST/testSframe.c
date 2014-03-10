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

/*#include "dcom.c"
#include "control.c"
#include "error.c"
#include "file.c"
#include "frame.c"
#include "time.c"*/

//-------------- Variable Declaration ------------\\
char name[200],name2[200];

/*CONTROL	== 00  I-TEXT
			== 01  I-IMG
			== 10  S-TEXT
			== 11  S-IMG	*/
int sender_timer,startTime;
int frame_number_last,frame_number_current;
int ackR,ackS,control,dataSize;
char** data8bit;
int control_is_txt_img;

unsigned char* file_ptr;

int mode,R_No=0,S_No=0;
char send_data[1000],c;
unsigned char* received_frame;
//-------------- Variable Declaration ------------//

// ----------------SET UP PART ---------------
void setup_serial(void)
{
	outportb(LCR, 0x80);
	/*set up bit 7 to a 1 to set Register address bit*/
	outportb(TXDATA,0x0C);
	outportb(TXDATA+1,0X00);
	/*load TxRegister with 12, crystal frequency is 1.8432 MHz*/
	outportb(LCR, 0x0B);
	/* Bit pattern loads is 00001010b, from MSB to LSB these are: */
	/* 0 - access TD/RD buffer, 0 - normal output */
	/* 0 - no stick bit, 0 - even parity */
	/* 1 - parity on, 0 – 1 stop bit */
	/* 10 – 7 data bits */
}

void send_character (int ch)
{
	unsigned char status;
	do{
		status = inportb(LSR) & 0x40;
	} while ( status!=0x40);
	/*repeat until Tx buffer empty ie bit 6 set */
	outportb(TXDATA,(unsigned char) ch);
}

int get_character(void)
{
	int status;
	do{
		status = inportb(LSR) & 0x01;
	}while (status!=0x01);
	/*Repeat until bit 1 in LSR is set */
	return( (int) inportb(TXDATA));
}

unsigned char sframe_new(int isFile,int ACKorNAK,int ACKno){
	unsigned char temp;
	temp=0;
	temp|=(1<<7)|(1<<5)|(1<<0);
	//set start control stop
	if(isFile==1)temp|=(1<<4);
	if(ACKorNAK==1)temp|=(1<<3);
	if(ACKno==1)temp|=(1<<2);
	return temp;
}

unsigned char* iframe_new_frame(int isFile,int lastframe,int frameno,int size,int* data){// text lastframe put 1
	int asize=size,i;
	unsigned int a;
	unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*12);
	for(a=0;a<12;a++)temp[a]=0;
	temp[0]|=(1<<7); //set start
	temp[11]|=(1);//set stop
	if(isFile==1)temp[0]|=(1<<4);
	if(lastframe==1)temp[0]|=(1<<3);
	if(frameno==1)temp[0]|=(1<<2);
	a=0x60;a&=size;a>>=5;temp[0]+=a;
	a=0x1F;a&=size;a<<=3;temp[1]+=a;			//set size
	//add data
	//0
	a=0x1C0;a&=data[0];a>>=6;
	temp[1]+=a;
	a=0x03F;a&=data[0];a<<=2;
	temp[2]+=a;
	//1
	a=0x180;a&=data[1];a>>=7;
	temp[2]+=a;
	a=0x07F;a&=data[1];a<<=1;
	temp[3]+=a;
	if(asize>1){
		//2
		a=0x100;a&=data[2];a>>=8;
		temp[3]+=a;
		a=0x0FF;a&=data[2];
		temp[4]+=a;
		asize--;
		if(asize>1){
			//3
			a=0x1FE;a&=data[3];a>>=1;
			temp[5]+=a;
			a=0x001;a&=data[3];a<<=7;
			temp[6]+=a;
			asize--;
			if(asize>1){
				//4
				a=0x1FC;a&=data[4];a>>=2;
				temp[6]+=a;
				a=0x003;a&=data[4];a<<=6;
				temp[7]+=a;
				asize--;
				if(asize>1){
					//5
					a=0x1F8;a&=data[5];a>>=3;
					temp[7]+=a;
					a=0x007;a&=data[5];a<<=5;
					temp[8]+=a;
					asize--;
					if(asize>1){
						//6
						a=0x1F0;a&=data[6];a>>=4;
						temp[8]+=a;
						a=0x00F;a&=data[6];a<<=4;
						temp[9]+=a;
						asize--;
						if(asize>1){
							//7
							a=0x1E0;a&=data[7];a>>=5;
							temp[9]+=a;
							a=0x01F;a&=data[7];a<<=3;
							temp[10]+=a;
							asize--;
							if(asize>1){
								//8
								a=0x1C0;a&=data[8];a>>=6;
								temp[10]+=a;
								a=0x03F;a&=data[8];a<<=2;
								temp[11]+=a;
							}
						}
					}
				}
			}
		}
	}
	return temp;
}

int main(void)
{
	int size,i;
	unsigned char sf;
	unsigned char* iframe555;
	int data[]={65,66,67,68,69,70,71,72};

	setup_serial();

	sf=sframe_new(0,1,1);
	//10 10 10 01

	printf("SF %x\n", sf);

	printf("IF PTR %d\n", iframe555);
	iframe555=iframe_new_frame(0,1,0,8,data);

	printf("IF %x\n", iframe555[0]);
	printf("IF %x\n", iframe555[1]);
	printf("IF %x\n", iframe555[2]);
	printf("IF %x\n", iframe555[3]);
	printf("IF %x\n", iframe555[4]);
	printf("IF %x\n", iframe555[5]);
	printf("IF %x\n", iframe555[6]);
	printf("IF %x\n", iframe555[7]);
	printf("IF %x\n", iframe555[8]);
	printf("IF %x\n", iframe555[9]);
	printf("IF %x\n", iframe555[10]);
	printf("IF %x\n", iframe555[11]);

	printf("Len %d\n", strlen(iframe555));

	/*send_character(size);
	printf("SIZE: %d\n",size );
	getch();*/

	puts("SUCCESS\n");

	getch();
}
