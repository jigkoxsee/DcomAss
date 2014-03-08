#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

unsigned char *buffer;
int parityGen(int v) { //EVEN Parity
	v ^= v >> 16;
	v ^= v >> 8;
	v ^= v >> 4;
	v &= 0xf;
	return (0x6996 >> v) & 1;
}

unsigned char *int2bin(int a,unsigned char *buffer, int buf_size) {
    int i;
    buffer += (buf_size - 1);

    for (i = 31; i >= 0; i--) {
        *buffer-- = (a & 1) + '0'; // get only last bit change it to charector
        a >>= 1;
    }
    return buffer;
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
    tempRightBottomParity += lastParitySubFrame;
    // parity horizontal
    for (i = 0;i < size;i++){
    	parityTemp = parityGen(input[i]);
    	outputTemp = (int)input[i] << 1;
    	outputTemp += parityTemp;
    	output[i] = outputTemp;
        //for parity Right Bottom
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

int parityChecker(int* input,int size){ int i;
	int temp;
	for(i = 0;i < size;i++){
		temp += input[i];
	}
	if (temp % 2 != 0){ 
		return 0;
	}
	return 1;
}

void printBin(int input) {
    buffer[16] = '\0';
    int2bin(input, buffer,16);
    printf("%s", buffer);
    printf("\n");
}

int main() {
	//unsigned char ch[]="HELLO";
	unsigned char ch[5];
	int* ch2;
	int size=5;

	ch[0]=getche();
	ch[1]=getche();
	ch[2]=getche();
	ch[3]=getche();
	ch[4]=getche();

	printf("\nCH = %s\n",ch);

	ch2=parityEncap(ch,size);

	printf("%x >> %x\n",ch[0],ch2[0]);
	printf("%x >> %x\n",ch[1],ch2[1]);
	printf("%x >> %x\n",ch[2],ch2[2]);
	printf("%x >> %x\n",ch[3],ch2[3]);
	printf("%x >> %x\n",ch[4],ch2[4]);
	printf("   >> %x\n",ch2[5]);
	printf("   >> %x\n",ch2[6]);

	getch();
}
