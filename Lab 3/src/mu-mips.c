#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

/***************************************************************/
/* Extend Sign Function from 16-bits to 32-bits */
/***************************************************************/
uint32_t extend_sign( uint32_t im )
{
	uint32_t data = ( im & 0x0000FFFF );
	uint32_t mask = 0x00008000;
	if ( mask & data ) 
	{
		data = data | 0xFFFF0000;
	}

	return data;
}      


/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	uint32_t rt = ( 0x001F0000 & MEM_WB.IR  ) >> 16;
	//rd MASK: 4x0000 1111 1000 0000 0000 = 0000F800;
	uint32_t rd = ( 0x0000F800 & MEM_WB.IR  ) >> 11;

	if(MEM_WB.type == 0)
	{
		NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
	}
	else if(MEM_WB.type == 1)
	{
		NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
	}
	else if(MEM_WB.type == 2 )
	{
		NEXT_STATE.REGS[rt] = MEM_WB.LMD;
	}
	else if( MEM_WB.type == 4)
	{
		NEXT_STATE.REGS[0] = 0XA;
		RUN_FLAG = FALSE;
	}
  	++INSTRUCTION_COUNT;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	MEM_WB.IR = EX_MEM.IR;
	MEM_WB.type = EX_MEM.type;

	if(EX_MEM.type <= 1)		//0 reg-reg, 1 reg-imm
	{
		MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	}
	else if(EX_MEM.type == 2)	//2 is Load
	{
		MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);
	}
	else if(EX_MEM.type == 3)	//3 is store
	{
		mem_write_32(EX_MEM.ALUOutput, EX_MEM.B);
	}

}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{

  //Load INstruction from Buffer
  EX_MEM.IR = ID_EX.IR;

	//opcode MASK: 1111 1100 0000 0000 4x0000 = FC0000000
	uint32_t oc = ( 0xFC000000 & EX_MEM.IR  );
  	uint32_t imm = ID_EX.imm;
	uint32_t func = (EX_MEM.IR & 0x0000003F);

	//Handle each case for instructions
	switch( oc )
	{
		//R-Type
		case 0x00000000: 
			{  
				EX_MEM.type = 0;                              

				switch( func ) 
				{
					case 0x00000020: 
						//ADD
						puts( "Add Function" );
						EX_MEM.ALUOutput = ID_EX.A + ID_EX.B;
						break; 

					case 0x00000021:
						//ADDU
						puts( "Add Unsigned Function" );
						EX_MEM.ALUOutput = ID_EX.A + ID_EX.B;
						break; 

					case 0x00000022:
						//SUB
						puts( "Subtract Function" );
						EX_MEM.ALUOutput = ID_EX.A - ID_EX.B;
						break;

					case 0x00000023:
						//SUBU
						puts( "Subtract Unsigned Function" );
						EX_MEM.ALUOutput = ID_EX.A - ID_EX.B;
						break;

					case 0x00000018:
					{
						//MULT
						puts( "Multiply Function" );
						EX_MEM.ALUOutput = ID_EX.A * ID_EX.B;
						break;
					}

					case 0x00000019:
					{
						//MULTU
						puts( "Multiply Unsigned Function" );
						EX_MEM.ALUOutput = ID_EX.A * ID_EX.B;
						break;
					}

					case 0x0000001A:
						//DIV
						puts( "Divide Function" );
						EX_MEM.ALUOutput = ID_EX.A / ID_EX.B;
						break;

					case 0x0000001B:
						//DIVU
						puts( "Divide Unsigned Function" );
						EX_MEM.ALUOutput = ID_EX.A / ID_EX.B;
						break;

					case 0X00000024:
						//AND
						EX_MEM.ALUOutput = ID_EX.A & ID_EX.B;
						break; 

					case 0X00000025:
						//OR
						EX_MEM.ALUOutput = ID_EX.A | ID_EX.B;
						break; 
					
					case 0X00000026:
						//XOR
						EX_MEM.ALUOutput = ID_EX.A ^ ID_EX.B;
						break;

					case 0x00000027:
						//NOR
						EX_MEM.ALUOutput = ~( ID_EX.A | ID_EX.B );
						break;

					case 0x0000002A:
						//SLT
						if( ID_EX.A < ID_EX.B )
							EX_MEM.ALUOutput = 0x00000001;
						else
							EX_MEM.ALUOutput = 0x00000000;
						break;

					case 0x00000000:
					{
						//SLL
            					uint32_t sa = imm >> 6;
						EX_MEM.ALUOutput = ID_EX.B << sa;
						break;
					}

					case 0x00000002:
					{
						//SRL
            					uint32_t sa = imm >> 6;
						EX_MEM.ALUOutput = ID_EX.B >> sa;
						break;
					}			

					case 0x00000003:
					{
						//SRA
            					uint32_t sa = imm >> 6;
						EX_MEM.ALUOutput = extend_sign( ID_EX.B >> sa );
						break;
					}
					case 0x0000000C:
						//SYSCALL - System Call, exit the program.
						//EX_MEM.ALUOutput = 0xA;
						EX_MEM.type = 4;
						break; 

					case 0x00000013:
						//MTLO
						puts( "Move to LO" );
						EX_MEM.LO = ID_EX.A;
						//NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
						break;

					case 0x0000011:
						//MTHI
						puts( "Move to HI" );
						EX_MEM.HI = ID_EX.A;
						//NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
						break;

					case 0x0000012:
						//MFLO
						puts( "Move from LO" );
						EX_MEM.ALUOutput = EX_MEM.LO;   
						//CURRENT_STATE.REGS[rd] = NEXT_STATE.LO;
						break;

					case 0x0000010:
						//MFHI
						puts( "Move from HI" );
						EX_MEM.ALUOutput = EX_MEM.HI;
						//CURRENT_STATE.REGS[rd] = NEXT_STATE.HI;
						break;
				}
				//prevInstruction = func;		
				break;

			}
		default:
			{
      
				switch( oc )
				{
					case 0x20000000:
						{	
							//ADDI
							puts( "ADDI" );
							EX_MEM.ALUOutput =  ID_EX.imm + ID_EX.A;
							EX_MEM.type = 1;
							break;
						}
					case 0x24000000:
						{	
							//ADDIU
							puts( "ADDIU" );
							EX_MEM.ALUOutput =  ID_EX.imm + ID_EX.A;
							EX_MEM.type = 1;
							break;
						}		
					case 0xA000000:
						{
							//SB - Store Byte 
							puts("STORE BYTE" );
							uint32_t eAddr = ID_EX.A + ID_EX.imm;              
							EX_MEM.ALUOutput = eAddr;
							EX_MEM.B = ID_EX.B;
							EX_MEM.type = 3;
							      
							break;
						}
					case 0xAC000000:
						{
							//SW - Store Word
							puts("STORE WORD" );
						        uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						        EX_MEM.ALUOutput = eAddr;
						        EX_MEM.B = ID_EX.B;
							EX_MEM.type = 3;
              
							break;
						}
					case 0xA4000000:
						{
							//SH - Store Halfword  
							puts("STORE HALFWORD" );
						      uint32_t eAddr = ID_EX.A +ID_EX.imm;              
						      EX_MEM.ALUOutput = eAddr;
						      EX_MEM.B = ID_EX.B;
              						EX_MEM.type = 3;
							break;
						}
					case 0x8C000000:
						{	
							//LW - Load Word
							puts("LOAD WORD" );
						      uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						      EX_MEM.ALUOutput = eAddr;
						      EX_MEM.B = ID_EX.B;
						      EX_MEM.type = 2;
							break;
						}	
					case 0x80000000:
						{	
							//LB - Load Byte  
							puts("LOAD BYTE" );
						      uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						      EX_MEM.ALUOutput = eAddr;
						      EX_MEM.B = ID_EX.B;
						      EX_MEM.type = 2;
							break;
						}
					case 0x84000000:
						{	
							//LH - Load Halfword
							puts("LOAD HALFWORD" );
						      uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						      EX_MEM.ALUOutput = eAddr;
						      EX_MEM.B = ID_EX.B;
              						EX_MEM.type = 2;
							break;

						}
					case 0x30000000:
						//ANDI
						{                                           
							puts("ANDI" );
							///zero extend immediate then and it with rs
							 EX_MEM.ALUOutput = (ID_EX.imm & 0x0000FFFF) & ID_EX.A;	
							EX_MEM.type = 1;
							break;
						}
					case 0x3C000000:
						{	
							//LUI - Load Upper Immediate
							puts("LOAD IMMEDIATE UPPER" );
							//Load data from instruction into rt register
							 EX_MEM.ALUOutput = (ID_EX.imm << 16);
							EX_MEM.type = 1;
							break;
						}

					case 0x38000000:
						//XORI
						{                                                   
							puts("XORI" );
							///zero extend immediate then and it with rs
							EX_MEM.ALUOutput = (ID_EX.imm & 0x0000FFFF) ^ ID_EX.A;
							EX_MEM.type = 1;
							break;
						}
					case 0x34000000:
						//ORI
						{           
							puts("ORI" );
							///zero extend immediate then and it with rs
							EX_MEM.ALUOutput  = (ID_EX.imm & 0x0000FFFF) | ID_EX.A;	
							EX_MEM.type = 1;
							break;
						}
					
				}

			}

	}

}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	//Update EX INstruction                             
  ID_EX.IR = IF_ID.IR;
                                
	//rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
	uint32_t rs = ( 0x03E00000 & IF_ID.IR  ) >> 21;
	//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
	uint32_t rt = ( 0x001F0000 & IF_ID.IR  ) >> 16;  
	//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
	uint32_t imm = ( 0x0000FFFF & IF_ID.IR  );
  
	  //Load data in ID->EX Buffer
	  ID_EX.A = CURRENT_STATE.REGS[rs];
	  ID_EX.B = CURRENT_STATE.REGS[rt];
	  ID_EX.imm = extend_sign( imm );
	  ID_EX.LO = CURRENT_STATE.LO;
	  ID_EX.HI = CURRENT_STATE.HI;
  
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	uint32_t ins = mem_read_32( CURRENT_STATE.PC );
  	IF_ID.IR = ins;
	NEXT_STATE.PC = CURRENT_STATE.PC + 0x4;  
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/

