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

int mode,R_No=0,S_No=0;
char send_data[1000],c;
unsigned char* received_frame;

//-------FRAME & CONTROL function ----------
unsigned char* frame_receiver(void);
int frame_sender(unsigned char* str);
unsigned char** Cut8Char(unsigned char str[],int size);

//----------- File ----------
void file_reader(char* filename);

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
					S_Frame_receive(); // Set control_is_txt_img << in HERE
				}else{
					//--------------------------------IIIIII--------------------------------
					I_Frame_receive(); // Check control_is_txt_img before receive
				}
			}

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

				//printf("File : %s \n",file_location);
				//file_reader(file_location);

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
/*void wait_ack(int ackNo){
	// if receive ACK(not TIME OUT) > break & send Next Frame
	int status;
	do{
		status = inportb(LSR) & 0x01;
	}while (status!=0x01);
	//Repeat until bit 1 in LSR is set

	if(elapse==sender_timer){ // TIME OUT
		printf("TIME-OUT\n");
		break;
	}

	//inportb(TXDATA);

}*/
void text_sender(char* send_data){
	// Add 2D-Parity;
	int parityGen(int v) {
		//EVEN Parity
		v ^= v >> 16;
		v ^= v >> 8;
		v ^= v >> 4;
		v &= 0xf;
		return (0x6996 >> v) & 1;
	}
	//---------------------
	
	dataSize=strlen(send_data);
	printf("SIZE : %d\n",dataSize);
	data8bit=Cut8Char(send_data,dataSize);

	// Calculate Number of Frame?
	frame_number_current=0;
	frame_number_last=dataSize/8;
	printf("Frame Number : %d\n",frame_number_last);
	if(dataSize%8!=0)	// Should not occur
		frame_number_last++;

	// Send All-1 Frame
	while(frame_number_current<frame_number_last){

		// Create  Frame
		// Send 1 frame + Start counter
		frame_sender(data8bit[frame_number_current]);
		printf("Sent Frame : %d\n",frame_number_current);

		//wait_ack(frame_number_current%2); // frame_number_current%2 == 0,1 << ACK 					

		// EXIT PROGRAM
		if(strchr(data8bit[frame_number_current],24)>=0){
			exit(0);
		}
		frame_number_current++; // Next frame

	}
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

void file_reader(char* filename){
	int fileSize=0;
	char* filename_new;
	short *ptr,ch;
	int i=0;
	FILE *fp1 = NULL, *fp2 = NULL;

	// rb= read-binary only start first 
	// wb = write-binary 
	//fp1=fopen("Print.ico", "rb");
	fp1=fopen(filename, "rb");

	i=strrchr(filename,'/');

	strcpy (filename_new,filename);
	strcat(filename_new,"_new");
	fp2=fopen(filename_new, "wb");

	printf("FILE Open: %s\n",filename);
	printf("FILE write: %s\n",filename_new);
	while(1);
	// fp1=fopen("text.txt", "rb");
	// fp2=fopen("text2.txt", "wb");

	fseek(fp2, SEEK_SET, 0);
	while(1){			// Find Size
		fread(&ch,sizeof(short),1, fp1);
		if(feof(fp1))
			break;
		fileSize++;
	}

	printf("FILE Size: %d\n",fileSize);
	ptr = (short*)malloc(sizeof(short)*fileSize); //CREAT Array

	i=0;
	fseek(fp1, SEEK_SET, 0);
	while(1){
		fread(&ch,sizeof(short),1, fp1);
		if(feof(fp1)) break;
		ptr[i++] = *&ch; //Save into Array
		//fwrite(&ch,sizeof(short),1, fp2);
		printf("%c\t",ch);
	}

	// Write into file
	//fwrite(ptr,sizeof(short),fileSize, fp2);
	for(i=0;i<fileSize;i++){
		// Send Frame << ptr[i] >>

	}

	//free(ptr);
	fclose(fp1);
	fclose(fp2);
}

void file_writer(char* filename,int fileSize){
	char* filename_new;
	short *ptr,ch;

	FILE *fileOut = NULL;

	// wb = write-binary 
	fileOut=fopen(filename, "wb");

	printf("FILE write: %s\n",filename);

	while(fileSize--){
		// Write into file
		fwrite(ptr,sizeof(short),1, fileOut);
	}

	//free(ptr);
	fclose(fileOut);
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

/*isimg>>img=1//text=0*/
iframe iframe_new(int isimg,int lastframe,int frameno,int size,int data[]){// text lastframe put 1
	int is3=0,is2=0;
	int asize=size;
	int a;
	iframe temp;
	temp.i1=temp.i2=temp.i3=0;
	if(isimg==1)temp.i1|=(1<<28);
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
	temp.i1*=-1;							//set start bit and control
	if(is2==1)temp.i2*=-1;
	if(is3==1)temp.i3*=-1;
	return temp;
}

sframe sframe_new(int isimg,int ACKorNAK,int ACKno){
	sframe temp;

	temp.s1=0;
	temp.s1|=(1<<7)|(1<<5)|(1<<0);//set start control stop
	
	if(isimg==1)
		temp.s1|=(1<<4);
	if(ACKorNAK=1)
		temp.s1|=(1<<3);
	if(ACKno==1)
		temp.s1|=(1<<2);

	return temp;
}