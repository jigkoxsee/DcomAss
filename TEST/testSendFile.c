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

int parityGen(int v) { //EVEN Parity
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
return (0x6996 >> v) & 1;
}

int* parityEncap(unsigned char* input,int size){
    int i,j;
    int mask = 0x01;
    unsigned char verticalBit[8];
    unsigned char temp;
    unsigned char lastParitySubFrame = 0;
    unsigned char tempLastParity = 0;
    int outputTemp;
    int parityTemp;
    int *output = malloc(size);
    int lastParity;
    int tempRightBottomParity = 0;

    //printf("show parity encap vertical \n");

    //get Vertical bit to char 8 bit
    for (i = 0;i < 8;i++){
        //printf("------------------------ \n");
        //printf(" i = %d",i);
        mask = 0x01 << i;
        if(i == 0)mask=0x01;
        verticalBit[i] = 0;
        for(j = 0;j < size;j++){

            temp = (input[j] & mask) >> i;
            verticalBit[i] += (temp << j);
        }
        tempLastParity = (char)parityGen((int)verticalBit[i]); // parity Gen
        lastParitySubFrame += (tempLastParity << i); // for parity right bottom by add every value
    }
    tempRightBottomParity += lastParitySubFrame;
    for (i = 0;i < size;i++){
        parityTemp = parityGen(input[i]);
        outputTemp = (int)input[i] << 1;
        outputTemp += parityTemp;
        output[i] = outputTemp;
        tempRightBottomParity += parityTemp << i;
    }
    lastParity = (int)lastParitySubFrame;
    lastParity = lastParity << 1;
    if(tempRightBottomParity % 2 != 0){
            lastParity += 1;
    }
    output[8] = lastParity;
    return output;
}

int parityChecker(int* input,int size){
    int i;
    for(i = 0;i < size;i++){
        if (parityGen(input[i]) != 0){
            return 0;
        }
    }
    return 1;
}


int main(void)
{
	int size,i;
	unsigned char* iframe555;
	unsigned char data[]={65};
	int* data_parity;
	size=1;
	//int *parityEncap(unsigned char* input,int size)
	data_parity=parityEncap(data,size);

	//unsigned char* iframe_new_frame(int isFile,int lastframe,int frameno,int size,int* data){
	iframe555=iframe_new_frame(0,1,0,1,data_parity);

	setup_serial();

	send_character(12);


	send_character(iframe555[0]);
	printf("IF : %x\n",iframe555[0]);
	send_character(iframe555[1]);
	printf("IF : %x\n",iframe555[1]);
	send_character(iframe555[2]);
	printf("IF : %x\n",iframe555[2]);
	send_character(iframe555[3]);
	printf("IF : %x\n",iframe555[3]);
	send_character(iframe555[4]);
	printf("IF : %x\n",iframe555[4]);
	send_character(iframe555[5]);
	printf("IF : %x\n",iframe555[5]);
	send_character(iframe555[6]);
	printf("IF : %x\n",iframe555[6]);
	send_character(iframe555[7]);
	printf("IF : %x\n",iframe555[7]);
	send_character(iframe555[8]);
	printf("IF : %x\n",iframe555[8]);
	send_character(iframe555[9]);
	printf("IF : %x\n",iframe555[9]);
	send_character(iframe555[10]);
	printf("IF : %x\n",iframe555[10]);
	send_character(iframe555[11]);
	printf("IF : %x\n",iframe555[11]);


	//size=4;
	/*size=file_reader("NEW_FOPE.txt");
	send_character(size);
	printf("SIZE: %d\n",size );
	getch();

	for(i=0;i<size;i++){
		send_character(*file_ptr++);
	}*/



	puts("SUCCESS\n");

	getch();

	return 0;
}