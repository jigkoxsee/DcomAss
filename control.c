
//-------FRAME & CONTROL function ----------
unsigned char* frame_receiver(void);
int frame_sender(unsigned char* str);
unsigned char** Cut8Char(unsigned char str[],int size);

int isS_Frame(unsigned char* received_frame);
void S_Frame_receive(unsigned char* received_frame);
void I_Frame_receive(unsigned char* received_frame);


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

	if(!checkACK(inportb(TXDATA),ackNo)){
		return 1;	// Fail to receive ACK
	}

	return 0; // Receive ACK & Send Complete
}

void control_sender(int* data_parity,int frame_no,int isFile,int dataSize){
	unsigned char* frame;
	int send_unsuccess;

	// Send All-1 Frame
	while(frame_number_current<frame_number_last){

		// CREATE frame from << data_parity
		frame=iframe_new(isFile,0,frame_number_current%2,dataSize-frame_number_current*8 /*size*/,data_parity+frame_number_current);
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
	frame=iframe_new(isFile,1,frame_number_current%2,dataSize-frame_number_current*8,data_parity+frame_number_current);// text lastframe put 1
	while(send_unsuccess){
		// Send 1 frame + Start counter
		frame_sender(frame);
		printf("Sent Frame : %d\n",frame_number_current);
		startTimer(sender_timer);

		send_unsuccess=wait_ack(frame_number_current%2); // frame_number_current%2 == 0,1 << ACK 					

		// EXIT PROGRAM
	}
}

void text_sender(unsigned char* send_data){
	int* data_parity;
	int send_unsuccess;

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

