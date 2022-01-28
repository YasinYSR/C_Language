#include <stdio.h>
#include <string.h>

void main()
{
	char pass[20];
	printf("Sifrelenecek degeri giriniz: \n");
	scanf("%s",pass);
	code(pass,0xFACA);
	printf("Sifrelenmis değer: %s\n",pass);
	decode(pass,0xFACA);
	printf("Sifrenin çözülmüş hali: %s\n",pass);
	return 0;
}

void code(char pass[],int key)
{
	unsigned int i;
	for(i=0;i<=strlen(pass);++i)
	{
		pass[i]=pass[i]-key;
	}
}

void decode(char pass[],int key)
{
	unsigned int i;
	for(i=0;i<=strlen(pass);++i)
	{
		pass[i]=pass[i]+key;
	}
}