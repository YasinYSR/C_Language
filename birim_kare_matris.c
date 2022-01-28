#include <stdio.h>
#include <conio.h>

int main()
{
	int i,j,n;
	printf("Kare matris buyuklugunu giriniz	:");
	scanf("%d",&n);
	int sqr[n][n];
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			if(i==j)
			{
				sqr[i][j]=1;
			}
			if(i!=j)
			{
				sqr[i][j]=0;
			}
		}
	}
	
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			printf("%d",sqr[i][j]);
		}
		printf("\n");
	}
}