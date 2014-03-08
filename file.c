//---------- File ----------
void file_sender(char* file_location);
void file_receiver(char* frame);
int file_reader(char* filename);
void file_writer(char* filename,int fileSize,short *ptr);


//------------------File part--------------------
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