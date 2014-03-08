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

/*Some ANSI C prototype definitions*/
void setup_serial(void);
void send_character(int ch);
int get_character(void);

void send_string(char* str);

//-------- Frame Declaration --------
typedef struct{
	int i1,i2,i3;
}iframe;

typedef struct{
	int s1;
}sframe;

iframe iframe_new(int isimg,int lastframe,int frameno,int size,int data[]);

sframe sframe_new(int isimg,int ACKorNAK,int ACKno);


//-------------- Variable Declaration------------
// Flow control
char name[200],name2[200];
/*
CONTROL	== 00  I-TEXT
		== 01  I-IMG
		== 10  S-TEXT
		== 11  S-IMG	*/
int sender_timer,frame_number_last,frame_number_current,ackR,ackS,control,dataSize;
char** data8bit;
int control_is_txt_img;
short *file_ptr;

int mode,R_No=0,S_No=0;
char send_data[1000],c;
unsigned char* received_frame;

//-------FRAME & CONTROL function ----------
unsigned char* frame_receiver(void);
int frame_sender(unsigned char* str);
unsigned char** Cut8Char(unsigned char str[],int size);

//----------- File ----------
int file_reader(char* filename);
void file_writer(char* filename,int fileSize,short *ptr);

int main( void)
{
	char filename[100];
	int fileSize;

	puts("Enter file name : ");
	gets(filename);

	fileSize=file_reader(filename);
	file_writer("icon.ico",fileSize,file_ptr);
	getch();
	return 0;
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

void file_writer(char* filename,int fileSize,short *ptr){
	char* filename_new;
	short ch;

	FILE *fileOut = NULL;

	// wb = write-binary 
	fileOut=fopen(filename, "wb");

	printf("FILE write: %s\n",filename);

	fwrite(ptr,sizeof(short),fileSize, fileOut);

	fclose(fileOut);
	printf("FILE write: Success\n");
}
