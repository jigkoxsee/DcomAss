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


int main( void)
{

	//------------------FOR TEST------------------
	strcpy(name,"COM1");
	strcpy(name2,"COM2");

	//---------------FOR TEST END-----------------

	// Setup part
	ackR=0;
	ackS=0;
	control_is_txt_img=1;
	setup_serial();

	printf("Select mode( s or r)\t: ");
	c = getche();
	printf("\n");

	printf("Enter you name\t: ");
	gets(name);

	if(c == 's'||c=='S'){
		mode = 1;
		printf("Set up sender time (ms)\t: ");
		scanf("%d",&sender_timer);
		while((c= getchar()) != '\n' && c != EOF); //Flushing scanf
	}
	else
		mode = 0;




	//-------------------Initial part-------------------\\
	//Send+Receive Name & Timer

	if(mode==1){//SENDER

		// Send name
		text_sender(name);
		// Send Timer
		text_sender(sender_timer);
		// Wait for name

	}else{//RECEIVER

		// Wait for name
		I_Frame_receive(received_frame)
		// Wait for Timer
		// Send name
		text_sender(name);

	}

	//-------------------Initial part-------------------//


	
	//-------------------Running part-------------------\\
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

				if(isS_Frame(received_frame)){
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
