/*
 * AESOP.c
 * 15 March 2017
 * 
 * This is the program flow:
 *
 * (INPUT) -> User-written program with comments, probably messed up
 * | (lexer removes comments and whitespace, translates opcodes)
 * Lexed program, free of comments and whitespace
 * | (parser parses out instructions plus following args, separated by spaces and newlines
 * Parsed instructions, ready to run
 * | (executor executes the instructions plus args one at a time)
 * 
 * Once lexed (eliminate whitespace and comments, convert to byte code), the program can execute.
 * Once the executor is passed the bytecode and the entry offset, the games can begin.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "aesop_definitions.h"

char 			aesop_context(char a);
uint16_t 	aesop_ascii_to_inst(char * unlexed_buf_ptr);
char * 		aesop_inst_to_args(uint16_t inst_value);
uint16_t 	aesop_ascii_to_reg(char * unlexed_buf_ptr);		//turns the register into whitespace as side effect
uint16_t 	aesop_ascii_to_lit(char * unlexed_buf_ptr);	//turns the literal into whitespace as side effect
struct 		token;
uint16_t 	aesop_lexer(struct token * token_head, char * unlexed_buf, uint32_t unlexed_len);
void 			print_tokens(struct token * mytokens, int n);
int				free_tokens(struct token * mytoken);
void 			print_registers(uint16_t * registers);
void print_instructions(uint32_t * instructions, uint16_t print_offset, uint16_t print_len);
struct token * append_new_token(struct token * mytoken);
uint16_t  aesop_parser(uint32_t * inst_buf, struct token * token_head);
uint16_t aesop_execute(uint32_t * inst_buf, uint16_t entry_offset);
uint8_t aesop_sets_flags(uint8_t inst);
uint8_t aesop_jumps(uint8_t inst);

struct token	//this is a linked list struct for tokens.
{
	char type;
	uint16_t value;
	struct token * next;
};


int main(int argc, char * argv[])
{
	if(argc < 2)
	{
		printf("Correct usage: %s path/of/program.aesop\n", argv[0]);
		exit(1);
	}
	/* open the file and read it into memory. */
	const char * unlexed_filename = argv[1];
	FILE * unlexed_fd = fopen(unlexed_filename, "r");
	if (unlexed_fd == 0)
	{
		printf("Could not find specified file. Quitting.\n");
		exit(1);
	}
	char *unlexed_buf = malloc(bitesize);		//this holds the user program.
	uint16_t unlexed_buf_len = fread(unlexed_buf, 1, bitesize, unlexed_fd);
	fclose(unlexed_fd);
	printf("Unlexed file read into memory; Length: %i bytes.\n", unlexed_buf_len);
	
	struct token * token_head = malloc(sizeof(struct token));
	struct token * token_ptr  = token_head;

	if(token_head==NULL) printf("Error allocating token_head.\n");
	
	//LEXER OPERATIONS
	uint16_t lexed_buf_len = aesop_lexer(token_head, unlexed_buf, unlexed_buf_len);
	free(unlexed_buf);
	
	printf("\nLexer output:\n");
	print_tokens(token_head, 100);

	//TOKEN OPERATIONS
	uint32_t * inst_buf = malloc(memory_space);	//allocate 64k of memory for this bad boy
	uint16_t inst_buf_len = aesop_parser(inst_buf, token_head);
	printf("\nThe parser output the following %i instructions: \n", inst_buf_len);
	print_instructions(inst_buf, (uint16_t) 0, inst_buf_len);

	aesop_execute(inst_buf, 0);
	

	printf("Attempting to free memory.\n");
	printf("Freed memory from %i tokens.\n",free_tokens(token_head));
	free(inst_buf);
	exit(0);
}

struct token * append_new_token(struct token * mytoken)
{
	struct token * newtoken = malloc(sizeof(struct token)); 
	newtoken->type = 'q';
	newtoken->value = 0x0000;
 	newtoken->next = NULL;
	mytoken->next = newtoken;
	return newtoken;
}

int free_tokens(struct token * mytoken)
{
	struct token * next_token = NULL;
	int i = 1;
	while(mytoken!=NULL)
	{
		next_token = mytoken->next;
		free(mytoken);
		mytoken = next_token;
		i++;
	}
	return i-1;
}		

void print_tokens(struct token * mytoken, int n)
{
	int i = 0;
	printf("Token type	value\n=====================\n");
	while(mytoken!=NULL && i < n)
	{
		printf("%c\t\t0x%04x\n", mytoken->type, mytoken->value);
		mytoken = mytoken->next;
		i++;
	}
}


