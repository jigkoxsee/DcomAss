int* parityEncap(unsigned char* input,int size);
int parityChecker(int* input,int size);

//------------------------------
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

int parityChecker(int* input,int size){
    int i;
    int parity;
    for(i = 0;
        i < size;
        i++){
        if (parityGen(input[i]) != 0){
            return 0;
        }
    }
    return 1;
}
