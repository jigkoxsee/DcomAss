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
/*Some ANSI C prototype definitions*/
void setup_serial(void);
void send_character(int ch);
int get_character(void);

void send_string(char* str);

//-------- Frame Declaration --------
typedef struct{
	int i1,i2,i3;
}iframe_sender;

/*typedef struct{
	int s1;
}sframe;*/

iframe_sender iframe_new(int isFile,int lastframe,int frameno,int size,int data[]);

char sframe_new(int isFile,int ACKorNAK,int ACKno);


//-------------- Variable Declaration------------
// Flow control
char name[200],name2[200];
/*
CONTROL	== 00  I-TEXT
		== 01  I-IMG
		== 10  S-TEXT
		== 11  S-IMG	*/
int sender_timer,startTime,frame_number_last,frame_number_current,ackR,ackS,control,dataSize;
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
int getTimer();
int startTimer();
int isS_Frame(unsigned char* received_frame);
void S_Frame_receive(unsigned char* received_frame);
void I_Frame_receive(unsigned char* received_frame);


//---------- File ----------
int file_reader(char* filename);
void file_writer(char* filename,int fileSize,short *ptr);
void file_sender(char* file_location);
			//void file_receiver(char* file_location);



int main( void)
{

	// FOR TEST
	strcpy(name,"COM1");
	strcpy(name2,"COM2");

	// Setup part
	ackR=0;
	ackS=0;
	control_is_txt_img=1;
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
			while(1){
				printf("%s << ",name);
				received_frame=frame_receiver();
				if(strchr(received_frame,24)>=0){
						// Send ACK to Sender before EXIT
						exit(0);
				}

				if(isS_Frame(received_frame){
					//--------------------------------SSSSSS--------------------------------
					S_Frame_receive(received_frame); // Set control_is_txt_img << in HERE
				}else{
					//--------------------------------IIIIII--------------------------------
					I_Frame_receive(received_frame); // Check control_is_txt_img before receive
				}
			}
			printf("Ending Frame");
			mode = 1; // Change to Sender

		}else if(mode == 1){

			printf("%s >> ",name);
			gets(send_data);

			if(strcmp(send_data,"send")==0){
				//----------------------------------FILE MODE----------------------------------
				char file_location[300];	
				printf("Enter file location : ");
				gets(file_location);
				file_sender(file_location); // Pass File data & file size to frame_sender << HERE
			}else{
				//----------------------------------TEXT MODE----------------------------------
				text_sender(send_data);
			}
			printf("Ending Frame");
			mode = 0; // Change to Receiver
			
		}
	}
	return 0;
}

//------------------------------------------------------
int wait_ack(int ackNo){
	int status;
	do{
		status = inportb(LSR) & 0x01;

		if(getTimer()==0){ // TIME OUT -> Send new frame
			printf("TIME-OUT\n");
			return 1; // Not receive ACK
		}
	}while (status!=0x01);
	//Repeat until get Something

	if(!checkACK(inportb(TXDATA))){
		return 1;	// Fail to receive ACK
	}

	return 0; // Receive ACK & Send Complete
}

void control_sender(int* data_parity,int frame_no,int isFile,int dataSize){
//iframe_new(int isFile,int lastframe,int frameno,int size,int data[]);// text lastframe put 1
	char* frame;	

	// Send All-1 Frame
	while(frame_number_current<frame_number_last){

		// CREATE frame from << data_parity
		frame=iframe_new(isFile,0,frame_number_current%2,dataSize-frame_number_current*8 /*size*/,data_parity[frame_number_current]);// text lastframe put 1
		printf("Frane Data size : %d\n",dataSize-frame_number_current*8);

		while(send_unsuccess){
			// Send 1 frame + Start counter
			frame_sender(frame);
			printf("Sent Frame : %d\n",frame_number_current);
			startTimer(sender_timer);

			send_unsuccess=wait_ack(frame_number_current%2); // frame_number_current%2 == 0,1 << ACK 					

			// EXIT PROGRAM
		}
		frame_number_current++; // Next frame

	}

	// LAST FRAME;
	iframe_new(isFile,1,frame_number_current%2,dataSize-frame_number_current*8,data_parity[frame_number_current]);// text lastframe put 1
	while(send_unsuccess){
		// Send 1 frame + Start counter
		frame_sender(frame);
		printf("Sent Frame : %d\n",frame_number_current);
		startTimer(sender_timer);

		send_unsuccess=wait_ack(frame_number_current%2); // frame_number_current%2 == 0,1 << ACK 					

		// EXIT PROGRAM
	}
}

