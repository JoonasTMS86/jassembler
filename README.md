# jassembler
JAssembler is my own Assembler which can assemble an Assembly source file into any microprocessor binary.
While there are plenty of existing Assemblers already, I created this program in order to be able to assemble my own JISA (Joonas Instruction Set Architecture) Assembly code into JISA binary code.

My Assembler can actually assemble source into any machine code, with JISA being just one example.
For example, it can assemble code of older 8-bit CPUs such as 6502, Z80, etc.
All you have to do is provide JAssembler with a text file which contains the opcodes for the machine code that you wish to assemble your source code into.

I have included the file "Instruction Set.txt", which contains the opcodes and textual descriptions of the JISA architecture.
I have also included the example source file, "example.asm", which you can assemble into JISA binary with JAssembler.
Source files can be assembled by entering the filename of the instruction set to be used, source file and target filename in the command line.

For example:
To assemble "example.asm" into a file named "result" using "Instruction Set.txt":

<code>jassembler "Instruction Set.txt" example.asm result</code>

To assemble the included example 6502 ASM source to a 6502 binary, use:

<code>jassembler 6502.txt 6502.asm 6502program</code>

JAssembler is a Work-In-Progress project. My initial goal was to create a program with which I can assemble some JISA source into JISA binary programs, so that I can use them in my website - it is in my plans to make my own JISA Virtual Machine to my website which executes JISA code. Many features, such as expressions, are not supported yet. I shall implement missing features in future versions of JAssembler.

2021 - 2025 Joonas Lindberg, The Mad Scientist
