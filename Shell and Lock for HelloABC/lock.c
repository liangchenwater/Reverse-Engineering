#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv, char** env)
{
	//handle with illegal input
	if(argc>3){
		printf("Please give me exactly 3 input arguments as this way: lock hello.exe hello2.exe");
		return 1;
	}
	
	//copy source hello.exe
	long int SrcEXELen=0;
	FILE *SrcEXEFp=NULL;
	SrcEXEFp = fopen(argv[1],"rb");
	fseek(SrcEXEFp,0,SEEK_END);
	SrcEXELen=ftell(SrcEXEFp);
	fseek(SrcEXEFp,0,SEEK_SET);
	unsigned char *SrcEXEBuf=(unsigned char*)malloc(SrcEXELen+1);
	fread(SrcEXEBuf,1,SrcEXELen,SrcEXEFp);
	SrcEXEBuf[SrcEXELen]=0;
	
	//shelldat.bin=shell.bin+ source hello.exe's head
	long int EXEHeadLen=(long int)((long int)SrcEXEBuf[9]*256+(long int)SrcEXEBuf[8])*16;
	long int ShellLen=0;
	FILE *ShellFp=NULL, *ShellDatFp=NULL;
	ShellFp= fopen("shell.bin", "rb");
	ShellDatFp=fopen("shelldat.bin","wb");
	fseek(ShellFp,0,SEEK_END);
	ShellLen=ftell(ShellFp);
	fseek(ShellFp,0,SEEK_SET);
	unsigned char *ShellDatBuf=(unsigned char*)malloc(ShellLen+EXEHeadLen+1);
	fread(ShellDatBuf,1,ShellLen,ShellFp);
	for(long int i=0;i<EXEHeadLen;i++) ShellDatBuf[i+ShellLen]=SrcEXEBuf[i];
	ShellDatBuf[ShellLen+EXEHeadLen]=0;
	fwrite(ShellDatBuf,1,ShellLen+EXEHeadLen,ShellDatFp);
	
	//Dest hello2.exe = Source hello.exe + shelldat.bin
	unsigned char *DestEXEBuf=(unsigned char*)malloc(SrcEXELen+ShellLen+EXEHeadLen+1);
	for(long int j=0;j<SrcEXELen;j++) DestEXEBuf[j]=SrcEXEBuf[j];
	for(long int k=0;k<ShellLen+EXEHeadLen;k++) DestEXEBuf[SrcEXELen+k]=ShellDatBuf[k];
	DestEXEBuf[SrcEXELen+ShellLen+EXEHeadLen]=0;
	//encode
	for(long int t=EXEHeadLen;t<SrcEXELen;t++) DestEXEBuf[t]^=0x33;
	//set # of relocated aims to 0
	DestEXEBuf[6]=0;
	DestEXEBuf[7]=0;
	//modify the length of Dest hello2.exe file
	long int qt=(SrcEXELen+ShellLen+EXEHeadLen)/0x200;
	long int rmd=(SrcEXELen+ShellLen+EXEHeadLen)%0x200;
	if(rmd!=0) qt++;
	DestEXEBuf[5]=qt/0x100;
	DestEXEBuf[4]=qt%0x100;
	DestEXEBuf[3]=rmd/0x100;
	DestEXEBuf[2]=rmd%0x100;
	//modify delat_cs and ip of Dest hello2.exe file
	long int cs=(SrcEXELen-EXEHeadLen)/0x10,ip=(SrcEXELen-EXEHeadLen)%0x10;
	DestEXEBuf[0x15]=ip/0x100;
	DestEXEBuf[0x14]=ip%0x100;
	DestEXEBuf[0x17]=cs/0x100;
	DestEXEBuf[0x16]=cs%0x100;
	//write Dest hello2.exe
	FILE *DestEXEFp=NULL;
	DestEXEFp=fopen(argv[2],"wb");
	fwrite(DestEXEBuf,1,SrcEXELen+ShellLen+EXEHeadLen,DestEXEFp);
	return 0;
}