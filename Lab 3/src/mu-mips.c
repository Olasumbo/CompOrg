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
	/*IMPLEMENT THIS*/
  ++INSTRUCTION_COUNT;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	/*IMPLEMENT THIS*/
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

	//Handle each case for instructions
	switch( oc )
	{
		//R-Type
		case 0x00000000: 
			{                                
				//rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
				uint32_t rs = ID_EX.A;
				//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
				uint32_t rt = ID_EX.B;
				//rd MASK: 4x0000 1111 1000 0000 0000 = 0000F800;
				uint32_t rd = ( 0x0000F800 & imm  ) >> 11;
				//sa MASK: 4X0000 0000 0111 1100 0000 = 000007C0;
				uint32_t sa = ( 0x000007C0 & imm  ) >> 6;
				//func MASK: 6x0000 0001 1111 = 0000001F
				uint32_t func = ( 0x0000003F & imm );

				printf( "\nR-Type Instruction:" 
						"\n-> OC: %x" 
						"\n-> rs: %x" 
						"\n-> rt: %x" 
						"\n-> rd: %x" 
						"\n-> func: %x\n",  
						oc, rs, rt, rd, func );

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
						EX_MEM.ALUOutput = 0x0;
						break; 

				/*	case 0x00000008:
						{
						//JR -
						uint32_t temp = CURRENT_STATE.REGS[rs];
						jump = temp - CURRENT_STATE.PC;
						break;
						}
					case 0x00000009:
						{
						//JALR - Jump and Link
						uint32_t temp = CURRENT_STATE.REGS[rs];
						NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 0x8;
						jump = temp - CURRENT_STATE.PC;
						break;
						}    */

					case 0x00000013:
						//MTLO
						puts( "Move to LO" );
						NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
						break;

					case 0x0000011:
						//MTHI
						puts( "Move to HI" );
						NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
						break;

					case 0x0000012:
						//MFLO
						puts( "Move from LO" );
						CURRENT_STATE.REGS[rd] = NEXT_STATE.LO;
						break;

					case 0x0000010:
						//MFHI
						puts( "Move from HI" );
						CURRENT_STATE.REGS[rd] = NEXT_STATE.HI;
						break;
				}
				//prevInstruction = func;		
				break;

			}
