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

/*Some ANSI C prototype definitions*/
void setup_serial(void);
void send_character(int ch);
int get_character(void);

void send_string(char* str);

//-------- Frame Declaration --------
typedef struct{
	unsigned char s1;
}sframe;

sframe* sframe_new(unsigned char ACKorNAK,unsigned char ACKno){
	sframe *temp;
	temp->s1=0;
	temp->s1|=(1<<7)|(1<<5)|(1<<4)|(1<<0);//set start control stop
	if(ACKorNAK==1)temp->s1|=(1<<3);
	if(ACKno==1)temp->s1|=(1<<3);
	return temp;
}

typedef struct{
	int i1,i2,i3;
}iframe;


//------- Variable Declaration-----
// Flow control
char name[200],name2[200];
/*
CONTROL	== 00  I-TEXT
		== 01  I-IMG
		== 10  S-TEXT
		== 11  S-IMG	*/
int sender_timer,frame_number_last,frame_number_current,ackR,ackS,control,dataSize;
char** data8bit;


int mode,R_No=0,S_No=0;
char received_data[1000],send_data[1000],c;

//-------FRAME & CONTROL function ----------
int frame_receiver(void);
int frame_sender(unsigned char* str);
unsigned char** Cut8Char(unsigned char str[],int size);

//----------- File ----------
void file_reader(char* filename);

int main( void)
{
	// Setup part
	ackR=0;
	ackS=0;
	setup_serial();
	printf("Select mode( s or r)\t: ");
	c = getche();
	printf("\n");

	//DISABLE_IN_TEST//printf("Enter you name\t: ");
	//DISABLE_IN_TEST//gets(name);

	if(c == 's'||c=='S'){
		mode = 1;
		//DISABLE_IN_TEST//printf("Set up sender time (ms)\t: ");
		//DISABLE_IN_TEST//scanf("%d",&sender_timer);
		//DISABLE_IN_TEST//while((c= getchar()) != '\n' && c != EOF); //Flushing scanf
	}
	else
		mode = 0;
	//printf("\n");



	// Initial part
		// Send+Receive Name & Timer

	
	// Running part
	while(1)
	{
		if(mode == 0){
			printf("RECEIVE\t<< ");
			while(1){
				frame_receiver();
				printf("\t<< %s\n",received_data);
				if(strcmp(received_data,"0")==0)
					break;
			}

			mode = 1; // Change to Sender
		}else if(mode == 1){
			printf("SEND >> ");
			gets(send_data);
			// Add 2D-Parity;
			dataSize=strlen(send_data);
			printf("SIZE : %d\n",dataSize);
			data8bit=Cut8Char(send_data,dataSize);

			// Calculate Number of Frame?
			frame_number_current=0;
			frame_number_last=dataSize/8;
			printf("Frame Number : %d\n",frame_number_last);
			if(strlen(send_data)%8!=0)	// Should not occur
				frame_number_last++;

			// Send All-1 Frame
			while(frame_number_current<frame_number_last){

				//LOOP until receive ACK
				while(1){
					// Create  Frame
					// Send 1 frame + Start counter
					frame_sender(data8bit[frame_number_current]);
					printf("Sent Frame : %d\n",frame_number_current);

						frame_number_current++;	break;	//Temp for test
					// if receive ACK(not TIME OUT) > break & send Next Frame
					
				}

			}
			
			// Send last Frame
			frame_sender("0");// Temp Frame
			printf("Ending Frame");


			mode = 0; // Change to Receiver
		}
	}
	return 0;
}


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

/*void send_string(char* str)
{
	do{
		if(str[S_No]==24){
			send_character(24);
			exit(0);
		}
		send_character(str[S_No++]);

	}while(str[S_No] != '\0');
	send_character('\0');
}*/


//------------------------------------------------------


int frame_receiver(void){
	R_No =0;
	//printf("Receive << ");
	do{
		received_data[R_No] = get_character();
		if(received_data[R_No] == 24)
			exit(0);
	}while((received_data[R_No++]) != 0);
	if(received_data[R_No] == 24)
		exit(0);
	//printf("%s \n",received_data);
	return 1;
}

int frame_sender(unsigned char* str){
	S_No = 0;

	if(strcmp (str,"send")==0){
		strcpy(str,"Successful");
		file_reader("pie");
		exit(0);
	}

	do{
		if(str[S_No]==24){
			send_character(24);
			exit(0);
		}
		send_character(str[S_No++]);

	}while(str[S_No] != '\0');
	send_character('\0');

	return 0;
}

unsigned char** Cut8Char(unsigned char str[],int size){
	int row= 0,j=0;
	unsigned char **word;
	int i,Rsize=(size%8==0)? size/8:(size/8)+1;
	word=(unsigned char**)malloc((Rsize)*sizeof(unsigned char*));
	for(i=0;i<=Rsize;i++){
		word[i]=(unsigned char* )malloc(8*sizeof(unsigned char));
	}
	i=0;
	while(size>j){
		//printf("|| %d - %d - %d ||  ",i,row,j);
		word[row][i]=str[j];
		i++;j++;
		if(i==8){i=0;++row;}
	}
	return word;
}

// File part

void file_reader(char* filename){
	char fileSize=0;
	unsigned char *ptr,ch;
	int i=0;
	FILE *fp1 = NULL, *fp2 = NULL;

	// rb= read-binary only start first 
	// wb = write-binary 
	fp1=fopen("Print.ico", "rb");
	fp2=fopen("Print2.ico", "wb");
	// fp1=fopen("text.txt", "rb");
	// fp2=fopen("text2.txt", "wb");

	fseek(fp2, SEEK_SET, 0);
	while(1){			// Find Size
		fread(&ch,sizeof(unsigned char),1, fp1);
		fileSize++;
		if(feof(fp1)) break;
		
	}
	printf("FILE Size: %d\n",fileSize);
	ptr = (unsigned char*)malloc(sizeof(unsigned char)*fileSize); //CREAT Array

	fseek(fp1, SEEK_SET, 0);
	while(1){
		fread(&ch,sizeof(unsigned char),1, fp1);
		if(feof(fp1)) break;
		ptr[i++] = ch; //Save into Array
		fwrite(&ch,sizeof(unsigned char),1, fp2);
	}

	//fwrite(ptr,sizeof(unsigned char),fileSize, fp2);
	// for(i=0;i<fileSize;i++){
	// 	printf("%c ",ptr[i]);
	// 	ch=ptr[i];
	// 	fwrite(*ch,sizeof(unsigned char),1, fp2);
	// }

	printf("\n");
	//free(ptr);
	fclose(fp1);
	fclose(fp2);
}