char* convert_Reg(uint32_t reg){
    if(reg== 0){
        return "$zero";
    }
    else if(reg == 1){
        return "$at";
    }
    else if(reg == 2){
        return "$v0";
    }
    else if(reg == 3){
        return "$v1";
    }
    else if(reg == 4){
        return "$a0";
    }
    else if(reg == 5){
        return "$a1";
    }
    else if(reg == 6){
        return "$a2";
    }
    else if(reg == 7){
        return "$a3";
    }
    else if(reg == 8){
        return "$t0";
    }
    else if(reg == 9){
        return "$t1";
    }
    else if(reg == 10){
        return "$t2";
    }
    else if(reg == 11){
        return "$t3";
    }
    else if(reg == 12){
        return "$t4";
    }
    else if(reg == 13){
        return "$t5";
    }
    else if(reg == 14){
        return "$t6";
    }
    else if(reg == 15){
        return "$t7";
    }
    else if(reg == 16){
        return "$s0";
    }
    else if(reg == 17){
        return "$s1";
    }
    else if(reg == 18){
        return "$s2";
    }
    else if(reg == 19){
        return "$s3";
    }
    else if(reg == 20){
        return "$s4";
    }
    else if(reg == 21){
        return "$s5";
    }
    else if(reg == 22){
        return "$s6";
    }
    else if(reg == 23){
        return "$s7";
    }
    else if(reg == 24){
        return "$t8";
    }
    else if(reg == 25){
        return "$t9";
    }
    else if(reg == 26){
        return "$k0";
    }
    else if(reg == 27){
        return "$k1";
    }
    else if(reg == 28){
        return "$gp";
    }
    else if(reg == 29){
        return "$sp";
    }
    else if(reg == 30){
        return "$fp";
    }
    else if(reg == 31){
        return "$ra";
    }
    else{
        return "error";
    }
}