/*		case 0x08000000:
			{
				uint32_t target = ( 0x03FFFFFF & ins  );
				uint32_t temp = target << 2;
				uint32_t bits = ( CURRENT_STATE.PC & 0xF0000000 );
				jump = ( bits | temp ) - CURRENT_STATE.PC;
				break;
			}

		case 0x0C000000:
			{
				//JAL-Jump and Link Instruction
				//target MASK: 0000 0011 1110 0000 4x0000 = 03E00000
				uint32_t target = ( 0x03FFFFFF & ins  );
				uint32_t temp = target << 2;
				uint32_t bits = ( CURRENT_STATE.PC & 0xF0000000 );
				NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 0x8;
				jump = ( bits | temp ) - CURRENT_STATE.PC;
				break;
			}*/
		default:
			{

				//rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
				uint32_t rs = ( 0x03E00000 & imm  ) >> 21;
				//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
				uint32_t rt = ( 0x001F0000 & imm  ) >> 16;
				//Immediate MASK: 4x0000 4x1111 = 0000FFFF;
				uint32_t im = ( 0x0000FFFF & imm  );

				//I-Type Instruction
				printf( "\nI-Type Instruction:" 
						"\n-> OC: %x" 
						"\n-> rs: %x" 
						"\n-> rt: %x" 
						"\n-> Immediate: %x\n",  
						oc, rs, rt, im );

				switch( oc )
				{
					case 0x20000000:
						{	
							//ADDI
							puts( "ADDI" );
							uint32_t result = extend_sign(im) + CURRENT_STATE.REGS[rs];
							NEXT_STATE.REGS[rt] = result;
							break;
						}
					case 0x24000000:
						{	
							//ADDIU
							puts( "ADDIU" );
							uint32_t result = extend_sign(im) + CURRENT_STATE.REGS[rs];
							NEXT_STATE.REGS[rt] = result;
							break;
						}		
					case 0xA000000:
						{
							//SB - Store Byte 
							puts("STORE BYTE" );
              uint32_t eAddr = ID_EX.A + imm;              
              EX_MEM.ALUOutput = eAddr;
              EX_MEM.B = ID_EX.B;
              
							break;
						}
					case 0xAC000000:
						{
							//SW - Store Word
							puts("STORE WORD" );
              uint32_t eAddr = ID_EX.A + imm;              
              EX_MEM.ALUOutput = eAddr;
              EX_MEM.B = ID_EX.B;
              
							break;
						}
					case 0xA4000000:
						{
							//SH - Store Halfword  
							puts("STORE HALFWORD" );
              uint32_t eAddr = ID_EX.A + imm;              
              EX_MEM.ALUOutput = eAddr;
              EX_MEM.B = ID_EX.B;
              
							break;
						}
					case 0x8C000000:
						{	
							//LW - Load Word
							puts("LOAD WORD" );
              uint32_t eAddr = ID_EX.A + imm;              
              EX_MEM.ALUOutput = eAddr;
              EX_MEM.B = ID_EX.B;
              
							break;
						}	
					case 0x80000000:
						{	
							//LB - Load Byte  
							puts("LOAD BYTE" );
              uint32_t eAddr = ID_EX.A + imm;              
              EX_MEM.ALUOutput = eAddr;
              EX_MEM.B = ID_EX.B;
              
							break;
						}
					case 0x84000000:
						{	
							//LH - Load Halfword
							puts("LOAD HALFWORD" );
              uint32_t eAddr = ID_EX.A + imm;              
              EX_MEM.ALUOutput = eAddr;
              EX_MEM.B = ID_EX.B;
              
							break;

						}
					case 0x30000000:
						//ANDI
						{                                           
							puts("ANDI" );
							///zero extend immediate then and it with rs
							uint32_t temp = (im & 0x0000FFFF) & CURRENT_STATE.REGS[rs];	
							NEXT_STATE.REGS[rt] = temp;
							break;
						}
					case 0x3C000000:
						{	
							//LUI - Load Upper Immediate
							puts("LOAD IMMEDIATE UPPER" );
							//Load data from instruction into rt register
							NEXT_STATE.REGS[rt] = (im << 16);
							break;
						}
				/*	case 0x10000000:	
						{
							//BEG    
							puts("BRANCH EQUAL" );
							uint32_t target = extend_sign( im ) << 2;
							if( CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt] )
							{
								jump = target;
							}
							break;
						}
					case 0x14000000:	
						{
							//BNE - Branch on Not Equal   
							puts("BRANCH NOT EQUAL" );
							uint32_t target = extend_sign( im ) << 2;
							if( CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt] )
							{
								jump = target;
							}
							break;
						}
					case 0x18000000:	
						{
							//BLEZ - Branch on Less Than or Equal to Zero    
							puts("Branch on Less Than or Equal to Zero" );
							uint32_t target = extend_sign( im ) << 2;
							if( ( CURRENT_STATE.REGS[rs] & 0x80000000 ) || ( CURRENT_STATE.REGS[rt] == 0 ) )
							{
								jump = target;
							}
							break;
						}
					case 0x1C000000:	
						{
							//BGTZ - Branch on Greater Than Zero     
							puts("BGTZ" );
							uint32_t target = extend_sign( im ) << 2;
							if( !( CURRENT_STATE.REGS[rs] & 0x80000000 ) || ( CURRENT_STATE.REGS[rt] != 0 ) )
							{
								jump = target;
							}
							break;
						}     */
					case 0x38000000:
						//XORI
						{                                                   
							puts("XORI" );
							///zero extend immediate then and it with rs
							uint32_t temp = (im & 0x0000FFFF) ^ CURRENT_STATE.REGS[rs];
							NEXT_STATE.REGS[rt] = temp;
							break;
						}
					case 0x34000000:
						//ORI
						{           
							puts("ORI" );
							///zero extend immediate then and it with rs
							uint32_t temp = (im & 0x0000FFFF) | CURRENT_STATE.REGS[rs];	
							NEXT_STATE.REGS[rt] = temp;
							break;
						}
					/*case 0x04000000:	
						{
							//REGIMM        
							switch( rt )
							{
								case 0x00000000:
									{
										//BLTZ - Branch on Less Than Zero      
						      	puts("BLTZ" );
										uint32_t target = extend_sign( im ) << 2;
										if( ( CURRENT_STATE.REGS[rs] & 0x80000000 ) )
										{
											jump = target;
										}
                    break;
									}
								  case 0x00000001:
									{
										//BGEZ - Branch on Greater Than or Equal to Zero           
						      	puts("BGEZ" );
										uint32_t target = extend_sign( im ) << 2;
										if( !( CURRENT_STATE.REGS[rs] & 0x80000000 ) )
										{
											jump = target;
										}
                    break;
									}
							}          
							break;
						}	*/	
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
void print_program(){
	/*IMPLEMENT THIS*/
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
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
