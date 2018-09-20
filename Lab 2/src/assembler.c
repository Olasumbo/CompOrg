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
      
uint32_t encode_rtype( FILE * fp, uint32_t opcode, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt, uint32_t funct )
{

    rs = rs << 21;
    rt = rt << 16;
    rd = rd << 11;    
    shamt = shamt << 6;  
    
    printf( "\nR-TYPE: %x, %x, %x, %x, %x, %x \n", opcode, rs, rt, rd, shamt, funct );
    
    return ( opcode + rs + rt + rd + shamt + funct );             
}
             
uint32_t encode_rtype_mult( FILE * fp, uint32_t opcode, uint32_t shamt, uint32_t funct )
{
    uint32_t rd = 0, rs = 0, rt = 0;
    char rs_s[32];
    char rt_s[32];
    
    getArg( rs_s, fp );      
    getArg( rt_s, fp );      
              
    rs = getRegister( rs_s );
    rt = getRegister( rt_s );    
      
    return encode_rtype( fp, opcode, rs, rt, rd, shamt, funct );   
}
             
uint32_t encode_rtype_move( FILE * fp, uint32_t opcode, uint32_t funct )
{
    uint32_t rd = 0, rs = 0, rt = 0, shamt = 0;
    char rd_s[32];
    
    getArg( rd_s, fp );      
              
    rd = getRegister( rd_s );    
      
    return encode_rtype( fp, opcode, rs, rt, rd, shamt, funct );   
}
             
uint32_t encode_rtype_shift( FILE * fp, uint32_t opcode, uint32_t funct )
{
    uint32_t rd = 0, rs = 0, rt = 0, shamt = 0;
    char rs_s[32];
    char rd_s[32];
    char shamt_s[32];
    
    getArg( rs_s, fp );      
    getArg( rd_s, fp );    
    getArg( shamt_s, fp );          
              
    rs = getRegister( rs_s );
    rd = getRegister( rd_s );    
      
    return encode_rtype( fp, opcode, rs, rt, rd, shamt, funct );   
}

uint32_t encode_rtype_generic( FILE * fp, uint32_t opcode, uint32_t shamt, uint32_t funct )
{
    uint32_t rd = 0, rs = 0, rt = 0;
    char rd_s[32];
    char rs_s[32];
    char rt_s[32];
    
    getArg( rs_s, fp );      
    getArg( rt_s, fp );      
    getArg( rd_s, fp );      
              
    rs = getRegister( rs_s );
    rt = getRegister( rt_s );
    rd = getRegister( rd_s );
    
    return encode_rtype( fp,opcode, rs, rt, rd, shamt, funct );             
}

uint32_t encode_itype( FILE * fp, uint32_t opcode, uint32_t rs, uint32_t rt, uint32_t im )
{
    rs = rs << 21;
    rt = rt << 16;  
    
    printf( "\nI-TYPE: %x, %x, %x, %x \n", opcode, rs, rt, im );
    
    return ( opcode + rs + rt + im );   
}

uint32_t encode_itype_branch( FILE * fp, uint32_t opcode, uint32_t bgez )
{
    uint32_t rs = 0, rt = 0, im = 0;
    char rs_s[32];
    char im_s[32];
        
    getArg( rs_s, fp );         
    getArg( im_s, fp );  
        
    rt = bgez;
    rs = getRegister( rs_s );
    im = (uint32_t) strtol( im_s, NULL, 16 );

    rs = rs << 21;
    rt = rt << 16;  
    
    printf( "\nI-TYPE(branch): %x, %x, %x, %x \n", opcode, rs, rt, im );
    
    return encode_itype( fp, opcode, rs, rt, im );   
}

uint32_t encode_itype_loadim( FILE * fp, uint32_t opcode )
{
    uint32_t rs = 0, rt = 0, im = 0;
    char rt_s[32];
    char im_s[32];
        
    getArg( rt_s, fp );         
    getArg( im_s, fp );  
        
    rt = getRegister( rt_s );
    im = (uint32_t) strtol( im_s, NULL, 16 );

    rs = rs << 21;
    rt = rt << 16;  
    
    printf( "\nI-TYPE(LoadIm): %x, %x, %x, %x \n", opcode, rs, rt, im );
    
    return encode_itype( fp, opcode, rs, rt, im );   
}

