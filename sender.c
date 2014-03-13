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

//______________ Variable Declaration ______________\\
char name[200],name2[200];

/*CONTROL	== 00  I-TEXT
			== 01  I-IMG
			== 10  S-TEXT
			== 11  S-IMG	*/
int sender_timer,startTime;
int frame_number_last,frame_number_current;
int ackR,ackS,control,dataSize;

int control_is_txt_img;// 0 = NONE, 1 = TXT, 2 = IMG

unsigned char* file_ptr; // OK

int mode,R_No=0,S_No=0;
char send_data[1000],c;
unsigned char* received_frame;
clock_t start, stop, elapsed;

//-------------- Variable Declaration --------------//


//______________ Function Declaration ______________\\
void setup_serial(void);
void send_character (int ch);
int get_character(void);

void setName2(unsigned char* nameIn);
void setTime(unsigned char* time);


void file_writer(char* filename,int fileSize,unsigned char* ptr);
int file_reader(char* filename);

unsigned int SgetStart(char frame);
unsigned int SgetControl(char frame);
unsigned int IgetIslastframe(char* frame);
unsigned int SgetEnd(char frame);
unsigned int IgetEnd(char* frame);
unsigned int IgetStart(char* frame);
unsigned int IgetControl(char* frame);
unsigned int IgetFrameno(char* frame);
unsigned int  IgetSize(char* frame);
unsigned int* Igetdata(char* frame);
unsigned char* iframe_new_frame(int isFile,int lastframe,int frameno,int size,int* data);
unsigned char* sframe_new(int isFile,int ACKorNAK,int ACKno);

int parityGen(int v);
unsigned int *parityEncap(unsigned char* input,int size);
int parityChecker(int* input,int size);

void Iframe_sender(unsigned char* data,int size,int isFile,int lastframe,int frameno);
unsigned char* Iframe_receiver();
void Sframe_sender(int isFile,int ACKno);
unsigned char** Cut8Char(unsigned char str[],int size);
void text_sender(unsigned char* send_data);
void control_sender(unsigned char** data8bit,int isFile,int dataSize);


int getTimer();
void startTimer();

//-------------- Function Declaration --------------//


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
	}
	else
		mode = 0;

	printf("Set up sender time (ms)\t: ");
	scanf("%d",&sender_timer);
	while((c= getchar()) != '\n' && c != EOF); //Flushing scanf

	printf("%s\n", mode?"Sender":"Receiver");

	//___________________Initial part___________________\\
	//Send+Receive Name & Timer
	if(mode==1){//SENDER

		// Send name
		text_sender(name);
		// Wait for name
		//setName2(Iframe_receiver());

	}else{//RECEIVER

		// Wait for name
		setName2(Iframe_receiver());
		// Send name
		//text_sender(name);

	}

	printf("Name  :%s\n", name);
	printf("Name2 :%s\n", name2);
	printf("Timer :%d\n",sender_timer );


	getch();

	//-------------------Initial part-------------------//


	/*
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
					S_Frame_receive(); // Set control_is_txt_img << in HERE
				}else{
					//--------------------------------IIIIII--------------------------------
					Iframe_receiver(); // Check control_is_txt_img before receive
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

	*/
	return 0;
}

//_____________________INIT______________________\\

void setName2(unsigned char* nameIn){
	int i=0;
	for(i=strlen(name2)-1;i>0;i--){
		name2[i]='\0';
	}
	for(i=0;i<strlen(nameIn)&&i<200;i++){
		name2[i]=nameIn[i];
	}
}

void setTime(unsigned char* time){
	sender_timer=*time;
}


//___________________File part___________________\\

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

//____________________________Frame part____________________________\\

unsigned int isIframe(unsigned char* frame){
	if(IgetStart(frame)!= 2)
		return 0;
	if(IgetControl(frame)!= 0 && IgetControl(frame)!= 1 )
		return 0;
	if(IgetEnd(frame)!= 1)
		return 0;
	return 1;
}

unsigned int isSframe(unsigned char frame){
	if(SgetStart(frame)!= 2)
		return 0;
	if(SgetControl(frame)!= 3 && SgetControl(frame)!= 2 )
		return 0;
	if(SgetEnd(frame)!= 1)
		return 0;
	return 1;
}

unsigned int SgetStart(char frame){
	unsigned char temp=0xC0;
	temp&=frame;
	temp>>=6;
	return temp;
}

unsigned int SgetControl(char frame){
	unsigned char temp=0x30;
	temp&=frame;
	temp>>=4;
	return temp;
}

unsigned int SgetACKno(char frame){ 
	unsigned char temp=0x04;
	temp&=frame;
	temp>>=2;
	return temp;
}

