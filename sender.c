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

//------- Variable Declaration-----
// Flow control
char name[100],name2[100];
int sender_timer;


int mode,R_No=0,S_No=0;
char received_data[200],send_data[200],c;

//-------FRAME & CONTROL function ----------
int frame_receiver(void);
int frame_sender(unsigned char* str);


int main( void)
{
	// Setup part
	setup_serial();
	printf("Select mode( s or r) :");
	c = getche();
	if(c == 's'||c=='S')
		mode = 1;
	else
		mode = 0;
	printf("\n");

	// Running part
	while(1)
	{
		if(mode == 0){
			mode = frame_receiver();
		}else if(mode == 1){
			printf("SEND >> ");
			gets(send_data);
			mode = frame_sender(send_data);
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

void send_string(char* str)
{
	do{
		if(send_data[S_No]==24){
			send_character(24);
			exit(0);
		}
		send_character(send_data[S_No++]);

	}while(send_data[S_No] != '\0');
	send_character('\0');
}


//------------------------------------------------------


int frame_receiver(void){
	R_No =0;
	printf("Receive << ");
	do{
		received_data[R_No] = get_character();
		if(received_data[R_No] == 24)
			exit(0);
	}while((received_data[R_No++]) != 0);
	if(received_data[R_No] == 24)
		exit(0);
	printf("%s \n",received_data);
	return 1;
}

int frame_sender(unsigned char* str){
	S_No = 0;
	// printf("SEND >> ");
	// gets(send_data);
	do{
		if(send_data[S_No]==24){
			send_character(24);
			exit(0);
		}
		send_character(send_data[S_No++]);

	}while(send_data[S_No] != '\0');
	send_character('\0');

	return 0;
}

// File part

void file_reader(char* filename){
	char fileSize=0;
	short *ptr,ch;
	int i=0;
	FILE *fp1 = NULL, *fp2 = NULL;

	// rb= read-binary only start first 
	fopen(&fp1, "test.txt", "rb");
	

	fseek(fp2, SEEK_SET, 0);

	while(1){// Find Size
		fread(&ch,sizeof(short),1, fp1);
		fileSize++;
		if(feof(fp1)) break;
		
	}

	fseek(fp1, SEEK_SET, 0);
	ptr = (short*)malloc(sizeof(short)*fileSize);

	while(1){
		fread(&ch,sizeof(short),1, fp1);
		if(feof(fp1)) break;
		ptr[i] = ch; //Save into Array
		i++;
		fwrite(&ch,sizeof(short),1, fp2);
		printf("!!!!!!7\n");
	}
	fclose(fp1);


	printf("!!!!!!\n");
	printf("\nEnd\n");

	// wb = write-binary 
	fopen(&fp2, "test_copy.txt", "wb");
	for(i=0;i<fileSize;i++){
		printf("%c ",ptr[i]);

	}

	printf("\n");
	fclose(fp2);
}