uint16_t aesop_lexer(struct token * token_head, char * unlexed_buf, uint32_t unlexed_buf_len)
{
	char * unlexed_buf_ptr = unlexed_buf;
	struct token * token_ptr = token_head;

	while(unlexed_buf_ptr < unlexed_buf + unlexed_buf_len)
	{
		switch(aesop_context(*unlexed_buf_ptr))
		{
			case 'i':
				token_ptr->type = 'i';
				token_ptr->value = aesop_ascii_to_inst(unlexed_buf_ptr);
				token_ptr = append_new_token(token_ptr);
				//advance until find a non-instruction character
				while(unlexed_buf_ptr < unlexed_buf + unlexed_buf_len && \
						(aesop_context(*unlexed_buf_ptr)=='i' || aesop_context(*unlexed_buf_ptr)=='x'))
				{
					unlexed_buf_ptr++;
				}
				break;

			case 'r':
				token_ptr->type = 'r';
				token_ptr->value = aesop_ascii_to_reg(unlexed_buf_ptr);
				token_ptr = append_new_token(token_ptr);
				//advance until find a non-register and non-literal char
				while(unlexed_buf_ptr < unlexed_buf + unlexed_buf_len && \
						(aesop_context(*unlexed_buf_ptr)=='r' ||  aesop_context(*unlexed_buf_ptr)=='l'))
				{
					unlexed_buf_ptr++;
				}
				break;

			case 'l':
				token_ptr->type = 'l';
				token_ptr->value = aesop_ascii_to_lit(unlexed_buf_ptr);
				token_ptr = append_new_token(token_ptr);
				//advance until find a non-literal char
				while(unlexed_buf_ptr < unlexed_buf + unlexed_buf_len && \
						 aesop_context(*unlexed_buf_ptr)=='l')
				{
					unlexed_buf_ptr++;
				}
				break;

			case 'c':
				//advance until newline
				while(aesop_context(*unlexed_buf_ptr)!='n')
				{
					unlexed_buf_ptr++;
				}
				break;

			case 'n':
				unlexed_buf_ptr++;
				break;

			case 'w':
				unlexed_buf_ptr++;
				break;
		}
	}
	return 0;
}

char aesop_context(char a)
{
	/*	returns a character representing the type of thing we are lexing. 
	 *
	 *	i = instruction
	 *	r = register
	 *	l = literal
	 *	c = comment
	 *	w = whitespace
	 *	x = error
	 */

	if(a=='n'||a=='a'||a=='A'||a=='s'||a=='S'||a=='^'||a=='|'||a=='&'||a=='m'||a=='q'||a=='j') 
	{
		//if(debug) printf("%c is an instruction.\n",a);
		return 'i';
	}
	if(a=='r')
	{
		//if(debug) printf("%c is a register.\n",a);
		return 'r';
	}
	if(a=='1'||a=='2'||a=='3'||a=='4'||a=='5'||a=='6'||a=='7'||a=='8'||a=='9'||a=='0')
	{
		//if(debug) printf("%c is a literal\n", a);
		return 'l';
	}
	if(a==';')
	{
		//if(debug) printf("%c is a comment.\n", a);
		return 'c';
	}
	if(a==0x20||a=='\t'||a==',')
	{
		//if(debug) printf("whitespace.\n");
		return 'w';
	}
	if(a=='\n')
	{
		//if(debug) printf("Newline.\n");
		return 'n';
	}
	return 'x';	
}

uint16_t aesop_ascii_to_inst(char * unlexed_buf_ptr)
{
 /* this function returns the instruction's opcode.
	*/
		if(debug) printf("ASCII_TO_INST: found instruction %c\n", *unlexed_buf_ptr);
		uint16_t returnme;
		switch(*unlexed_buf_ptr)
		{
			case 'n':
				returnme = opcode_n;
				break;
			case 'a':
				returnme = opcode_a;
				break;
			case 'A':
				returnme = opcode_A;
				break;
			case 's':
				returnme = opcode_s;
				break;
			case 'S':
				returnme = opcode_S;
				break;
			case '^':
				returnme = opcode_xor;
				break;
			case '|':
				returnme = opcode_or;
				break;
			case '&':
				returnme = opcode_and;
				break;
			case 'm':
				returnme = opcode_m;
				break;
			case 'q':
				returnme = opcode_q;
				break;
			case 'j':
				if(*(unlexed_buf_ptr + 1) == 'z')
				{
					returnme = opcode_jz;	//jz instruction
				}
				else
				{
					returnme = opcode_j;
				}
				break;
		}
		return (uint16_t)returnme;
}