void text_sender(char* send_data){
	int* send_data;
	int* data_parity;
	int send_success;

	send_unsuccess=1;
	//----------SSSSSSSS----------
	// Create S_FRAME;
	while(send_unsuccess){
		// Send 1 frame + Start counter
		frame_sender(s_frame);
		printf("Sent S-Frame\n");
		startTimer(sender_timer);
		send_unsuccess=wait_ack(0); // frame_number_current%2 == 0,1 << ACK 					
	}


	//----------IIIIIIII----------
	send_unsuccess=1;
	dataSize=strlen(send_data);
	printf("SIZE : %d\n",dataSize);
	data8bit=Cut8Char(send_data,dataSize);

	// Add 2D-Parity (send_data,dataSize)
	// Use new pointer from parity to Send >> data_parity
	data_parity=parityGenerator(send_data,dataSize);

	frame_number_current=0;
	frame_number_last=dataSize/8;
	printf("Frame Number : %d\n",frame_number_last);
	if(dataSize%8!=0)
		frame_number_last++;

	control_sender(data_parity,frame_number_last,0,dataSize);

}

int frame_sender(unsigned char* str){
	S_No = 0;

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

unsigned char* frame_receiver(void){
	unsigned char received_data[100];
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
	return received_data;
}


unsigned char** Cut8Char(unsigned char str[],int size){
	int row= 0,j=0;
	unsigned char** word;
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
		if(i==8){
			i=0;
			++row;
		}
	}
	return word;
}

// File part

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

// SET UP PART ---------------

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



// --------------- Frame function -----------------

/*isFile>>img=1//text=0*/
char* iframe_new(int isFile,int lastframe,int frameno,int size,int data[]){// text lastframe put 1
	int is3=0,is2=0;
	int asize=size;
	unsigned int a,b,i,j;
	char tempp[12];
	iframe_sender temp;
	//tempp=new char[12];
	temp.i1=temp.i2=temp.i3=0;
	if(isFile==1)temp.i1|=(1<<28);
	if(lastframe==1)temp.i1|=(1<<27);
	if(frameno==1)temp.i1|=(1<<26);
	asize<<=19;
	temp.i1+=asize;								//set size
	//add data
	a=data[0];											//0
	a<<=10;
	temp.i1+=a;
	size-=8;
	if(size>0){
		a=data[1]; a<<=1; temp.i1+=a;size-=8;				//1
		if(size>0){
			a=data[2];										//2
			if(a>=256){temp.i1|=(1<<0);a-=256;}
			if(a>=128){is2=1;a-=128;}
			a<<=24;temp.i2+=a;size-=8;
			if(size>0){
				a=data[3]; a<<=15; temp.i2+=a;	size-=8;			//3
				if(size>0){
					a=data[4]; a<<=6;  temp.i2+=a;	size-=8;		//4
					if(size>0){
						a=data[5];									//5
						if(a>=256){temp.i1|=(1<<5);a-=128;}
						if(a>=128){temp.i1|=(1<<4);a-=128;}
						if(a>=64){temp.i1|=(1<<3);a-=64;}
						if(a>=32){temp.i1|=(1<<2);a-=32;}
						if(a>=16){temp.i1|=(1<<1);a-=16;}
						if(a>=8){temp.i1|=(1<<0);a-=8;}
						if(a>=4){is3=1;a-=4;}
						a<<=29;temp.i3+=a;size-=8;
						if(size>0){
							a=data[6]; a<<=20; temp.i3+=a;	size-=8;			//6
							if(size>0){
								a=data[7]; a<<=11; temp.i3+=a;	size-=8;		//7
								if(size>0){
									a=data[8]; a<<=2;  temp.i3+=a;				//8
									temp.i3|=(1<<0);								//set stop bit
								}else temp.i3|=(1<<9);								//set stop bit//8
							}else temp.i3|=(1<<18);								//set stop bit//7
						}else temp.i3|=(1<<27);								//set stop bit//6
					}else temp.i2|=(1<<4);								//set stop bit//5
				}else temp.i2|=(1<<13);								//set stop bit//4
			}else temp.i2|=(1<<22);								//set stop bit//3
		}else is2=1;								//set stop bit//2
	}else temp.i1|=(1<<8);							//set stop bit  //1
	i=0;a=0xFF000000;j=24;
	while(i<=3){
		b=a;
		b&=temp.i1;
		b>>=j;
		tempp[i]=(char)b;
		i++;a/=0x00000100;j-=8;
	}
	a=0xFF000000;j=24;
	while(i<=7){
		b=a;
		b&=temp.i2;
		b>>=j;
		tempp[i]=(char)b;
		i++;a/=0x00000100;j-=8;
	}
	a=0xFF000000;j=24;
	while(i<=11){
		b=a;
		b&=temp.i3;
		b>>=j;
		tempp[i]=(char)b;
		i++;a/=0x00000100;j-=8;
	}
	tempp[0]|=(1<<8);							//set start bit and control
	if(is2==1)tempp[4]|=(1<<8);
	if(is3==1)tempp[8]|=(1<<8);

	return tempp;
}

char sframe_new(int isFile,int ACKorNAK,int ACKno){
	char temp; temp=0;
	temp|=(1<<7)|(1<<5)|(1<<0);

//set start control stop if(isFile==1)temp|=(1<<4);
	if(ACKorNAK==1)temp|=(1<<3);
	if(ACKno==1)temp|=(1<<2);
	return temp;
}
//-----------------------------TIMER-----------------------------
int startTimer(){
	startTime=clock();
}

int getTimer(){
	return sender_timer-(startTime-clock());
}