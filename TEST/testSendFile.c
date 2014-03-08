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

int file_reader(char* filename){
	int fileSize=0;
	unsigned char ch;
	int i=0;
	FILE *fp1 = NULL;
	// rb= read-binary only start first 
	fp1=fopen(filename, "rb");

	printf("FILE Open: %s\n",filename);
	while(1){	// Find Size
		fread(&ch,sizeof(unsigned char),1, fp1);
		if(feof(fp1))
			break;
		fileSize++;
	}
	printf("FILE Size: %d\n",fileSize);
	file_ptr = (unsigned char*)malloc(sizeof(unsigned char)*fileSize); //CREAT Array
	fseek(fp1, SEEK_SET, 0);
	while(1){
		fread(&ch,sizeof(unsigned char),1, fp1);
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


int main( void)
{
	int size,i;
	setup_serial();

	size=file_reader("NEW_FOPE.txt");
	
	//size=4;

	send_character(size);
	printf("SIZE: %d\n",size );
	getch();


	for(i=0;i<size;i++){
		send_character(*file_ptr++);
		//send_character('A');
	}
	//send_character('0');
	//send_character('0');
	puts("SUCCESS\n");

	getch();
}