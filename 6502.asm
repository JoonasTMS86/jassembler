; 6502 demonstration source file.

something = $0A

	.ORG $0900
	LDA #something
	BNE label
	LDA #$00
label
	STA $D020
	JMP *

	.BYTE $31,$EA,$41
	.TEXT "Hello world."