unsigned int IgetIslastframe(char* frame){
	unsigned char temp=0x08;
	temp&=frame[0];
	temp>>=3;
	return temp;
}

unsigned int SgetEnd(char frame){
	unsigned char temp=0x03;
	temp&=frame;
	return temp;
}

unsigned int IgetEnd(char* frame){
	unsigned char temp=0x03;
	temp&=frame[11];
	return temp;
}

unsigned int IgetStart(char* frame){
	unsigned char temp=0xC0;
	temp&=frame[0];
	temp>>=6;
	return temp;
}

unsigned int IgetControl(char* frame){
	unsigned char temp=0x30;
	temp&=frame[0];
	temp>>=4;
	return temp;
}
unsigned int IgetFrameno(char* frame){
	unsigned char temp=0x04;
	temp&=frame[0];
	temp>>=2;
	return temp;
}

unsigned int  IgetSize(char* frame){
	unsigned char temp=0x03,a=0xF8;
	temp&=frame[0];
	temp<<=5;
	a&=frame[1];
	a>>=3;
	temp+=a;
	return temp;
}

unsigned int* Igetdata(char* frame){
	unsigned int temp[9]={0,0,0,0,0,0,0,0,0};
	unsigned int a;
//0
	a=0x07;a&=frame[1];a<<=6;
	temp[0]+=a;
	a=0xFC;a&=frame[2];a>>=2;
	temp[0]+=a;

//1
	a=0x03;a&=frame[2];a<<=7;
	temp[1]+=a;
	a=0xFE;a&=frame[3];a>>=1;
	temp[1]+=a;

//2
	a=0x01;a&=frame[3];a<<=8;
	temp[2]+=a;
	a=0xFF;a&=frame[4];
	temp[2]+=a;

//3
	a=0xFF;a&=frame[5];a<<=1;
	temp[3]+=a;
	a=0x80;a&=frame[6];a>>=7;
	temp[3]+=a;

//4
	a=0x7F;a&=frame[6];a<<=2;
	temp[4]+=a;
	a=0xC0;a&=frame[7];a>>=6;
	temp[4]+=a;

//5
	a=0x3F;a&=frame[7];a<<=3;
	temp[5]+=a;
	a=0xE0;a&=frame[8];a>>=5;
	temp[5]+=a;

//6
	a=0x1F;a&=frame[8];a<<=4;
	temp[6]+=a;
	a=0xF0;a&=frame[9];a>>=4;
	temp[6]+=a;

//7
	a=0x0F;a&=frame[9];a<<=5;
	temp[7]+=a;
	a=0xF8;a&=frame[10];a>>=3;
	temp[7]+=a;

//8
	a=0x07;a&=frame[10];a<<=6;
	temp[8]+=a;
	a=0xFC;a&=frame[11];a>>=2;
	temp[8]+=a;

	return &temp;
}

