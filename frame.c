//-------- Frame Declaration --------
typedef struct{
	int start,control,ACKorNAK,ACKno,end;

}sframe;

typedef struct{
	int start,control,last_frame,frame_no,size,data,end;

}iframe;

typedef struct{
	int i1,i2,i3;
}iframe_sender;

/*typedef struct{
	int s1;
}sframe;*/


// --------------- Frame function -----------------

/*isFile>>img=1//text=0*/

unsigned char* iframe_new_frame(int isFile,int lastframe,int frameno,int size,int* data){// text lastframe put 1
	int is3=0,is2=0;
	int asize=size;
	unsigned int a,b,i,j;
	unsigned char* tempp = malloc(12);
	iframe_sender temp;
	//tempp=new unsigned char[12];
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
		tempp[i]=(unsigned char)b;
		i++;a/=0x00000100;j-=8;
	}
	a=0xFF000000;j=24;
	while(i<=7){
		b=a;
		b&=temp.i2;
		b>>=j;
		tempp[i]=(unsigned char)b;
		i++;a/=0x00000100;j-=8;
	}
	a=0xFF000000;j=24;
	while(i<=11){
		b=a;
		b&=temp.i3;
		b>>=j;
		tempp[i]=(unsigned char)b;
		i++;a/=0x00000100;j-=8;
	}
	tempp[0]|=(1<<8);							//set start bit and control
	if(is2==1)tempp[4]|=(1<<8);
	if(is3==1)tempp[8]|=(1<<8);

	return tempp;
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