void print_instruction(uint32_t addr);

void print_program(){
	//int i;
	//uint32_t addr;

  show_pipeline();

	/*for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	} */
}

void print_instruction( uint32_t addr ){
	//Get the current instruction
    uint32_t ins = mem_read_32( addr );
    
    //opcode MASK: 1111 1100 0000 0000 4x0000 = FC0000000
    uint32_t oc = ( 0xFC000000 & ins  );
    
    //Handle each case for instructions
    switch( oc )
    {
            //R-Type
        case 0x00000000:
        {
            //rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
            uint32_t rs = ( 0x03E00000 & ins  ) >> 21;
            //rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
            uint32_t rt = ( 0x001F0000 & ins  ) >> 16;
            //rd MASK: 4x0000 1111 1000 0000 0000 = 0000F800;
            uint32_t rd = ( 0x0000F800 & ins  ) >> 11;
            //sa MASK: 4X0000 0000 0111 1100 0000 = 000007C0;
            uint32_t sa = ( 0x000007C0 & ins  ) >> 6;
            //func MASK: 6x0000 0001 1111 = 0000001F
            uint32_t func = ( 0x0000003F & ins );
            
            switch( func )
            {
                case 0x00000020:
                    //ADD
                    printf( "ADD %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x00000021:
                    //ADDU
                    printf( "ADDU %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x00000022:
                    //SUB
                    printf( "SUB %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x00000023:
                    //SUBU
                    //puts( "Subtract Unsigned Function" );
                    printf( "SUBU %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x00000018:
                {
                    //MULT
                    printf( "MULT %s, %s", convert_Reg(rs), convert_Reg(rt));
                    break;
                }
                    
                case 0x00000019:
                {
                    //MULTU
                    printf( "MULTU %s, %s", convert_Reg(rs), convert_Reg(rt));
                    break;
                }
                    
                case 0x0000001A:
                    //DIV
                    printf( "DIV %s, %s", convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x0000001B:
                    //DIVU
                    printf( "DIVU %s, %s", convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0X00000024:
                    //AND
                    printf( "AND %s, %s, 0x%x", convert_Reg(rd), convert_Reg(rs), rt);
                    break;
                    
                case 0X00000025:
                    //OR
                    printf( "OR %s, %s, 0x%x", convert_Reg(rd), convert_Reg(rs), rt);
                    break;
                    
                case 0X00000026:
                    //XOR
                    printf( "XOR %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x00000027:
                    //NOR
                    printf( "NOR %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0x0000002A:
                    //SLT
                    printf( "SLTs %s, %s, 0x%x", convert_Reg(rd), convert_Reg(rs), rt);
                    break;
                    
                case 0x00000000:
                {
                    //SLL
                    printf( "SLL %s, %s, 0x%x", convert_Reg(rd), convert_Reg(rs), sa);
                    break;
                }
                    
                case 0x00000002:
                {
                    //SRL
                    printf( "SRL %s, %s, 0x%x", convert_Reg(rd), convert_Reg(rs), sa);
                    break;
                }
                    
                case 0x00000003:
                {
                    //SRA
                    printf( "SRA %s, %s, 0x%x", convert_Reg(rd), convert_Reg(rs), sa);
                    break;
                }
                case 0x0000000C:
                    //SYSCALL - System Call, exit the program.
                    printf( "SYSCALL");
                    break;
                    
                case 0x00000008:
                    //JR
                    printf( "JR %s", convert_Reg(rs));
                    break;
                case 0x00000009:
                    //JALR
                    printf( "JALR %s, %s", convert_Reg(rd), convert_Reg(rs));
                    break;
                    
                case 0x00000013:
                    //MTLO
                    printf( "MTLO %s", convert_Reg(rs));
                    break;
                    
                case 0x0000011:
                    //MTHI
                    printf( "MTHI %s", convert_Reg(rs));
                    break;
                    
                case 0x0000012:
                    //MFLO
                    printf( "MFLO %s", convert_Reg(rd));
                    break;
                    
                case 0x0000010:
                    //MFHI
                    printf( "MFHI %s", convert_Reg(rd));
                    break;
            }
            break;
            
        }
        case 0x0C000000:
        {
            //JAL-Jump and Link Instruction
            uint32_t target = ( 0x03FFFFFF & ins  );
            printf( "JAL %s", convert_Reg(target));
            break;
        }
            
        default:
        {
            
            //rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
            uint32_t rs = ( 0x03E00000 & ins  ) >> 21;
            //rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
            uint32_t rt = ( 0x001F0000 & ins  ) >> 16;
            //Immediate MASK: 4x0000 4x1111 = 0000FFFF;
            uint32_t im = ( 0x0000FFFF & ins  );
            
            switch( oc )
            {
                case 0x20000000:
                {
                    //ADDI
                    printf( "ADDI %s, %s, 0x%x", convert_Reg(rt), convert_Reg(rs), im);
                    break;
                }
                case 0x24000000:
                {
                    //ADDIU
                    printf( "ADDIU %s, %s, 0x%x", convert_Reg(rt), convert_Reg(rs), im);
                    break;
                }
                case 0xA000000:
                {
                    //SB - Store Byte
                    printf( "SB %s, 0x%x(%s)", convert_Reg(rt), im, convert_Reg(rs));
                    break;
                }
                case 0xAC000000:
                {
                    //SW - Store Word
                    printf( "SW %s, 0x%x(%s)", convert_Reg(rt), im, convert_Reg(rs));
                    break;
                }
                case 0xA4000000:
                {
                    //SH - Store Halfword
                    printf( "SH %s, 0x%x(%s)", convert_Reg(rt), im, convert_Reg(rs));
                    break;
                }
                case 0x38000000:
                {
                    //XORI - XORI w/
                    printf( "XORI %s,%s 0x%x", convert_Reg(rt),convert_Reg(rs), im);
                    break;
                }
                case 0x8C000000:
                {
                    //LW - Load Word
                    printf( "LW %s, 0x%x(%s)", convert_Reg(rt), im, convert_Reg(rs));
                    break;
                }
                case 0x80000000:
                {
                    //LB - Load Byte
                    printf( "LB %s, 0x%x(%s)", convert_Reg(rt), im, convert_Reg(rs));
                    break;
                }
                case 0x84000000:
                {
                    //LH - Load Halfword
                    printf( "LH %s, 0x%x(%s)", convert_Reg(rt), im, convert_Reg(rs));
                    break;
                    
                }
                case 0x30000000:
                {
                    //ANDI
                    printf( "ANDI %s, %s, 0x%x", convert_Reg(rt), convert_Reg(rs), im);
                    break;
                }
                case 0x3C000000:
                {
                    //LUI - Load Upper Immediate
                    printf( "LUI %s, 0x%x", convert_Reg(rt), im);
                    break;
                }
                case 0x10000000:
                {
                    //BEQ
                    printf( "BEQ %s, %s, 0x%x", convert_Reg(rs), convert_Reg(rt), im);
                    break;
                }
                case 0x14000000:
                {
                    //BNE - Branch on Not Equal
                    printf( "BNE %s, %s, 0x%x", convert_Reg(rs), convert_Reg(rt), im);
                    break;
                }
                case 0x18000000:
                {
                    //BLEZ - Branch on Less Than or Equal to Zero
                    printf( "BLEZ %s, 0x%x", convert_Reg(rs), im);
                    break;
                }
                case 0x1C000000:
                {
                    //BGTZ - Branch on Greater Than Zero
                    printf( "BGTZ %s, 0x%x", convert_Reg(rs), im);
                    break;
                }
                case 0x34000000:
                {
                    //ORI
                    printf( "ORI %s, %s, 0x%x", convert_Reg(rs), convert_Reg(rt), im);
                    break;
                }
                case 0x04000000:
                {
                    //REGIMM
                    switch( rt )
                    {
                        case 0x00000000:
                        {
                            //BLTZ - Branch on Less Than Zero
                            printf( "BLTZ %s, 0x%x", convert_Reg(rs), im);
                            break;
                        }
                        case 0x00000001:
                        {
                            //BGEZ - Branch on Greater Than or Equal to Zero
                            printf( "BGEZ %s, 0x%x", convert_Reg(rs), im);
                            break;
                        }
                    }
                    break;
                }
            }
            
        }
            
    }
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline()
{

  printf( "\nCurrent PC: %x ", CURRENT_STATE.PC );
  
  printf( "\n\nIF/ID.IR %x ", IF_ID.IR );   
  print_instruction( IF_ID.IR );
  printf( "\nIF/ID.PC %x", IF_ID.PC );
  
  printf( "\n\nID/EX.IR %x ", ID_EX.IR );
  print_instruction( ID_EX.IR );
  printf( "\nID/EX.A %x", ID_EX.A );
  printf( "\nID/EX.B %x", ID_EX.B );
  printf( "\nID/EX.imm %d", ID_EX.imm );
  
  printf( "\n\nEX/MEM.IR %x ", EX_MEM.IR );  
  print_instruction( EX_MEM.IR );
  printf( "\nEX/MEM.A %d", EX_MEM.A );
  printf( "\nEX/MEM.B %d", EX_MEM.B );
  printf( "\nEX/MEM.ALUOutput %d", EX_MEM.ALUOutput );     
  
  printf( "\n\nMEM/WB.IR %x ", MEM_WB.IR );
  print_instruction( MEM_WB.IR );
  printf( "\nMEM/WB.A %d", MEM_WB.ALUOutput );
  printf( "\nMEM/WB.B %d", MEM_WB.LMD );
  
  printf( "\n\nDone.\n\n" );

/*
Current PC: value
IF/ID.IR value instruction( e.g. add $1, $2, $3)
IF/ID.PC value //notice that it contains the next PC
ID/EX.IR value instruction
ID/EX.A value
ID/EX.B value
ID/EX.imm value
EX/MEM.IR value
EX/MEM.A value
EX/MEM.B value
EX/MEM.ALUOutput value
MEM/WB.IR value
MEM/WB.ALUOutput value
MEM/WB.LMD value

CPU_Pipeline_Reg IF_ID;
CPU_Pipeline_Reg ID_EX;
CPU_Pipeline_Reg EX_MEM;
CPU_Pipeline_Reg MEM_WB;

*/

}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
