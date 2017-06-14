/*
 * I should make a struct that defines every fucking thing about each instruction.
 * There will be an array of structs indexed by the instruction's coded value (i.e. 0x0b for jump)
 *
 * The struct's elements will include: arguments string, sets IP (i.e. if it jumps) boolean, sets flags boolean, char * representation string (for lookups)...
 *
 * That should free up some things and make looking things up much simpler. 
 *
 */

#ifndef AESOP_DEFINITIONS_H_
#define AESOP_DEFINITIONS_H_
//protector function


#include <stdint.h>
#include <stdlib.h>

#define debug 1
#define parser_debug 1
#define exec_debug 1
#define bitesize 10000
//#define memory_space 65536
#define memory_space 1000

/************* REGISTERS ************/
#define reg_r0 0x00
#define reg_r1 0x01
#define reg_r2 0x02
#define reg_r3 0x03
#define reg_r4 0x04
#define reg_r5 0x05
#define reg_r6 0x06
#define reg_r7 0x07

//aliases
#define reg_a 0x00
#define reg_b 0x01
#define reg_c 0x02
#define reg_d 0x03
#define reg_e 0x04
#define reg_bp 0x05
#define reg_ip 0x06
#define reg_fl 0x07

/************* MNEMONIC TO OPCODE ************/
#define opcode_n 0x00
#define opcode_a 	0x01
#define opcode_A 	0x02
#define opcode_s	0x03
#define opcode_S	0x04
#define opcode_xor	0x05
#define opcode_or		0x06
#define opcode_and	0x07
#define opcode_m	0x08
#define opcode_q	0x09
#define opcode_j	0x0a
#define opcode_jz	0x0b


/*Defining args in this thing:
 * 	r = register
 * 	l = literal
 */

char args_n[] = "";
char args_a[] = "rr";
char args_A[] = "rl";
char args_s[] = "rr";
char args_S[] = "rl";
char args_xor[] = "rr";
char args_or[] = "rr";
char args_and[] = "rr";
char args_m[] = "rr";
char args_q[] = "";
char args_j[] = "l";
char args_jz[] = "l";

/************* OPCODE FLAGS DEFINES ************/
#define flags_n	1
#define flags_a	1
#define flags_A	1
#define flags_s	1
#define flags_S	1
#define flags_xor	1
#define flags_or	1
#define flags_and	1
#define flags_m	1
#define flags_q	0
#define flags_j	0
#define flags_jz	0


/************* OPCODE JUMP DEFINES ************/
#define jump_n	0
#define jump_a	0
#define jump_A	0
#define jump_s	0
#define jump_S	0
#define jump_xor	0
#define jump_or	0
#define jump_and	0
#define jump_m	0
#define jump_q	1
#define jump_j	1
#define jump_jz	1

#endif	//end of protector
