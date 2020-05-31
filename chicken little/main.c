
// Dummy test file.

#include "gameplay.h"

int main()
{

	// Load an array, parse it to my array check, watch in debug the result to find out if it works.

	FILE *f;
	int array[25]; // the array
	int *result;

	f=fopen("array.dat.txt", "r");

	{
		int i=0;

		for (i=0;i!=25;i++)
			fscanf(f, "%i", &array[i]);
	}


	fclose(f);

	result = CheckMatch(5, 5, array);

	// Print matching pattern

	{
		int x, y;

		for (y=0;y!=5;y++)
		{
			for (x=0;x!=5;x++)
			{
				if (result[y*5+x] == 0)
					printf("0");
				else
					printf("1");
			}
			printf("\n");
		}

		printf("\n");
	}

	system("pause");


	return 0;
}