char * aesop_inst_to_args(uint16_t inst_value)
{
		switch(inst_value)
		{
			case opcode_n:
				return &args_n[0];
				break;
			case opcode_a:
				return &args_a[0];
				break;
			case opcode_A:
				return &args_A[0];
				break;
			case opcode_s:
				return &args_s[0];
				break;
			case opcode_S:
				return &args_S[0];
				break;
			case opcode_xor:
				return &args_xor[0];
				break;
			case opcode_or:
				return &args_or[0];
				break;
			case opcode_and:
				return &args_and[0];
				break;
			case opcode_m:
				return &args_m[0];
				break;
			case opcode_q:
				return &args_q[0];
				break;
			case opcode_j:
				return &args_j[0];
				break;
			case opcode_jz:
				return &args_jz[0];
				break;
		}
}

uint16_t aesop_ascii_to_reg(char * unlexed_buf_ptr)	//this only handles single digit registers
{
	/* this function returns the number of chars it interpreted to create a single-byte register reference. */
	if(* unlexed_buf_ptr != 'r')
	{
		printf("Error: Register literal not preceded by 'r' character. Exiting.");
		exit(1);
	}
	*unlexed_buf_ptr = 0x20;
	unlexed_buf_ptr++;
	uint8_t literal_value = (uint8_t)*unlexed_buf_ptr - 0x30;
	*unlexed_buf_ptr = 0x20;
	if(debug) printf("Register literal is %i\n", (int)literal_value);
	return literal_value;
}

uint16_t aesop_ascii_to_lit(char * unlexed_buf_ptr)
{
	//grab all of the integers. Each time you find one, multiply the rest by 10 and add in the newer one.
	uint16_t literal_value = 0; 
	while(aesop_context(*unlexed_buf_ptr)=='l')
	{
		literal_value = (literal_value*10) + (*unlexed_buf_ptr - 0x30);
		*unlexed_buf_ptr = 0x20;
		unlexed_buf_ptr++;
	}
	if(debug) printf("Read literal as %03i.\n", literal_value);
	return literal_value;
}


uint16_t aesop_parser(uint32_t * inst_buf, struct token * mytoken)
{
	uint32_t * inst_buf_ptr = inst_buf;

	if(parser_debug) printf("\nParsing tokens...\n");

	while(mytoken!=NULL)
	{
		if(mytoken->type == 'i')
		{
			if(parser_debug) print_tokens(mytoken, 1);
			char * args_string = aesop_inst_to_args(mytoken->value);
			if(parser_debug) printf("String of args: %s\n", args_string);
			
			uint32_t build_instruction = (uint32_t)(mytoken->value) << 24;	//set the instruction value in bits 32-16
			uint8_t instruction_index = 2;	//counts how far along building instruction we are

			for(args_string; *args_string != 0; args_string++)	//iterate through the args string
			{
				if(mytoken->next != NULL) mytoken = mytoken->next;
				else
				{
					printf("PARSER: Unexpectedly reached end of token linked list. \n");
					exit(1);
				}

				if(mytoken->type == 'r' && *args_string == 'r')
				{
					if(parser_debug) printf("Found a register token.\n");
					build_instruction |= ((uint32_t)(mytoken->value) << (32 - (8*instruction_index)));
					instruction_index++;
				}
				else
				{
					if(mytoken->type == 'l' && *args_string == 'l')	//literals are always up to 16 bits long
					{
						if(parser_debug) printf("Found a literal token.\n");
						uint32_t build_instruction_add = ((uint32_t)(mytoken->value) << (24 - (8*(instruction_index))));
						printf("PARSER: Printing the following into the instruction: 0x%04X\n", build_instruction_add);
						build_instruction |= ((uint32_t)(mytoken->value) << (24 - (8*(instruction_index))));
						instruction_index+=2;
					}
					else
					{
						printf("PARSER: Expected %c argument, but found %c.\n", *args_string, mytoken->type);
					}
				}
			}
			if(parser_debug) printf("Built instruction: 0x%08x\n",build_instruction);
			* inst_buf_ptr = build_instruction;
			inst_buf_ptr++;
			if(parser_debug) printf("\n");
		}
		mytoken = mytoken->next;
	}
	
	return( (uint16_t)(inst_buf_ptr - inst_buf));
}