unsigned char* sframe_new(int isFile,int ACKorNAK,int ACKno){
	unsigned char temp;
	temp=0;
	temp|=(1<<7)|(1<<5)|(1<<0);
	//set start control stop
	if(isFile==1)temp|=(1<<4);
	if(ACKorNAK==1)temp|=(1<<3);
	if(ACKno==1)temp|=(1<<2);
	return &temp;
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


//_______________________________Error part_______________________________\\
int parityGen(int v) { //EVEN Parity
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
return (0x6996 >> v) & 1;
}

unsigned int *parityEncap(unsigned char* input,int size){
    int i,j;
    int mask = 0x01;
    unsigned char verticalBit[8];
    unsigned char temp = 0;
    unsigned char lastParitySubFrame = 0;
    unsigned char tempLastParity = 0;
    int outputTemp = 0;
    int parityTemp = 0;
    unsigned int *output = malloc(size);
    int lastParity = 0;
    int tempRightBottomParityVertical = 0;
    int tempRightBottomParityHorizontal = 0;
    int parityLastParitySubframe = 0;
    //get Vertical bit to char 8 bit
    for (i = 0;i < 8;i++){
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
    // parity horizontal
    for (i = 0;i < size;i++){
        parityTemp = parityGen(input[i]);
        outputTemp = (int)input[i] << 1;
        outputTemp += parityTemp;
        output[i] = outputTemp;
        //for parity Right Bottom
    }
    //collect last frame parity
    for(i = 0;i < size;i++){
        tempRightBottomParityHorizontal += (output[i] & 0x01) << i;
    }
    tempRightBottomParityHorizontal = tempRightBottomParityHorizontal << 9;
    lastParity = (int)lastParitySubFrame;
    lastParity = lastParity << 1; // shift space for right bottom
    parityLastParitySubframe = tempRightBottomParityHorizontal + lastParity;
    parityLastParitySubframe = parityGen(parityLastParitySubframe);
    lastParity += parityLastParitySubframe;
    output[8] = lastParity;

    return output;
}


int parityChecker(int* input,int size){
	int i;
	for(i = 0;i < size;i++){
		if (parityGen(input[i]) != 0){
			return 0;
		}
	} return 1;
}


//___________________Frame Control part___________________\\
int wait_ack(int ackNo){
	int status;
	unsigned char ack;// = (unsigned char*)malloc(sizeof(unsigned char));
	startTimer();
	printf("Timer Start\n");
	do{
		status = inportb(LSR) & 0x01;

		//printf("getTimer = %d\n",getTimer() );

		if(getTimer()==0){ // TIME OUT -> Send new frame
			printf("TIME-OUT\n");
			return 1; // Not receive ACK
		}
	}while (status!=0x01);
	//Repeat until get Something

	ack=inportb(TXDATA);

	if(!isSframe(ack)){
		return 1;	// Fail to receive ACK
	}else{
		if(SgetACKno(ack)==ackNo)
			return 0; // Receive ACK & Send Complete
	}
	//printf("received ACK No. %d\n",ack);
}

void control_sender(unsigned char** data8bit,int isFile,int dataSize){
	unsigned char* frame;
	int i=0;
	int send_unsuccess=1;
	printf("Frame Number Last %d \n", frame_number_last);
	printf("------------------\n");

	/*printf("---DATA--\n"); 
	for(i=0;i<dataSize;i++){
		printf("Row %d : %c\n",i,data8bit[i]); 
	}*/

	// Send All-1 Frame
	while(frame_number_current<frame_number_last){

		printf("Frame Number 1 Current %d \n", frame_number_current);

		//send_unsuccess=1;
		//while(send_unsuccess){

		Iframe_sender(data8bit[frame_number_current],dataSize-frame_number_current*8,isFile,0,frame_number_current%2);

		printf("Frane Data size : %d\n",dataSize-frame_number_current*8);

		printf("Sent Frame : %d\n",frame_number_current);

			// EXIT PROGRAM
		//}
		frame_number_current++; // Next frame

	}


	printf("Transmission complete");
	getch();
/*
	// LAST FRAME;
	send_unsuccess=1;
	while(send_unsuccess){

		Iframe_sender(data8bit+frame_number_current,dataSize-frame_number_current*8,isFile,1,frame_number_current%2);

		printf("Sent Frame (Last): %d\n",frame_number_current);

		send_unsuccess=wait_ack(frame_number_current%2); // frame_number_current%2 == 0,1 << ACK 					

		// EXIT PROGRAM
	}
*/
}


void text_sender(unsigned char* send_data){
	int i=0;
	char** data8bit;


	dataSize=strlen(send_data);
	printf("SIZE : %d\n",dataSize);
	data8bit=Cut8Char(send_data,dataSize);

	for(i=0;i<dataSize;i++){
		printf("Data8bit[%d] : %c\n",i,data8bit[i]); 
	}

	frame_number_current=0;
	frame_number_last=dataSize/8;
	printf("Frame Number Last : %d\n",frame_number_last);
	if(dataSize%8!=0)
		frame_number_last++;

	control_sender(data8bit,0,dataSize);
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

void Iframe_sender(unsigned char* data,int size,int isFile,int lastframe,int frameno){

	int i;
	int send_unsuccess=1;
	unsigned int* data_parity;
	unsigned char* iframe_send;
	unsigned char* dataparity;
	unsigned char* data_exact;
	unsigned char* data_test=(unsigned char**)malloc((4)*sizeof(unsigned char*));

	data_test[0]='A';
	data_test[1]='B';
	data_test[2]='C';
	data_test[3]='D';

	data=data_test;


	printf("(start)Data Size : %d\n",size);

	if(size>8)
		while(1){
			printf("ERROR SIZE > 8");
			getch();
			exit(0);
		}
	printf("\n"); 
	for(i=0;i<size+1;i++){
		printf("Row Iframe_sender %d : %c / %x\n",i,data[i],data[i]); 
	}

	//printf("_____parity______\n"); 
	data_parity=parityEncap(data,size);

	for(i=0;i<size+1;i++){
		printf("row %d : %x\n",i,data_parity); 
	}



	//unsigned char* iframe_new_frame(int isFile,int lastframe,int frameno,int size,int* data){
	iframe_send=iframe_new_frame(isFile,lastframe,frameno,size,data_parity);

	while(send_unsuccess){

		send_character(iframe_send[0]);
		send_character(iframe_send[1]);
		send_character(iframe_send[2]);
		send_character(iframe_send[3]);
		send_character(iframe_send[4]);
		send_character(iframe_send[5]);
		send_character(iframe_send[6]);
		send_character(iframe_send[7]);
		send_character(iframe_send[8]);
		send_character(iframe_send[9]);
		send_character(iframe_send[10]);
		send_character(iframe_send[11]);

		printf("Send i-Frame finish\n");

		//send_character(s_frame);

		dataparity=Igetdata(iframe_send);
		data_exact=(unsigned char*)malloc(sizeof(unsigned char)*IgetSize(iframe_send));
		size=IgetSize(iframe_send);
		printf("Data Size : %d\n",size);

		i=0;
		while(i<=size){
			data_exact[i++]=(*dataparity)>>1;
			dataparity++;
		}

		i=0;
		while(i<size){
			printf("Dout %d : %c\n",i,data_exact[i]);
			data_exact++;
			i++;
		}

		send_unsuccess=wait_ack(frameno);
	}
	printf("Send i-Frame Complete (ACK ok)\n");

	//printf("IF : %x\n",iframe_send[0]);
	//printf("IF : %x\n",iframe_send[1]);
	//printf("IF : %x\n",iframe_send[2]);
	//printf("IF : %x\n",iframe_send[3]);
	//printf("IF : %x\n",iframe_send[4]);
	//printf("IF : %x\n",iframe_send[5]);
	//printf("IF : %x\n",iframe_send[6]);
	//printf("IF : %x\n",iframe_send[7]);
	//printf("IF : %x\n",iframe_send[8]);
	//printf("IF : %x\n",iframe_send[9]);
	//printf("IF : %x\n",iframe_send[10]);
	//printf("IF : %x\n",iframe_send[11]);
	//printf("I>S : %d\n ",IgetStart(iframe_send));
	//printf("I>C : %d\n ",IgetControl(iframe_send));
	//printf("I>F : %d\n ",IgetFrameno(iframe_send));
	//printf("I>Z : %d\n ",IgetSize(iframe_send));


	// Wait for ACK
	//unsigned char* sframe_new(int isFile,int ACKorNAK,int ACKno);
}

unsigned char* Iframe_receiver(){
	int roundNo,i,size;
	char ch;
	unsigned int* dataparity;
	unsigned char* data;
	unsigned char* datain = (unsigned char*)malloc(sizeof(unsigned char)*12);
	unsigned char* sframe;

	//roundNo=12;
	//printf("roundNo:%d\n",roundNo);

	i=0;
	while(i<12){
		printf("I : %d",i);
		ch=get_character();
		datain[i++]=ch;
		printf(">> %x\n",datain[i-1]);
	}
	puts("RECEIVED\n");

	size=IgetSize(datain);
	printf("I>S : %d\n ",IgetStart(datain));
	printf("I>C : %d\n ",IgetControl(datain));
	printf("I>F : %d\n ",IgetFrameno(datain));
	printf("I>Z : %d\n ",size);

	dataparity=Igetdata(datain);

	///////////// Parity Checker \\\\\\\\\\\\\\\\\\\\\
	//printf("Parity Check %s\n", parityChecker(dataparity,size)); //parityChecker(dataparity,size)==1?"TRUE":"FALSE"

	data=(unsigned char*)malloc(sizeof(unsigned char)*IgetSize(datain));

	// Decap Data form 2D-Parity
	i=0;
	while(i<size){
		data[i]=(dataparity[i])>>1;
		i++;
		//dataparity++;
	}

	i=0;
	while(i<size){
		printf("D: %c 0x%x\n",data[i],data[i]);
		//data++;
		i++;
	}

	Sframe_sender(IgetControl(datain)%2,IgetFrameno(datain));

	return data; // User strlen(data) to find it Size.
}

void control_receiver(){
	unsigned char* data_receive;

	while(1){
		Iframe_receiver();
	}
}

void Sframe_sender(int isFile,int ACKno){
	int send_unsuccess=1;
	//unsigned char* sframe_new(int isFile,int ACKorNAK,int ACKno);
	unsigned char* s_frame=sframe_new(isFile,1,ACKno);

	while(send_unsuccess){
		send_character(*s_frame);
	}
}



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

//TIME
void startTimer(){
	start = clock();
}

int getTimer(){
	stop = clock();
	elapsed = 10000 * (stop - start);// / (CLOCKS_PER_SEC);

	//printf("Start %d / elapsed %d \n", start,elapsed);
	return elapsed-sender_timer;
}