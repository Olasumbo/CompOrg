#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


uint32_t getRegister( char * ins )
{

  if( strcmp( "$zero", ins ) == 0 )
  {
    return 0x00;
  }
  else if( strcmp( "$at", ins ) == 0 )
  {
    return 0x01;
  }
  else if( strcmp( "$v0", ins ) == 0 )
  {
    return 0x02;
  }
  else if( strcmp( "$v1", ins ) == 0 )
  {
    return 0x03;
  }
  else if( strcmp( "$a0", ins ) == 0 )
  {
    return 0x04;
  }
  else if( strcmp( "$a1", ins ) == 0 )
  {
    return 0x05;
  }
  else if( strcmp( "$a2", ins ) == 0 )
  {
    return 0x06;
  }
  else if( strcmp( "$a3", ins ) == 0 )
  {
    return 0x07;
  }
  else if( strcmp( "$t0", ins ) == 0 )
  {
    return 0x08;
  }
  else if( strcmp( "$t1", ins ) == 0 )
  {
    return 0x09;
  }
  else if( strcmp( "$t2", ins ) == 0 )
  {
    return 0x0A;
  }
  else if( strcmp( "$t3", ins ) == 0 )
  {
    return 0x0B;
  }
  else if( strcmp( "$t4", ins ) == 0 )
  {
    return 0x0C;
  }
  else if( strcmp( "$t5", ins ) == 0 )
  {
    return 0x0D;
  }
  else if( strcmp( "$t6", ins ) == 0 )
  {
    return 0x0E;
  }
  else if( strcmp( "$t7", ins ) == 0 )
  {
    return 0x0F;
  }
  else if( strcmp( "$s0", ins ) == 0 )
  {
    return 0x10;
  }
  else if( strcmp( "$s1", ins ) == 0 )
  {
    return 0x11;
  }
  else if( strcmp( "$s2", ins ) == 0 )
  {
    return 0x12;
  }
  else if( strcmp( "$s3", ins ) == 0 )
  {
    return 0x13;
  }
  else if( strcmp( "$s4", ins ) == 0 )
  {
    return 0x14;
  }
  else if( strcmp( "$s5", ins ) == 0 )
  {
    return 0x15;
  }
  else if( strcmp( "$s6", ins ) == 0 )
  {
    return 0x16;
  }
  else if( strcmp( "$s7", ins ) == 0 )
  {
    return 0x17;
  }
  else if( strcmp( "$t8", ins ) == 0 )
  {
    return 0x18;
  }
  else if( strcmp( "$t9", ins ) == 0 )
  {
    return 0x19;        
  }
  else if( strcmp( "$k0", ins ) == 0 )
  {
    return 0x1A;
  }
  else if( strcmp( "$k1", ins ) == 0 )
  {
    return 0x1B;
  }
  else if( strcmp( "$gp", ins ) == 0 )
  {
    return 0x1C;
  }
  else if( strcmp( "$sp", ins ) == 0 )
  {
    return 0x1D;
  }
  else if( strcmp( "$fp", ins ) == 0 )
  {
    return 0x1E;
  }
  else if( strcmp( "$ra", ins ) == 0 )
  {
    return 0x1F;
  }
  
  return 0;

}

void getArg( char * rtn, FILE * fp )
{                           
    fscanf( fp, "%s", rtn );
    if( rtn[ strlen(rtn) - 1] == ',' )
    {
        rtn[ strlen(rtn) - 1] = '\0';
    }    
    
    //TEST
    //printf( "\n-> %s", rtn );
    
    return;                
}

uint32_t encode_rtype( FILE * fp, uint32_t opcode, uint32_t shamt, uint32_t funct, int zero_rd )
{
    uint32_t rd = 0, rs = 0, rt = 0;
    char rd_s[32];
    char rs_s[32];
    char rt_s[32];
    
    getArg( rt_s, fp );
    getArg( rs_s, fp );      
          
    rt = getRegister( rt_s );
    rs = getRegister( rs_s );
    
    if( zero_rd == 0 )
    { 
      getArg( rd_s, fp );     
      rd = getRegister( rd_s );
    }
    else
    {
      rd = 0;
    }

    rs = rs << 21;
    rt = rt << 16;
    rd = rd << 11;    
    shamt = shamt << 6;  
    
    printf( "\nR-TYPE: %x, %x, %x, %x, %x, %x \n", opcode, rs, rt, rd, shamt, funct );
    
    return ( opcode + rs + rt + rd + shamt + funct );             
}

uint32_t encode_itype( FILE * fp, uint32_t opcode )
{
    uint32_t rs = 0, rt = 0, im = 0;
    char rs_s[32];
    char rt_s[32];
    char im_s[32];
    
    getArg( rt_s, fp );      
    getArg( rs_s, fp );         
    getArg( im_s, fp );  
        
    rt = getRegister( rt_s );
    rs = getRegister( rs_s );
    im = (uint32_t) strtol( im_s, NULL, 16 );

    rs = rs << 21;
    rt = rt << 16;  
    
    printf( "\nI-TYPE: %x, %x, %x, %x \n", opcode, rs, rt, im );
    
    return ( opcode + rs + rt + im );   
}

uint32_t encode_jtype( FILE * fp, uint32_t opcode )
{
    uint32_t target = 0;
    char target_s[32];
    
    getArg( target_s, fp );    
    
    printf( "\nJ-TYPE: %x, %x\n", opcode, target );
    
    return ( opcode + target );   
}

int main(int argc, char *argv[]) 
{                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS ASSEMBLER...\n");
	printf("**************************\n\n");

	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}
                        
  char prog_file[32];
	strcpy(prog_file, argv[1]);

	FILE * fp, * fw; 

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}
  
  /*Open writing file */
	fw = fopen( "instruction.txt" , "w");
	if (fp == NULL) {
		printf("Error: Can't open writing file file %s\n", prog_file);
		exit(-1);
	}

  char data[32];
  uint32_t ins = 0x0;

	while( fscanf( fp, "%s", data) != EOF )
	{
    printf( "\n%s", data );
    uint32_t opcode = 0x0, shamt = 0x0, funct = 0x0;
    
    if( strcmp( data, "add" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000020;
      ins = encode_rtype( fp, opcode, shamt, funct, 0 );
    }
    if( strcmp( data, "addu" ) == 0 )
    {             
      opcode = 0x00000000;   
      shamt = 0x00000000;
      funct = 0x00000021;
      ins = encode_rtype( fp, opcode, shamt, funct, 0 );
    }   
    else if( strcmp( data, "addi" ) == 0 )
    {         
      opcode = 0x20000000;
      ins = encode_itype( fp, opcode );
    }
    else if( strcmp( data, "addiu" ) == 0 )
    {         
      opcode = 0x24000000;
      ins = encode_itype( fp, opcode );
    }  
    else if( strcmp( data, "sub" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000022;
      ins = encode_rtype( fp, opcode, shamt, funct, 0 );
    }
    else if( strcmp( data, "subu" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000023;
      ins = encode_rtype( fp, opcode, shamt, funct, 0 );
    }
    else if( strcmp( data, "mult" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x0000002;
      ins = encode_rtype( fp, opcode, shamt, funct, 1 );
    }
    
    printf( "INSTRUCTION: %x\n", ins);
    fprintf( fw, "%x\n", ins );
      
	}

	fclose( fw );	
	fclose( fp );	

	return 0;

}
