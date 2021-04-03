// reverse01.cpp : Defines the entry point for the console application.
//

#include"stdafx.h"
#include<stdlib.h>

int main(int argc, char* argv[])
{
	unsigned int machine_code;
	scanf("%x",&machine_code);
	machine_code^=0xDEADBEEF;
	machine_code+=~0xBADC0DE;
    machine_code+=1;
    unsigned int m1=machine_code&0xFF000000;
    unsigned int m2=machine_code&0x000000FF;
    machine_code&=0x00FFFF00;
    m2+=~0x042;
	m2+=1;
	m2&=0x0FF;
	m1>>=24;
	m1+=0x057;
	m1&=0x0FF;
	m1<<=24;
   	machine_code|=m1|m2;
   	unsigned int t1=machine_code&0x00FF0000;
	unsigned int t2=machine_code&0x0000FF00;
	machine_code&=0xFF0000FF;
	t1>>=16;
	t2>>=8;
	for(int i=0;i<2;i++){
		unsigned int first_bit=t1&0x080;
		t1=((t1<<1)+(first_bit>>7))&0x0FF;
	}
	unsigned int last_bit=t2&0x01;
	t2=(t2>>1)+(last_bit<<7);
	t1<<=16;
	t2<<=8;
	machine_code|=t1|t2;
	printf("%x\n",machine_code);
	system("pause");
	return 0;
}