uint16_t aesop_execute(uint32_t * inst_buf, uint16_t entry_offset)
{
	//define the registers
	uint16_t * aesop_registers = malloc(8);
	for(uint8_t ctr=0; ctr < 8; ctr++) aesop_registers[ctr]=0;
	aesop_registers[reg_ip] = entry_offset;
	if(exec_debug) printf("\nEXECUTE: Set up registers.\n");
	if(exec_debug) print_registers(aesop_registers);
	uint32_t * current_ip;	
	
	while(aesop_registers[reg_ip] < memory_space )//8 )//8 )//8 )//8 )//8 )//8 )//8 )//(uint64_t)(inst_buf + 8)) //memory_space))	//exit of program leaves program bounds
	{
		current_ip = inst_buf + aesop_registers[reg_ip];
		if(exec_debug) printf("Current instruction: %08X\n", (*current_ip));
		uint8_t quit_now = 0;

		uint8_t inst_bytes[4];
		for(int i=0;i<4;i++)
		{
			inst_bytes[i] = (uint8_t)(*current_ip >> (24 - i*8));
		}
		


		switch(inst_bytes[0])	//Look at the first byte (the instruction)
		{
			case(0x00): //nop
				break;
			case(0x01): //a
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] + aesop_registers[inst_bytes[2]];

				break;
			case(0x02):	//A
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] + (inst_bytes[2] * 256) + inst_bytes[3];
				break;
			case(0x03):	//s
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] - aesop_registers[inst_bytes[2]];
				break;
			case(0x04):	//S
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] - ((inst_bytes[2] * 256) + inst_bytes[3]);
				break;
			case(0x05): // ^
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] ^ aesop_registers[inst_bytes[2]];
				break;
			case(0x06): // |
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] | aesop_registers[inst_bytes[2]];
				break;
			case(0x07): // &
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[1]] & aesop_registers[inst_bytes[2]];
				break;
			case(0x08): // mov
				aesop_registers[inst_bytes[1]] = aesop_registers[inst_bytes[2]];
				break;
			case(0x09): // quit
				printf("\n\n>>>>>> Done <<<<<<\n\n");
				quit_now = 1;
				break;
			case(0x0a):	//jump always
				aesop_registers[reg_ip] = (inst_bytes[1] * 256) + inst_bytes[2];
				break;
			case(0x0b):	//jump zero 
				if(aesop_registers[reg_fl])
				{
					aesop_registers[reg_ip] = (inst_bytes[1] * 256) + inst_bytes[2];
					if(exec_debug) printf("Jump zero!\n");
				}
				else
				{
					if(exec_debug) printf("No jump zero.\n");
					aesop_registers[reg_ip]++;
				}
				break;
			default:
				printf("Instruction not recognized. Something went wrong.\n");
				break;
		}
		if(quit_now) return 0;

		if(aesop_sets_flags(inst_bytes[0]))
		{
			if(!aesop_registers[inst_bytes[1]]) aesop_registers[reg_fl] = 1;
			else aesop_registers[reg_fl] = 0;
			if(exec_debug) printf("Setting flags to 0x%04X\n", aesop_registers[reg_fl]);
		}
		else
		{
			aesop_registers[reg_fl] = 0x0000;
		}

		if(!aesop_jumps(inst_bytes[0])) aesop_registers[reg_ip] ++;	//go to next instruction lol
		
		print_registers(aesop_registers);

		if(quit_now) break;
		
	}

	free(aesop_registers);
	return 0;
}

uint8_t aesop_sets_flags(uint8_t inst)
{
	if(inst > 0x00 && inst < 0x09) return 1;
	else return 0;
}

uint8_t aesop_jumps(uint8_t inst)
{
	if(inst == 0x0a || inst == 0x0b) return 1;
	else return 0;
}



void print_registers(uint16_t * registers)
{
	printf("Register values: A    B    C    D    E    BP   IP   FL\n                 ");
	int ctr = 0;
	for(ctr; ctr < 8;ctr++)
	{
		printf("%04X ",registers[ctr]);
	}
	printf("\n\n");
}

void print_instructions(uint32_t * instructions, uint16_t print_offset, uint16_t print_len)
{
	printf("Address		Contents \n========================\n");
	for(uint16_t i = print_offset; i < print_offset + print_len ; i++)
	{
		printf("%08x	%08X\n", i, *(instructions + i));
	}
	printf("\n");
}






