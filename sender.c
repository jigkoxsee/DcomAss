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

#include "dcom.c"
#include "control.c"
#include "error.c"
#include "file.c"
#include "frame.c"
#include "time.c"


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
