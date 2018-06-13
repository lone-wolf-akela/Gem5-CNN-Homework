#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <stdint.h>

#include "peripheral.h"
#include "output_label.h"

#define BATCH 100

int main()
{
	int count,pic,i,j,index,correct = 0;
	volatile uint8_t * data;
	int finalresult[BATCH];
	printf("Program Start.\n");

	data = (uint8_t*) mmap(PERI_ADDR[0], 1+sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	printf("Peripherals Registered.\n");

	for(count = 0; count < BATCH; count++)
	{
		periInit(data);         
		printf("Inited.\n");
		periRead(data, finalresult + count, sizeof(int));
		printf("Got result: %d\n", finalresult[count]);
	}

	periLogout(0);
	for(i = 0; i < BATCH; i++)
	{
		if(finalresult[i] == target_list[i])
			correct++;
	}
	printf("accu: %f\n", (double)correct / (double)BATCH);

	return 0;
}
