#include <iostream>
#include "../UGen/UGen.h"

#define BLOCKSIZE 128

/**
 A simple console application demonstrating UGen in the console.
 */
int main (int argc, char * const argv[]) 
{
	// this initialises the UGen system, it should be called at the start of your app.
	UGen::initialise();
	
	// before processing any audio provide a sample rate, blocksize and (optionally) a control-rate blocksize
	UGen::prepareToPlay(44100, BLOCKSIZE, 64);
	
	// an array of floats to write our output data to
	float output[BLOCKSIZE];
	
	// create a UGen graph to play..
	UGen test = SinOsc::AR(100, 0, 1.0);	// 100Hz sine
	
	float previous = 0.f;
	
	// process 100 blocks..
	for(int j = 0; j < 100; j++)
	{
		// obtain a new blockID before commencing the processing of each block
		int blockID = UGen::getNextBlockID(BLOCKSIZE);
		
		// tell our UGen graph where to write its data
		test.setOutput(output, BLOCKSIZE, 0);
		
		// prepare, and process the block
		test.prepareAndProcessBlock(BLOCKSIZE, blockID, 0);
				
		// let's see what's been written to our output[] array..
		for(int i = 0; i < BLOCKSIZE; i++)
		{
			printf("block[%4d] output[%4d]  ", j, i);
			
			if(fabs(output[i] - previous) < 0.01)
			{
				int spaces = output[i] * 30 + 30;
				while(spaces--) printf(" ");
				printf("|");
			}
			else if(fabs(output[i] - previous) > 0.2)
			{
				int spaces = min(output[i], previous) * 30 + 30;
				while(spaces--) printf(" ");
				int dashes = fabs(output[i] - previous) * 30;
				while(dashes--) printf("-");
			}
			else if(output[i] > previous)
			{
				int spaces = output[i] * 30 + 30;
				while(spaces--) printf(" ");
				printf("\\");
			}
			else
			{
				int spaces = output[i] * 30 + 30;
				while(spaces--) printf(" ");
				printf("/");
			}
			
			printf("\n");
			
			previous = output[i];
		}
	}
	
	// shutdown the UGen system
	UGen::shutdown();
    return 0;
}

