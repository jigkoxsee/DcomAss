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
	int is3=0,is2=0;
	int asize=size;
	unsigned int a,b,i,j;
	unsigned char* tempp = (unsigned char*)malloc(sizeof(unsigned char)*12);
	int i_frame_part1,i_frame_part2,i_frame_part3=0;
	//tempp=new unsigned char[12];
	i_frame_part1=i_frame_part2=i_frame_part3=0;
	if(isFile==1)i_frame_part1|=(1<<28);
	if(lastframe==1)i_frame_part1|=(1<<27);
	if(frameno==1)i_frame_part1|=(1<<26);
	asize<<=19;
	i_frame_part1+=asize;								//set size
	//add data
	a=data[0];											//0
	a<<=10;
	i_frame_part1+=a;
	size-=8;
	if(size>0){
		a=data[1]; a<<=1; i_frame_part1+=a;size-=8;				//1
		if(size>0){
			a=data[2];										//2
			if(a>=256){i_frame_part1|=(1<<0);a-=256;}
			if(a>=128){is2=1;a-=128;}
			a<<=24;i_frame_part2+=a;size-=8;
			if(size>0){
				a=data[3]; a<<=15; i_frame_part2+=a;	size-=8;			//3
				if(size>0){
					a=data[4]; a<<=6;  i_frame_part2+=a;	size-=8;		//4
					if(size>0){
						a=data[5];									//5
						if(a>=256){i_frame_part1|=(1<<5);a-=128;}
						if(a>=128){i_frame_part1|=(1<<4);a-=128;}
						if(a>=64){i_frame_part1|=(1<<3);a-=64;}
						if(a>=32){i_frame_part1|=(1<<2);a-=32;}
						if(a>=16){i_frame_part1|=(1<<1);a-=16;}
						if(a>=8){i_frame_part1|=(1<<0);a-=8;}
						if(a>=4){is3=1;a-=4;}
						a<<=29;i_frame_part3+=a;size-=8;
						if(size>0){
							a=data[6]; a<<=20; i_frame_part3+=a;	size-=8;			//6
							if(size>0){
								a=data[7]; a<<=11; i_frame_part3+=a;	size-=8;		//7
								if(size>0){
									a=data[8]; a<<=2;  i_frame_part3+=a;				//8
									i_frame_part3|=(1<<0);								//set stop bit
								}else i_frame_part3|=(1<<9);								//set stop bit//8
							}else i_frame_part3|=(1<<18);								//set stop bit//7
						}else i_frame_part3|=(1<<27);								//set stop bit//6
					}else i_frame_part2|=(1<<4);								//set stop bit//5
				}else i_frame_part2|=(1<<13);								//set stop bit//4
			}else i_frame_part2|=(1<<22);								//set stop bit//3
		}else is2=1;								//set stop bit//2
	}else i_frame_part1|=(1<<8);							//set stop bit  //1
	i=0;a=0xFF000000;j=24;
	while(i<=3){
		b=a;
		b&=i_frame_part1;
		b>>=j;
		tempp[i]=(unsigned char)b;
		i++;a/=0x00000100;j-=8;
	}
	a=0xFF000000;j=24;
	while(i<=7){
		b=a;
		b&=i_frame_part2;
		b>>=j;
		tempp[i]=(unsigned char)b;
		i++;a/=0x00000100;j-=8;
	}
	a=0xFF000000;j=24;
	while(i<=11){
		b=a;
		b&=i_frame_part3;
		b>>=j;
		tempp[i]=(unsigned char)b;
		i++;a/=0x00000100;j-=8;
	}
	tempp[0]|=(1<<8);							//set start bit and control
	if(is2==1)tempp[4]|=(1<<8);
	if(is3==1)tempp[8]|=(1<<8);

	return tempp;
}

int main( void)
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


	
	/*send_character(size);
	printf("SIZE: %d\n",size );
	getch();*/

	puts("SUCCESS\n");

	getch();
}