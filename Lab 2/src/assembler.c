#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int main( void )
{

	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}


	while( !EOF )
	{
	}

	fclose( fp );	

	return 0;

}
