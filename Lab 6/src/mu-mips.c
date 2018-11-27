#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"
#include "mu-cache.h"
//test


char* convert_Reg(uint32_t reg);

/***************************************************************/
/* Extend Sign Function from 16-bits to 32-bits */
/***************************************************************/
uint32_t extend_sign( uint32_t im )
{
	uint32_t data = ( im & 0x0000FFFF );
	uint32_t mask = 0x00008000;
	
	/*if( data == 0x00008000 )
	{
		return data;
	}*/
		
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
		case 'f':
			if (scanf("%d", &ENABLE_FORWARDING) != 1) {
				break;
			}
			ENABLE_FORWARDING == 0 ? printf("Forwarding OFF\n") : printf("Forwarding ON\n");
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

	//MEM_WB.RegWrite = EX_MEM.RegWrite;

	uint32_t rt = ( 0x001F0000 & MEM_WB.IR  ) >> 16;
	uint32_t rd = ( 0x0000F800 & MEM_WB.IR  ) >> 11;

	//printf( "\n\nINS: %d", MEM_WB.type );
	//print_instruction( MEM_WB.PC );
	//printf( "\n\n" );

	if( MEM_WB.type == 0 )
	{
		NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
		CURRENT_STATE.REGS[rd] = MEM_WB.ALUOutput;

		NEXT_STATE.HI = MEM_WB.HI;
		NEXT_STATE.LO = MEM_WB.LO;
		CURRENT_STATE.HI = MEM_WB.HI;
		CURRENT_STATE.LO = MEM_WB.LO;

	}
	else if( MEM_WB.type == 1)
	{
		NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
		CURRENT_STATE.REGS[rt] = MEM_WB.ALUOutput;
	}
	else if( MEM_WB.type == 2 )
	{
		NEXT_STATE.REGS[rt] = MEM_WB.LMD;
		CURRENT_STATE.REGS[rt] = MEM_WB.LMD;
		//printf( "\nLoaded %x\n", MEM_WB.LMD );
	}
	else if( MEM_WB.type == 3 )
	{
			printf( "\nWB UPDATE[%x]:\n"
				"-> [0] = %u\n"
				"-> [4] = %u\n"
				"-> [8] = %u\n"
				"-> [c] = %u\n", ( (MEM_WB.ALUOutput) ),
				writeBuffer.words[0],writeBuffer.words[1],writeBuffer.words[2],writeBuffer.words[3]);

		mem_write_32( (MEM_WB.ALUOutput & 0xFFFFFFF0) + 0x0, writeBuffer.words[0] );
		mem_write_32( (MEM_WB.ALUOutput & 0xFFFFFFF0) + 0x4, writeBuffer.words[1] );
		mem_write_32( (MEM_WB.ALUOutput & 0xFFFFFFF0) + 0x8, writeBuffer.words[2] );
		mem_write_32( (MEM_WB.ALUOutput & 0xFFFFFFF0) + 0xC, writeBuffer.words[3] );
	}
	else if( MEM_WB.type == 4)
	{
		//NEXT_STATE.REGS[0] = 0XA;
		RUN_FLAG = FALSE;
	}
	else if( MEM_WB.type == 5)
	{
	}

  	++INSTRUCTION_COUNT;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	if( MEM_STALL > 0 )
	{
		--MEM_STALL;
		printf( "MEM STAGE STALL : %d", MEM_STALL ); 
		return;
	}

	MEM_WB.IR = EX_MEM.IR;
	MEM_WB.PC = EX_MEM.PC;
	MEM_WB.type = EX_MEM.type;
	MEM_WB.RegisterRs = EX_MEM.RegisterRs;
	MEM_WB.RegisterRt = EX_MEM.RegisterRt;
	MEM_WB.RegisterRd = EX_MEM.RegisterRd;
	MEM_WB.RegWrite = EX_MEM.RegWrite;
	MEM_WB.DestReg = EX_MEM.DestReg;

	MEM_WB.LO = EX_MEM.LO;
	MEM_WB.HI = EX_MEM.HI;

	printf( "\nHITS: %d; MISSES: %d\n", cache_hits, cache_misses );

	if(EX_MEM.type <= 1)		//0 reg-reg, 1 reg-imm
	{
		MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	}
	else if(EX_MEM.type == 2)	//2 is Load
	{
		uint32_t blocknum = ( EX_MEM.ALUOutput & 0x000000F0 ) >> 4;
		uint32_t wordnum  = ( EX_MEM.ALUOutput & 0x0000000C ) >> 2;
		//uint32_t byteoff  = ( MEM_WB.ALUOutput & 0x00000003 );
		CacheBlock getBlock = L1Cache.blocks[blocknum];

		if( EX_MEM.CacheMiss == 0 )
		{
			//HIT
			MEM_WB.LMD = getBlock.words[wordnum];
		}
		else
		{
			//MISS
			MEM_WB.LMD = mem_read_32( EX_MEM.ALUOutput );
			getBlock.words[0] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0x0 );
			getBlock.words[1] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0x4 );
			getBlock.words[2] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0x8 );
			getBlock.words[3] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0xC );

			MEM_STALL = 100;
		}
	}
	else if(EX_MEM.type == 3)	//3 is store
	{
		int index = ( EX_MEM.ALUOutput & 0x000000F0 ) >> 4;
		int word_offset  = ( EX_MEM.ALUOutput & 0x0000000C ) >> 2;
		CacheBlock getBlock = L1Cache.blocks[index];

		if( EX_MEM.CacheMiss == 0 )
		{
			//HIT
			getBlock.words[word_offset] = EX_MEM.B;
			getBlock.valid = 1;
			writeBuffer = getBlock;
			MEM_WB.ALUOutput = EX_MEM.ALUOutput;
		}
		else
		{
			//MISS
			getBlock.words[0] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0x0 );
			getBlock.words[1] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0x4 );
			getBlock.words[2] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0x8 );
			getBlock.words[3] = mem_read_32( (EX_MEM.ALUOutput & 0xFFFFFFF0) + 0xC );
			
			getBlock.words[word_offset] = EX_MEM.B;

			printf( "\nCACHE UPDATE[%d, %d]:\n"
				"-> [0] = %u\n"
				"-> [4] = %u\n"
				"-> [8] = %u\n"
				"-> [c] = %u\n",
				index, word_offset,
				getBlock.words[0],getBlock.words[1],getBlock.words[2],getBlock.words[3] );
			getBlock.valid = 1;
			writeBuffer = getBlock;
			MEM_WB.ALUOutput = EX_MEM.ALUOutput;
		}
		
	}
	else if(EX_MEM.type == 6)
	{
		MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	}

}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	if( MEM_STALL > 0 )
	{
		return;
	}

	//Load INstruction from Buffer
	EX_MEM.IR = ID_EX.IR;
	EX_MEM.PC = ID_EX.PC;
	EX_MEM.RegisterRs = ID_EX.RegisterRs;
	EX_MEM.RegisterRt = ID_EX.RegisterRt;
	EX_MEM.RegisterRd = ID_EX.RegisterRd;
	EX_MEM.CacheMiss = 0;

	//opcode MASK: 1111 1100 0000 0000 4x0000 = FC0000000
	uint32_t oc = ( 0xFC000000 & EX_MEM.IR  );
  	//uint32_t imm = ID_EX.imm;
	uint32_t func = (EX_MEM.IR & 0x0000003F);
	//rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
	uint32_t rs = ( 0x03E00000 & EX_MEM.IR  ) >> 21;
	//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
	uint32_t rt = ( 0x001F0000 & EX_MEM.IR  ) >> 16;
	//rd MASK: 4x0000 1111 1000 0000 0000 = 0000F800;
	//uint32_t rd = ( 0x0000F800 & EX_MEM.IR  ) >> 11;
	//sa MASK: 4X0000 0000 0111 1100 0000 = 000007C0;
	uint32_t sa = ( 0x000007C0 & EX_MEM.IR  ) >> 6;

	if( ID_EX.IR == 0 )
	{
		EX_MEM.type = 5;
		EX_MEM.RegWrite = 0;
		EX_MEM.DestReg = 0;
		puts( "EX STALLED ONE CYCLE" );
		return;
	}

	//Handle each case for instructions
	switch( oc )
	{
		//R-Type
		case 0x00000000: 
			{  
				EX_MEM.type = 0;
				EX_MEM.RegWrite = 1;
				EX_MEM.DestReg = EX_MEM.RegisterRd;

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
						CNT_STALL += 2;
						break;

					case 0x0000001B:
						//DIVU
						puts( "Divide Unsigned Function" );
						EX_MEM.type = 5;
						if( ID_EX.B == 0 )
						{ 
							puts( "ERROR: Trying to divide by 0" ); 
						}
						else
						{
							EX_MEM.ALUOutput = 0;
							EX_MEM.HI = ID_EX.A % ID_EX.B ;
							EX_MEM.LO = ID_EX.A / ID_EX.B ;
						}
						CNT_STALL += 2;
						break;

					case 0X00000024:
						//AND                              
						puts("AND" );
						EX_MEM.ALUOutput = ID_EX.A & ID_EX.B;
						break; 

					case 0X00000025:
						//OR                      
						puts("OR" );
						EX_MEM.ALUOutput = ID_EX.A | ID_EX.B;
						break; 
					
					case 0X00000026:
						//XOR                      
						puts("XOR" );
						EX_MEM.ALUOutput = ID_EX.A ^ ID_EX.B;
						break;

					case 0x00000027:
						//NOR                      
						puts("NOR" );
						EX_MEM.ALUOutput = ~( ID_EX.A | ID_EX.B );
						break;

					case 0x0000002A:
						//SLT                      
						puts("SLT" );
						if( ID_EX.A < ID_EX.B )
							EX_MEM.ALUOutput = 0x00000001;
						else
							EX_MEM.ALUOutput = 0x00000000;
						break;

					case 0x00000000:
					{
						//SLL                      
						puts("SLL" );
						EX_MEM.ALUOutput = ID_EX.B << sa;
						break;
					}

					case 0x00000002:
					{
						//SRL                      
						puts("SRL" );
						EX_MEM.ALUOutput = ID_EX.B >> sa;
						break;
					}			

					case 0x00000003:
					{
						//SRA                      
						puts("SRA" );
						printf("\nB: %x\n", ID_EX.B );
						EX_MEM.ALUOutput = extend_sign( ( ID_EX.B >> sa ) );
						break;
					}
					case 0x0000000C:
						//SYSCALL - System Call, exit the program.                      
						puts("SYSCALL" );
						//EX_MEM.ALUOutput = 0xA;
						EX_MEM.type = 4;
						break; 

					case 0x00000013:
						//MTLO
						EX_MEM.type=5;
						puts( "Move to LO" );
						EX_MEM.LO = ID_EX.A;
						NEXT_STATE.LO = ID_EX.A;
						break;

					case 0x0000011:
						//MTHI
						EX_MEM.type=5;
						puts( "Move to HI" );
						EX_MEM.HI = ID_EX.A;
						NEXT_STATE.HI = ID_EX.A;
						break;

					case 0x0000012:
						//MFLO
						puts( "Move from LO" );
						EX_MEM.ALUOutput = ID_EX.LO;  
						printf("\nLO VALUE: %x", ID_EX.LO ); 
						//CURRENT_STATE.REGS[rd] = NEXT_STATE.LO;
						break;

					case 0x0000010:
						//MFHI
						puts( "Move from HI" );
						EX_MEM.ALUOutput = ID_EX.HI;
						printf("\nHI VALUE: %x", ID_EX.HI );
						//CURRENT_STATE.REGS[rd] = NEXT_STATE.HI;
						break;
					case 0x00000008:
						{
							//JR -
							TAKE_JUMP = 1;
							CNT_STALL = 1;
							EX_MEM.DestReg = 0;
							EX_MEM.RegWrite = 0;
							EX_MEM.type = 6;
							uint32_t temp = ID_EX.A;
								temp = 0x004000bc;
							EX_MEM.ALUOutput = temp;
							NEXT_STATE.PC = temp;
							//printf( "\n\nJR TO : %X", 0x004000bc );
							break;
						}
					case 0x00000009:
						{
							//JALR -
							TAKE_BRANCH = 1;
							CNT_STALL = 1;
							EX_MEM.DestReg = 0;
							EX_MEM.RegWrite = 0;
							uint32_t temp = ID_EX.A;
								temp = 0x00400090;
							EX_MEM.ALUOutput = temp - CURRENT_STATE.PC;
							NEXT_STATE.PC = temp;
							//NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 0x8;
							break;
						}

				}
				//prevInstruction = func;		
				break;

			}
		case 0x08000000:
			{
				TAKE_JUMP = 1;
				CNT_STALL = 1;
				EX_MEM.DestReg = 0;
				EX_MEM.RegWrite = 0;
				EX_MEM.type = 6;

				uint32_t target = ( 0x03FFFFFF & ID_EX.IR  );
				uint32_t temp = target << 2;
				uint32_t bits = ( CURRENT_STATE.PC & 0xF0000000 );

				printf("\n\nJump INS:\n"
					"Address: %x\n"
					"CS.PC: %x\n", 
					(bits | temp), CURRENT_STATE.PC );
				
				NEXT_STATE.PC = (bits | temp);
				EX_MEM.ALUOutput = ( bits | temp );
				break;
			}

		case 0x0C000000:
			{
				TAKE_JUMP = 1;
				CNT_STALL = 1;
				EX_MEM.DestReg = 0;
				EX_MEM.RegWrite = 0;
				EX_MEM.type = 6;
				uint32_t target = ( 0x03FFFFFF & ID_EX.IR  );
				uint32_t temp = target << 2;
				uint32_t bits = ( CURRENT_STATE.PC & 0xF0000000 );
				NEXT_STATE.PC = (bits | temp);
				EX_MEM.ALUOutput = ( bits | temp );
//				NEXT_STATE.REGS[sa] = CURRENT_STATE.PC + 0x8;
				break;
			}
		default:
			{
				EX_MEM.DestReg = EX_MEM.RegisterRt;
				EX_MEM.RegWrite = 1;

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
							printf("\nEX->ADDIU: %s %s %u  \n", convert_Reg(rs), convert_Reg(rt), ID_EX.imm);
							break;
						}		
					case 0xA0000000:
						{
							//SB - Store Byte 
							puts("STORE BYTE" );
							uint32_t eAddr = ID_EX.A + ID_EX.imm;              
							EX_MEM.ALUOutput = eAddr;
							EX_MEM.B = ID_EX.B;
							EX_MEM.type = 3;
							EX_MEM.RegWrite = 0;	
							printf( "\n%x | STOREBYTEDATA-> rt(B): %x; rs(A): %x", ID_EX.IR, ID_EX.B , ID_EX.A );						      

							uint32_t blocknum = ( eAddr & 0x000000F0 ) >> 4;
							uint32_t tag 	  = ( eAddr & 0xFFFFFF00 );

							CacheBlock getBlock = L1Cache.blocks[blocknum];
							if( ( getBlock.tag == tag ) && ( getBlock.valid == 1 ) )
							{
								++cache_hits;
							}
							else
							{
								++cache_misses;
								EX_MEM.CacheMiss = 1;
							}
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
							EX_MEM.RegWrite = 0;
							printf( "\n%x | STOREWORDDATA-> rt(B): %x; rs(A): %x", ID_EX.IR, ID_EX.B , ID_EX.A );

							uint32_t blocknum = ( eAddr & 0x000000F0 ) >> 4;
							uint32_t tag 	  = ( eAddr & 0xFFFFFF00 );

							CacheBlock getBlock = L1Cache.blocks[blocknum];
							if( ( getBlock.tag == tag ) && ( getBlock.valid == 1 ) )
							{
								++cache_hits;
							}
							else
							{
								++cache_misses;
								EX_MEM.CacheMiss = 1;
							}
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
							EX_MEM.RegWrite = 0;

							uint32_t blocknum = ( eAddr & 0x000000F0 ) >> 4;
							uint32_t tag 	  = ( eAddr & 0xFFFFFF00 );

							CacheBlock getBlock = L1Cache.blocks[blocknum];
							if( ( getBlock.tag == tag ) && ( getBlock.valid == 1 ) )
							{
								++cache_hits;
							}
							else
							{
								++cache_misses;
								EX_MEM.CacheMiss = 1;
							}
							break;
						}
					case 0x8C000000:
						{	
							//LW - Load Word
							puts("LOAD WORD" );
						      	uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						      	EX_MEM.ALUOutput = eAddr;
						      	EX_MEM.type = 2;

							uint32_t blocknum = ( eAddr & 0x000000F0 ) >> 4;
							uint32_t tag 	  = ( eAddr & 0xFFFFFF00 );

							CacheBlock getBlock = L1Cache.blocks[blocknum];
							if( ( getBlock.tag == tag ) && ( getBlock.valid == 1 ) )
							{
								++cache_hits;
							}
							else
							{
								++cache_misses;
								EX_MEM.CacheMiss = 1;
							}

							break;
						}	
					case 0x80000000:
						{	
							//LB - Load Byte  
							puts("LOAD BYTE" );
						      	uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						      	EX_MEM.ALUOutput = eAddr;
						      	EX_MEM.type = 2;

							uint32_t blocknum = ( eAddr & 0x000000F0 ) >> 4;
							uint32_t tag 	  = ( eAddr & 0xFFFFFF00 );

							CacheBlock getBlock = L1Cache.blocks[blocknum];
							if( ( getBlock.tag == tag ) && ( getBlock.valid == 1 ) )
							{
								++cache_hits;
							}
							else
							{
								++cache_misses;
								EX_MEM.CacheMiss = 1;
							}

							printf( "\n->> LoadByteFrom-> %x", eAddr );
							break;
						}
					case 0x84000000:
						{	
							//LH - Load Halfword
							puts("LOAD HALFWORD" );
						      	uint32_t eAddr = ID_EX.A + ID_EX.imm;              
						      	EX_MEM.ALUOutput = eAddr;
							EX_MEM.type = 2;

							uint32_t blocknum = ( eAddr & 0x000000F0 ) >> 4;
							uint32_t tag 	  = ( eAddr & 0xFFFFFF00 );

							CacheBlock getBlock = L1Cache.blocks[blocknum];
							if( ( getBlock.tag == tag ) && ( getBlock.valid == 1 ) )
							{
								++cache_hits;
							}
							else
							{
								++cache_misses;
								EX_MEM.CacheMiss = 1;
							}

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
					case 0x28000000:
						//SLTI                      
						puts("SLTI" );
						EX_MEM.type = 1;
						if( ID_EX.A < extend_sign( ID_EX.imm ) )
							EX_MEM.ALUOutput = 0x00000001;
						else
							EX_MEM.ALUOutput = 0x00000000;
						break;
					case 0x10000000:	
						{             
							puts("BEQ" );
							//BEQ
							EX_MEM.DestReg = 0;
							EX_MEM.RegWrite = 0;
							EX_MEM.type = 6;		

							if( ID_EX.A == ID_EX.B )
							{
								TAKE_BRANCH = 1;
								uint32_t target = extend_sign( ID_EX.imm << 2 );
								//uint32_t bits = ( ID_EX.PC & 0xF0000000 );
								EX_MEM.ALUOutput = ( ID_EX.PC + target );
								NEXT_STATE.PC = ( ID_EX.PC + target );

								puts("Taking Branch Equal");
							}
							else
							{
								TAKE_BRANCH = 0;
							}

							break;
						}
					case 0x14000000:	
						{
							puts("BNE" );
							//BNE - Branch on Not Equal
							CNT_STALL = 1;
							EX_MEM.DestReg = 0;
							EX_MEM.RegWrite = 0;
							EX_MEM.type = 6;
							
							if( ID_EX.A != ID_EX.B )
							{
								TAKE_BRANCH = 1;
								uint32_t target = extend_sign( ID_EX.imm << 2 );
								//uint32_t bits = ( ID_EX.PC & 0xF0000000 );
								EX_MEM.ALUOutput = ( ID_EX.PC + target );
								NEXT_STATE.PC = ( ID_EX.PC + target );
								CURRENT_STATE.PC = ( ID_EX.PC + target );
								puts("Taking Branch NOT Equal");
							}
							else
							{
								TAKE_BRANCH = 0;
							}

							break;
						}
					case 0x18000000:	
						{
							puts("BLEZ" );
							//BLEZ - Branch on Less Than or Equal to Zero
							CNT_STALL = 1;
							EX_MEM.DestReg = 0;
							EX_MEM.RegWrite = 0;
							EX_MEM.type = 6;		

							if( ( ID_EX.A & 0x80000000 ) || ( ID_EX.A == 0 ) )
							{
								TAKE_BRANCH = 1;
								uint32_t target = extend_sign( ID_EX.imm << 2 );
								//uint32_t bits = ( ID_EX.PC & 0xF0000000 );
								EX_MEM.ALUOutput = ( ID_EX.PC + target );
								NEXT_STATE.PC = ( ID_EX.PC + target );
								CURRENT_STATE.PC = ( ID_EX.PC + target );
								puts("Taking Branch Less Than Equal");
							}
							else
							{
								TAKE_BRANCH = 0;
							}
							break;
						}
					case 0x1C000000:	
						{
							puts("BGTZ" );
							//BGTZ - Branch on Greater Than Zero
							CNT_STALL = 1;
							EX_MEM.DestReg = 0;
							EX_MEM.RegWrite = 0;
							EX_MEM.type = 6;		

							if( !( ID_EX.A & 0x80000000 ) || ( ID_EX.A != 0 ) )
							{
								TAKE_BRANCH = 1;
								uint32_t target = extend_sign( ID_EX.imm << 2 );
								//uint32_t bits = ( ID_EX.PC & 0xF0000000 );
								EX_MEM.ALUOutput = ( ID_EX.PC + target );
								NEXT_STATE.PC = ( ID_EX.PC + target );
								CURRENT_STATE.PC = ( ID_EX.PC + target );
							}
							else
							{
								TAKE_BRANCH = 0;
							}
							break;
						}
					case 0x04000000:	
						{
							//REGIMM
							switch( rt )
							{
								case 0x00000000:
									{
										puts("BLTZ" );
										//BLTZ - Branch on Less Than Zero
										CNT_STALL = 1;
										EX_MEM.DestReg = 0;
										EX_MEM.RegWrite = 0;
										EX_MEM.type = 6;		

										if( ID_EX.A & 0x80000000 )
										{
											TAKE_BRANCH = 1;
											uint32_t target = extend_sign( ID_EX.imm << 2 );
											//uint32_t bits = ( ID_EX.PC & 0xF0000000 );
											EX_MEM.ALUOutput = ( ID_EX.PC + target );
											NEXT_STATE.PC = ( ID_EX.PC + target );
											CURRENT_STATE.PC = ( ID_EX.PC + target );
										}
										else
										{
											TAKE_BRANCH = 0;
										}
										break;
									}
								case 0x00000001:
									{
										puts("BGEZ" );
										//BGEZ - Branch on Greater Than or Equal to Zero
										CNT_STALL = 1;
										EX_MEM.DestReg = 0;
										EX_MEM.RegWrite = 0;
										EX_MEM.type = 6;		

										if( !( ID_EX.A & 0x80000000 ) )
										{
											TAKE_BRANCH = 1;
											uint32_t target = extend_sign( ID_EX.imm << 2 );
											//uint32_t bits = ( ID_EX.PC & 0xF0000000 );
											EX_MEM.ALUOutput = ( ID_EX.PC + target );
											NEXT_STATE.PC = ( ID_EX.PC + target );
											CURRENT_STATE.PC = ( ID_EX.PC + target );
										}
										else
										{
											TAKE_BRANCH = 0;
										}
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
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	if( MEM_STALL > 0 )
	{
		return;
	}

	//Update EX INstruction     
  	ID_EX.IR = IF_ID.IR;
	ID_EX.PC = IF_ID.PC;

	//printf( "\nINS: %x\n", ID_EX.IR );
                        
	//opcode MASK: 1111 1100 0000 0000 4x0000 = FC0000000
//	uint32_t oc = ( 0xFC000000 & ID_EX.IR  );
	//rs MASK: 0000 0011 1110 0000 4x0000 = 03E00000
	uint32_t rs = ( 0x03E00000 & ID_EX.IR  ) >> 21;
	//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
	uint32_t rt = ( 0x001F0000 & ID_EX.IR  ) >> 16;  
	//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;
	uint32_t rd = ( 0x0000F800 & ID_EX.IR  ) >> 11;  
	//rt MASK: 0000 0000 0001 1111 4x0000 = 001F0000;[400008]	STALL COUNT: 0
	uint32_t imm = ( 0x0000FFFF & ID_EX.IR  );
  
	//Load data in ID->EX Buffer
	ID_EX.A = CURRENT_STATE.REGS[rs];
	ID_EX.B = CURRENT_STATE.REGS[rt];
	ID_EX.LO = CURRENT_STATE.LO;
	ID_EX.HI = CURRENT_STATE.HI;
	ID_EX.imm = extend_sign( imm );

	ID_EX.RegisterRs = rs;
	ID_EX.RegisterRt = rt;
	ID_EX.RegisterRd = rd;

	if( CNT_STALL > 0 )
	{
		puts( "->ID Stall" );
		ID_EX.A = NEXT_STATE.REGS[rs];
		ID_EX.B = NEXT_STATE.REGS[rt];
	}

	if ( MEM_WB.RegWrite && (MEM_WB.DestReg != 0) && (MEM_WB.DestReg == ID_EX.RegisterRs))
	{
		puts( "Mem.Dest = Rs" );
		if( ENABLE_FORWARDING == 1 )
		{
			ID_EX.A = MEM_WB.LMD;
			ID_EX.B = NEXT_STATE.REGS[rt];
		}
		else
		{
			CNT_STALL = 1;
		}
	}
	else if ( MEM_WB.RegWrite && (MEM_WB.DestReg != 0) && (MEM_WB.DestReg == ID_EX.RegisterRt))
	{
		puts( "Mem.Dest = Rt" );
		if( ENABLE_FORWARDING == 1 )
		{
			ID_EX.B = MEM_WB.LMD;
			ID_EX.A = NEXT_STATE.REGS[rs];
		}
		else
		{
			CNT_STALL = 1;
		}
	}
	else if ( EX_MEM.RegWrite && (EX_MEM.DestReg != 0) && (EX_MEM.DestReg == ID_EX.RegisterRs) )
	{
		puts( "Ex.Dest = Rs" );
		if( ENABLE_FORWARDING == 1 )
		{
			ID_EX.A = EX_MEM.ALUOutput;
			ID_EX.B = NEXT_STATE.REGS[rt];
			printf( "Forward to ID_EX.A from ALU = %x", EX_MEM.ALUOutput );
			CNT_STALL = 0;
		} else {
			CNT_STALL = 2;
		}
	}
	else if ( EX_MEM.RegWrite && (EX_MEM.DestReg != 0) && (EX_MEM.DestReg == ID_EX.RegisterRt))
	{

		puts( "Ex.Dest = Rt" );
		if( ENABLE_FORWARDING == 1 )
		{
			ID_EX.A = NEXT_STATE.REGS[rs];
			ID_EX.B = EX_MEM.ALUOutput;
			printf( "Forward to ID_EX.B from ALU = %x", EX_MEM.ALUOutput );
			CNT_STALL = 0;
		} else {
			CNT_STALL = 2;
		}
	}

	if( ( ENABLE_FORWARDING == 1 ) && EX_MEM.RegWrite && (EX_MEM.DestReg != 0) && (EX_MEM.DestReg == ID_EX.RegisterRs) && ( EX_MEM.type == 2 ) )
	{
		CNT_STALL = 1;
	}
	else if ( ( ENABLE_FORWARDING == 1 ) && EX_MEM.RegWrite && (EX_MEM.DestReg != 0) && (EX_MEM.DestReg == ID_EX.RegisterRt) && ( EX_MEM.type == 2 ) )
	{
		CNT_STALL = 1;
	}

	if( ( CNT_STALL > 0 ) || ( TAKE_BRANCH == 1 ) || ( TAKE_JUMP == 1 ) )
	{
		//puts("Sending Blank INS");
		ID_EX.IR = 0;
		ID_EX.A = 0;
		ID_EX.B = 0;
		ID_EX.LO = 0;
		ID_EX.HI = 0; 
		ID_EX.imm = 0;
		ID_EX.RegisterRs = 0;
		ID_EX.RegisterRt = 0;
		ID_EX.RegisterRd = 0;
	}

	printf( "\n\nREADING: rs: %x; rt: %x; imm: %x\n", ID_EX.A, ID_EX.B, ID_EX.imm );

}


/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	if( MEM_STALL > 0 )
	{
		return;
	}

	printf( "\n[%x]	STALL COUNT: %d;\n", CURRENT_STATE.PC, CNT_STALL );
	print_instruction( CURRENT_STATE.PC );
	printf( "\n" );

	if( CNT_STALL > 0 )
	{
		puts( "->IF Stall" );
		--CNT_STALL;
	}
	else	
	{
		if( TAKE_BRANCH == 1 )
		{
			puts( "Taking Branch" );
			//NEXT_STATE.PC = MEM_WB.PC + MEM_WB.ALUOutput;
		    	IF_ID.PC = NEXT_STATE.PC;
			uint32_t ins = mem_read_32( NEXT_STATE.PC );
		  	IF_ID.IR = ins;
			TAKE_BRANCH = 0;
			NEXT_STATE.PC = CURRENT_STATE.PC + 0x4;
		}
		else if( TAKE_JUMP == 1 )
		{
			puts( "Taking Jump" );
			//NEXT_STATE.PC = MEM_WB.ALUOutput;
		    	IF_ID.PC = NEXT_STATE.PC;
			uint32_t ins = mem_read_32( NEXT_STATE.PC );
		  	IF_ID.IR = ins;
			TAKE_JUMP = 0;
		}
		else
		{
			NEXT_STATE.PC = CURRENT_STATE.PC + 0x4;
		    	IF_ID.PC = CURRENT_STATE.PC;
			uint32_t ins = mem_read_32(  CURRENT_STATE.PC );
		  	IF_ID.IR = ins;
		}
	}
	printf( "TAKE_BRANCH: %d;\n", TAKE_BRANCH );
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
	cache_misses = 0;
	cache_hits = 0;
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
    else {
        return "error";
    }
}


void print_program(){
	int i;
	uint32_t addr;

	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("\n\n[0x%x]\t", addr);
		print_instruction(addr);
	}
	puts( "\n" );
}

void print_instruction( uint32_t addr ){
//Get the current instruction
    uint32_t ins = mem_read_32( addr );
    //uint32_t ins = addr;
    //opcode MASK: 1111 1100 0000 0000 4x0000 = FC0000000
    uint32_t oc = ( 0xFC000000 & ins  );
	//printf( "BATNMA: %x\n", oc );

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
                    printf( "AND %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
                    break;
                    
                case 0X00000025:
                    //OR
                    printf( "OR %s, %s, %s", convert_Reg(rd), convert_Reg(rs), convert_Reg(rt));
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
            printf( "JAL %x", target );
            break;
        }
        case 0x08000000:
        {
            //J-Jump
            uint32_t target = ( 0x03FFFFFF & ins  );
            printf( "J %x", target);
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
                    printf( "ADDIU %s, %s, 0x%x", convert_Reg(rt), convert_Reg(rs), im );
                    break;
                }
                case 0xA0000000:
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
		case 0x28000000:
                {
                    //SLTI 
                    printf( "SLTI %s,%s 0x%x", convert_Reg(rt),convert_Reg(rs), im);
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
                    printf( "BEQ %s, %s, 0x%x", convert_Reg(rs), convert_Reg(rt), im );
                    break;
                }
                case 0x14000000:
                {
                    //BNE - Branch on Not Equal
                    printf( "BNE %s, %s, 0x%x", convert_Reg(rs), convert_Reg(rt), im );
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
  printf( "\nEX/MEM.A %x", EX_MEM.A );
  printf( "\nEX/MEM.B %x", EX_MEM.B );
  printf( "\nEX/MEM.ALUOutput %x", EX_MEM.ALUOutput );
  
  printf( "\n\nMEM/WB.IR %x ", MEM_WB.IR );
  print_instruction( MEM_WB.IR );
  printf( "\nMEM/WB.A %x", MEM_WB.ALUOutput );
  printf( "\nMEM/WB.B %x", MEM_WB.LMD );
  
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
CPU_Pipeline_Reg MEM_WB;[400070]	STALL COUNT: 1
BLEZ $t6

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
