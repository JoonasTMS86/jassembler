; This 6502 example is a C64 program which puts the word "HI" in PETSCII characters to the top-left corner of the screen
; and then flashes the border.
	LDA #$08
	STA $0400
	LDA #$09
	STA $0401
	INC $D020
	JMP $080B
