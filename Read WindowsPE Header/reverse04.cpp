#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

long convert_to_long(unsigned char* buf)
{
	long res=0;
	for(int i=3;i>=0;i--){
		res*=256;
		res+=buf[i];
	}
	return res;
}

int main()
{
	/*open PE file*/
	FILE *fp=NULL;
	char* peName=NULL;
	long nameSize;
	printf("If the length of PE's name is larger than 100, please input the an upper bound of the length. Otherwise, please input 0, and the upper bound is set to 100 by default.\n");
	scanf("%d",&nameSize);
	if(nameSize<=100) {
		peName=(char*)malloc(102*sizeof(char));
		nameSize=100;
	}
	else peName=(char*)malloc((nameSize+2)*sizeof(char));
	printf("Please input filename.\n");
	getchar();
	fgets(peName,nameSize+2,stdin);
	peName[strlen(peName)-1]=0;
	fp=fopen(peName,"rb");
	assert(fp);
	if(peName){
		free(peName);
		peName=NULL;
	}
	
	/*find base address of PE head in file*/
	unsigned char temp[5];
	memset(temp,0,sizeof(temp));
	fseek(fp,0x3C,SEEK_SET);
	fread(temp,1,4,fp);
	long peBaseAddr=convert_to_long(temp);
	fseek(fp,peBaseAddr,SEEK_SET);
	memset(temp,0,sizeof(temp));
	fread(temp,1,2,fp);
	if(temp[0]!='P'||temp[1]!='E') {
		printf("This is not a PE file!\n");
		return 1;
	}
	
	/*find address of Import Table in file*/
	/*find amount of sections*/
	fseek(fp,6+peBaseAddr,SEEK_SET);
	fread(temp,1,2,fp);
	unsigned int segCount=(unsigned int)convert_to_long(temp);
	/*find address of Import Table in memory and the length of Import Table*/
	fseek(fp,0x80+peBaseAddr,SEEK_SET);
	fread(temp,1,4,fp);
	long tableOffset=convert_to_long(temp);
	fread(temp,1,4,fp);
	long tableLength=convert_to_long(temp);
	if(tableLength==0) {
		printf("No Import table!\n"); 
		return 1; 
	}
	assert(tableOffset!=0);
	unsigned char segDescp[0x29];
	/*find the descriptions of sections*/
	fseek(fp,0xF8+peBaseAddr,SEEK_SET);
	/*finally fix which section Import Table is in and find address of Import Table in file*/
	long tableFileOffset=0;
	for(unsigned int i=0;i<segCount;i++){
		fread(segDescp,1,0x28,fp);
		long segOffset=convert_to_long(segDescp+0xC);
		long segLength=convert_to_long(segDescp+8);
		long segFileOffset=convert_to_long(segDescp+0x14);
		if(tableOffset>=segOffset&&tableOffset<segOffset+segLength){
			tableFileOffset=segFileOffset+tableOffset-segOffset;
			break;
		}
	}
	
	/*output information in Import Table*/
	FILE* out=NULL;
	out=fopen("record.txt","w");
	unsigned char impDescp[0x15];
	long cnt=0;
	char ch=0;
	fseek(fp,tableFileOffset+cnt*0x14,SEEK_SET);
	fread(impDescp,1,0x15,fp);
	int is_first=1;
	while(convert_to_long(impDescp)!=0){
		if(is_first) is_first=0;
		else{
		putchar('\n');
		fputc('\n',out);
		}
		long APITableOffset=convert_to_long(impDescp); //address of APITable in memory
		long ddlNameOffset=convert_to_long(impDescp+0xC); //address of ddlName in memory
		assert(APITableOffset!=0);
		assert(ddlNameOffset!=0);
		/*output ddlName*/
		fseek(fp,tableFileOffset+ddlNameOffset-tableOffset,SEEK_SET);
		for(;;){
		ch=fgetc(fp);
		if(ch==0) break;
		putchar(ch);
		fputc(ch,out);
		}
		printf(":\n");
		fprintf(out,":\n");
		/*output API Name or API Order*/
		int ccnt=0;
		fseek(fp,tableFileOffset+APITableOffset-tableOffset+ccnt*4,SEEK_SET);
		fread(temp,1,4,fp);
		long APINameOffset=convert_to_long(temp);
		while(APINameOffset!=0){
			/*output API Name*/
			if(APINameOffset>0){
			fseek(fp,tableFileOffset+APINameOffset-tableOffset,SEEK_SET);
			ch=fgetc(fp);
			ch=fgetc(fp);
			for(;;){
			ch=fgetc(fp);
			if(ch==0) break;
			putchar(ch);
			fputc(ch,out);
			}
			putchar('\n');
			fputc('\n',out);
		}
			/*output API order*/
			else{
				temp[3]&=0x7F;
				printf("%08x\n",convert_to_long(temp));
				fprintf(out,"%08x\n",convert_to_long(temp));
			}
			ccnt++;
			fseek(fp,tableFileOffset+APITableOffset-tableOffset+ccnt*4,SEEK_SET);
			fread(temp,1,4,fp);
			APINameOffset=convert_to_long(temp);
		}
		cnt++;
		fseek(fp,tableFileOffset+cnt*0x14,SEEK_SET);
		fread(impDescp,1,0x15,fp);
	}
	fclose(fp);
	fclose(out);
	system("pause");
	return 0;
}