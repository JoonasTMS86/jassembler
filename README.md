# jassembler
JAssembler is my own Assembler which can assemble an Assembly source file into any microprocessor binary.
While there are enough existing Assemblers already, I created this program in order to be able to assemble my own JISA (Joonas Instruction Set Architecture) Assembly code into JISA binary code.

My Assembler can actually assemble source into any machine code, with JISA being just one example.
For example, it can assemble 8-bit code such as 6502, Z80, etc.
All you have to do is provide JISA with a text file which contains the opcodes for the machine code that you wish to assemble your source code into.

I have included the file "Instruction Set.txt", which contains the opcodes and textual descriptions of the JISA architecture.
I have also included the example source file, "example.asm", which you can assemble into JISA binary with JAssembler.
Source files can be assembled by entering the name of the instruction set, source file and target filename in the command line.

For example:
To assemble "example.asm" into a file named "result" using "Instruction Set.txt":

<code>jassembler "Instruction Set.txt" example.asm result</code>

JAssembler is a Work-In-Progress program. Variables, labels and other source code features are not available yet. I will implement them in future versions of JAssembler. My initial goal was to create a program with which I can assemble some JISA source into JISA binary programs, so that I can use them in my website - it is in my plans to make my own JISA Virtual Machine to my website which executes JISA code.

2021 Joonas Lindberg, The Mad Scientist
