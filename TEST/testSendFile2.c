#define COM1BASE 0x3F8
#define COM2BASE 0X2F8
#define COM3BASE 0X3E8
#define TXDATA COM3BASE
#define LCR (COM3BASE+3) /*0x3F8 line control*/
#define LSR (COM3BASE+5) /*0x3FD line status */
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

int file_reader(char* filename){
	int fileSize=0;
	short ch;
	int i=0;
	FILE *fp1 = NULL;

	// rb= read-binary only start first 
	fp1=fopen(filename, "rb");

	printf("FILE Open: %s\n",filename);

	while(1){	// Find Size
		fread(&ch,sizeof(short),1, fp1);
		if(feof(fp1))
			break;
		fileSize++;
	}

	printf("FILE Size: %d\n",fileSize);

	file_ptr = (short*)malloc(sizeof(short)*fileSize); //CREAT Array

	fseek(fp1, SEEK_SET, 0);
	while(1){
		fread(&ch,sizeof(short),1, fp1);
		if(feof(fp1))
			break;
		file_ptr[i++] = *&ch; //Save into Array		printf("%c\t",ch);
	}
	fclose(fp1);

	return fileSize;
}

void file_writer(char* filename,int fileSize,unsigned char* ptr){
	char* filename_new;

	FILE *fileOut = NULL;

	// wb = write-binary 
	fileOut=fopen(filename, "wb");

	printf("FILE write: %s\n",filename);

	fwrite(ptr,sizeof(unsigned char),fileSize, fileOut);

	fclose(fileOut);
	printf("FILE write: Success\n");

}


unsigned int SgetStart(char frame){
	unsigned char temp=0xC0;
	temp&=frame;
	temp>>=6;
	return temp;
}


unsigned int SgetControl(char frame){
	unsigned char temp=0x30;
	temp&=frame;
	temp>>=4;
	return temp;
}

unsigned int IgetIslastframe(char* frame){
	unsigned char temp=0x08;
	temp&=frame[0];
	temp>>=3;
	return temp;
}


unsigned int SgetEnd(char frame){
	unsigned char temp=0x03;
	temp&=frame;
	return temp;
}

unsigned int IgetEnd(char* frame){
	unsigned char temp=0x03;
	temp&=frame[11];
	return temp;
}

unsigned int IgetStart(char* frame){
	unsigned char temp=0xC0;
	temp&=frame[0];
	temp>>=6;
	return temp;
}

unsigned int IgetControl(char* frame){
	unsigned char temp=0x30;
	temp&=frame[0];
	temp>>=4;
	return temp;
}
unsigned int IgetFrameno(char* frame){
	unsigned char temp=0x04;
	temp&=frame[0];
	temp>>=2;
	return temp;
}

unsigned int  IgetSize(char* frame){
	unsigned char temp=0x03,a=0xF8;
	temp&=frame[0];
	temp<<=5;
	a&=frame[1];
	a>>=3;
	temp+=a;
	return temp;
}
unsigned int* Igetdata(char* frame){
	unsigned int temp[9]={0,0,0,0,0,0,0,0,0};
	unsigned int a;
//0
	a=0x07;a&=frame[1];a<<=6;
	temp[0]+=a;
	a=0xFC;a&=frame[2];a>>=2;
	temp[0]+=a;

//1
	a=0x03;a&=frame[2];a<<=7;
	temp[1]+=a;
	a=0xFE;a&=frame[3];a>>=1;
	temp[1]+=a;

//2
	a=0x01;a&=frame[3];a<<=8;
	temp[2]+=a;
	a=0xFF;a&=frame[4];
	temp[2]+=a;

//3
	a=0xFF;a&=frame[5];a<<=1;
	temp[3]+=a;
	a=0x80;a&=frame[6];a>>=7;
	temp[3]+=a;

//4
	a=0x7F;a&=frame[6];a<<=2;
	temp[4]+=a;
	a=0xC0;a&=frame[7];a>>=6;
	temp[4]+=a;

//5
	a=0x3F;a&=frame[7];a<<=3;
	temp[5]+=a;
	a=0xE0;a&=frame[8];a>>=5;
	temp[5]+=a;

//6
	a=0x1F;a&=frame[8];a<<=4;
	temp[6]+=a;
	a=0xF0;a&=frame[9];a>>=4;
	temp[6]+=a;

//7
	a=0x0F;a&=frame[9];a<<=5;
	temp[7]+=a;
	a=0xF8;a&=frame[10];a>>=3;
	temp[7]+=a;

//8
	a=0x07;a&=frame[10];a<<=6;
	temp[8]+=a;
	a=0xFC;a&=frame[11];a>>=2;
	temp[8]+=a;

	return &temp;
}


int main( void)
{
	int size,i;
	char ch;
	unsigned char* dataparity;
	unsigned char* data;

	unsigned char* datain = (unsigned char*)malloc(sizeof(unsigned char)*12);
	setup_serial();

	size=get_character();
	printf("SIZE:%d\n",size);

	printf("-----------\n");

	i=0;
	while(size--){
		printf("I : %d",i);
		ch=get_character();
		datain[i++]=ch;
		printf(">> %x\n",datain[i-1]);
	}
	puts("RECEIVED\n");

	printf("I>S : %d\n ",IgetStart(datain));
	printf("I>C : %d\n ",IgetControl(datain));
	printf("I>F : %d\n ",IgetFrameno(datain));
	printf("I>Z : %d\n ",IgetSize(datain));

	dataparity=Igetdata(datain);
	data=(unsigned char*)malloc(sizeof(unsigned char)*IgetSize(datain));
	size=IgetSize(datain);

	i=0;
	while(i<size){
		data[i++]=(*dataparity)>>1;
		dataparity++;
	}

	i=0;
	while(i<size){
		printf("D: %c\n",data[i]);
		data++;
		i++;
	}


	
	// i=0;
	// while(size--){
	// 	printf("I : %d",i);
	// 	ch=get_character();
	// 	file_ptr[i++]=ch;
	// 	printf(">> %c\n",ch);
	// }
	// puts("RECEIVED\n");
	// file_writer("NEW2.txt",i,file_ptr);


	puts("SUCCESS\n");
	getch();

}