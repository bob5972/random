#include <stdio.h>
#include <stdlib.h>

#include "random.h"

int main(int argc, char *argv[])
{
	int min, max;
	int value;
	
	//printf("argc = %d\n", argc);
	if (argc != 3) {
		printf("Usage: random min max\n");
		return 0;
	}
	
	// argv[0] is the program name
	min = atoi(argv[1]);
	max = atoi(argv[2]);
	
	//printf("min = %d\n", min);
	//printf("max = %d\n", max);
	
	if (min > max) {
		int temp = min;
		min = max;
		max = temp;
	}
	
	Random_Init();
	value = Random_Int(min, max);
	Random_Exit();
	
	printf("%d\n", value);
	
	return 0;
}

