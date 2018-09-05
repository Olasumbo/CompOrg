#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

uint32_t prevInstruction;

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
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
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
			runAll(); 
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
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/

	/*

	   1. READ instruction from mem_read( CurrentState.PC );
	   2. Using the OPCODE, do a switch the handel each of the instructions.

	 */

	//Get the current instruction
	uint32_t ins = mem_read_32( CURRENT_STATE.PC );

	//0000 0000 0010 0001 0000 0000 0010 0000
	//0x00210020
	//ins = 0x20010001;

	//puts( ins );
	puts( "//*************************//" );
	printf( "\nInstruction: %08x \n", ins );

	//opcode MASK: 1111 1100 0000 0000 4x0000 = FC0000000
	uint32_t oc = ( 0xFC000000 & ins  );

	printf( "\nOpcode: %08x \n", oc );

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
				//func MASK: 6x0000 0001 1111 = 0000001F
				uint32_t func = ( 0x0000003F & ins );

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
						puts( "Add Function" );
						NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
						break; 

					case 0x00000021:
						puts( "Add Unsigned Function" );
						NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
						break; 

					case 0x00000022:
						puts( "Subtract Function" );
						NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
						break;

					case 0x00000023:
						puts( "Subtract Unsigned Function" );
						NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
						break;

					case 0x00000018:
						puts( "Multiply Function" );

						//if either of the 2 preceding instructions were MFLO or MFHI, result is undefined
						if(prevInstruction == 0x0000012 || prevInstruction == 0x0000011)
						{
							puts("Result is undefined");
							break;						
						}
						int64_t tempResult = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
						
						NEXT_STATE.LO = tempResult & 0xFFFFFFFF; //get low 32-bits
						NEXT_STATE.HI = tempResult >> 32; //get high 32-bits
						break;

					case 0x0000000C: 
						puts( "Terminate" );
						RUN_FLAG = FALSE;
						break; 

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
				prevInstruction = func;		
				break;

			}
		case 0x08000000:

			//J-Jump Instruction
			break;

		case 0x0C000000:

			//JAL-Jump and Link Instruction
			break;

		default:
			{

				//rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
				uint32_t rs = ( 0x03E00000 & ins  ) >> 21;
				//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
				uint32_t rt = ( 0x001F0000 & ins  ) >> 16;
				//Immediate MASK: 4x0000 4x1111 = 0000FFFF;
				uint32_t im = ( 0x0000FFFF & ins  );

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
							uint32_t sum = ( rt + im ) << 16;
							*NEXT_STATE.REGS = *CURRENT_STATE.REGS | sum;
							break;
						}	
					case 0xA000000:
						{
							//SB - Store Byte

							//Extend sign of immediate register to get memory offset
							uint32_t offset = extend_sign( im );
							//Create a effective memory address
							uint32_t eAddr = offset + CURRENT_STATE.REGS[rs];

							//Store a byte sized data in register rt to the virtual address. Shift right by 24 to only store one byte
							mem_write_32( eAddr, NEXT_STATE.REGS[rt] >> 24 );
							break;
						}
					case 0x8C000000:
						{	
							//LW - Load Word

							//Extend sign of immediate register to get memory offset
							uint32_t offset = extend_sign( im );
							//Create a effective memory address
							uint32_t eAddr = offset + CURRENT_STATE.REGS[rs];

							//Load data from memory into rt register
							NEXT_STATE.REGS[rt] = mem_read_32( eAddr );
							break;

						}	
				}

			}

	}
	NEXT_STATE.PC = CURRENT_STATE.PC + 0x4;
	//?CURRENT_STATE.PC = CURRENT_STATE.PC + 0x4;
	//RUN_FLAG = FALSE;

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
	int i;
	uint32_t addr;

	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
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
