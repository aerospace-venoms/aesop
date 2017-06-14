The following is documentation on the AESOP language.

AESOP stands for A<my name>' Extremely Simple Ocular Program. It is an 8-register, 16-bit virtual machine,
along with a lexer for interpreting your assembly-style AESOP programs. The instructions are a fixed
width of 32 bits, where the first byte is the opcode, leaving 24 bits for args.

Being a 16-bit machine, AESOP deals with an address space with 2^16 addresses, each of which holds
32 bits, or one full instruction. Thus, AESOP can work with a memory space up to 64KB. 


aesop.c creates and runs byecode from AESOP assembly programs. AESOP program syntax looks like this: 

a r1 r2	;this is a comment.
A r1 44	;the third argument on this line is a literal value of 44.
^ r1 r3
j 124		;this jumps to address 124.

There are various operation codes (opcodes) for different functions. Opcodes will be detailed later.
The character 'r' precedes a register for human readability. Registers will be detailed later.
Naked numerals (no preceding 'r') are interpreted as literal values.
Note that different arguments use different combinations of literals and registers for similar operations.
For example, 'a' takes two resters as arguments, while 'A' takes a register and a literal.
You may delimit your arguments using any combination of newlines, tabs, spaces, or commas.
You must put at least one whitespace delimiter character between each of your arguments.
Comments begin with a ; character and last until a newline.


======================REGISTERS======================

The 8 registers correspond to the following:
REGISTER	|	USAGE
----------+--------------------
r0				|	user register a
r1				|	user register b
r2				|	user register c
r3				|	user register d
r4				|	user register e
r5				|	base pointer
r6				|	instruction pointer
r7				|	opcode flags

r0-r4 are free for you to use in your program.
r5-r7 are reserved for program use. Write to these at your own risk (here be dragons)
The flags in r7 are set following the execution of most opcodes. More information on flags later.


======================OPERATION CODES======================

The following details the opcodes used in AESOP programs.

MNEMONIC is the representation of the opcode that you use when writing your own .aesop program.
OPCODE is the byte-length operation code used in AESOP machine code.
ARG* details the arguments that the opcode uses when human-written.
	ARGn is the nth argument used for each operand.
	In interpreted instructions, all instructions (operator + operands)
USAGE details how one might use this operator. Unless otherwise specified, appropriate flags are set.

MNEMONIC	|	OPCODE	|	ARG1	ARG2	| USAGE "EXAMPLE" (effects)
----------+---------+-------+----------------------------------------------------
n					|	0x00		| n/a		n/a	|	Null operation: "r" (no registers changed. flags set to 0x00)
a					|	0x01		|	r			| r	Add registers: "a r1 r2" (r1 = r1 + r2)
A					|	0x02		|	r	l			| Add literal: "A r1 99" (r1 = r1 + 99)
s					|	0x03		|	r r		| Subtract registers: "s r1 r2" (r1 = r1 - r2)
S					|	0x04		|	r l		| Subtract literal: "S r1 88" (r1 = r1 - 88)
^					|	0x05		|	r r		| XOR registers: "^ r1 r2" (r1 = r1 ^ r2)
|					|	0x06		|	r r		| OR registers: "| r1 r2"	(r1 = r1 | r2)
&					|	0x07		|	r r		| AND registers: "& r1 r2" (r1 = r1 & r2)
m					|	0x08		|	r r		| MOV registers: "m r1 r2" (r2 = r1)
q					|	0x09		|	none		| Quit: Exits the program. No flags.
j					| 0x0a		| address			| Jump: Always sets the the instruction pointer register to the 
					|					|				|		offset stored in the second arg. "j r1" No flags.
jz				| 0x0b		| address			|	Jump if zero: if the ones bit of FLAGS is set, jumps to offset in second arg.
					|					|				|		Otherwise, does nothing and moves to the next instruction in memory. No flags.


flags are set for all instructions except n, m, q, j, and jz. 
If flags are set, they are set to equal the last specified "to" register (i.e. the first arg of the last valid
flag instruction). They are set to 0x0000 for all other operations.
Flags ones place indicates a "zero" write on last operation iff the ones place is set to 1. 


in the future I need to define structs with the following characteristics:
instruction(a):
	int number_of_arguments
	uint8_t opcode
	uint8_t sets_flags
	uint8_t sets_ip