uint32_t encode_itype_generic( FILE * fp, uint32_t opcode )
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
    
    return encode_itype( fp, opcode, rs, rt, im );   
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
		printf("Error: Can't open program file %s\n", prog_file );
		exit(-1);
	}
  
  /*Open writing file */
	fw = fopen( "instruction.in" , "w");
	if (fp == NULL) {
		printf("Error: Can't open writing file file %s\n", prog_file );
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
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
    if( strcmp( data, "addu" ) == 0 )
    {             
      opcode = 0x00000000;   
      shamt = 0x00000000;
      funct = 0x00000021;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }   
    else if( strcmp( data, "addi" ) == 0 )
    {         
      opcode = 0x20000000;
      ins = encode_itype_generic( fp, opcode );
    }
    else if( strcmp( data, "addiu" ) == 0 )
    {         
      opcode = 0x24000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "sub" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000022;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "subu" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000023;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "mult" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000028;
      
      ins = encode_rtype_mult( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "multu" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000029;
      ins = encode_rtype_mult( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "div" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x0000002A;
      ins = encode_rtype_mult( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "divu" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x0000002B;
      ins = encode_rtype_mult( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "and" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000023;
      ins = encode_rtype_generic( fp, opcode, shamt, funct);
    }
    else if( strcmp( data, "andi" ) == 0 )
    {         
      opcode = 0x30000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "or" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000025;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "ori" ) == 0 )
    {         
      opcode = 0x34000000;
      ins = encode_itype_generic( fp, opcode );
    } 
    else if( strcmp( data, "xor" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000026;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "xori" ) == 0 )
    {         
      opcode = 0x38000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "nor" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000027;
      ins = encode_rtype_generic( fp, opcode, shamt, funct);
    }
    else if( strcmp( data, "slt" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x0000002A;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
    else if( strcmp( data, "slti" ) == 0 )
    {         
      opcode = 0x28000000;
      ins = encode_itype_generic( fp, opcode );
    }     
    else if( strcmp( data, "sll" ) == 0 )
    {
      opcode = 0x00000000;
      funct = 0x00000000;
      ins = encode_rtype_shift( fp, opcode, funct );
    }
    else if( strcmp( data, "srl" ) == 0 )
    {
      opcode = 0x00000000;
      funct = 0x00000002;
      ins = encode_rtype_shift( fp, opcode, funct );
    }
    else if( strcmp( data, "sra" ) == 0 )
    {
      opcode = 0x00000000;
      funct = 0x00000003;
      ins = encode_rtype_shift( fp, opcode, funct );
    }
    else if( strcmp( data, "lw" ) == 0 )
    {         
      opcode = 0x8C000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "lb" ) == 0 )
    {         
      opcode = 0x80000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "lh" ) == 0 )
    {         
      opcode = 0x84000000;
      ins = encode_itype_loadim( fp, opcode );
    }  
    else if( strcmp( data, "sw" ) == 0 )
    {         
      opcode = 0xAC000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "sb" ) == 0 )
    {         
      opcode = 0xA0000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "sh" ) == 0 )
    {         
      opcode = 0xA4000000;
      ins = encode_itype_generic( fp, opcode );
    }  
    else if( strcmp( data, "mfhi" ) == 0 )
    {
      opcode = 0x00000000;
      funct = 0x00000002;
      ins = encode_rtype_move( fp, opcode, funct );
    }
else if( strcmp( data, "jalr" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000009;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
else if( strcmp( data, "jal" ) == 0 )
    {
      opcode = 0x0C000000;
      ins = encode_jtype( fp, opcode );
    }
else if( strcmp( data, "jr" ) == 0 )
    {
      opcode = 0x00000000;
      shamt = 0x00000000;
      funct = 0x00000008;
      ins = encode_rtype_generic( fp, opcode, shamt, funct );
    }
else if( strcmp( data, "j" ) == 0 )
    {
      opcode = 0x04000000;
      ins = encode_jtype( fp, opcode );
    }
else if( strcmp( data, "beq" ) == 0 )
    {
      opcode = 0x10000000;
      ins = encode_itype_generic( fp, opcode );
    }
else if( strcmp( data, "bne" ) == 0 )
    {
      opcode = 0x14000000;
      ins = encode_itype_generic( fp, opcode );
    }
else if( strcmp( data, "blez" ) == 0 )
    {
	//special case
      opcode = 0x04000000;
      ins = encode_itype_branch( fp, opcode, 0x0 );
    }
else if( strcmp( data, "bltz" ) == 0 )
    {
	//special case
      opcode = 0x18000000;
	    uint32_t rt = 0x0; 
      ins = encode_itype_branch( fp, opcode, rt );
    }  
else if( strcmp( data, "bgez" ) == 0 )
    {
	//special case
      opcode = 0x04000000;
      uint32_t rt = 0x1;
      ins = encode_itype_branch( fp, opcode, rt );
    }
else if( strcmp( data, "bgtz" ) == 0 )
    {
        opcode = 0x1C000000;
  	    uint32_t rt = 0x0;
        ins = encode_itype_branch( fp, opcode, rt );
    }
    else if( strcmp( data, "mflo" ) == 0 )
    {
        opcode = 0x04000000;
        ins = encode_itype_generic( fp, opcode );
    }
    else if( strcmp( data, "syscall" ) == 0 )
    {
        ins = 0xC;
    }
    
    printf( "INSTRUCTION: %x\n", ins);
    fprintf( fw, "%x\n", ins );
      
	}

	fclose( fw );	
	fclose( fp );	

	return 0;

}
