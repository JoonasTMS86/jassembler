; This is an example of a JISA program. In my JISA Virtual Machine, 0x0 to 0x5EEBFF represents the RGB screen.
; Pixels are put to said screen by simply writing to that area.
; In this example, a red pixel is put to the top-left corner of the screen by writing the 24-bit value $FF000 (R = $FF, G = $00, B = $00)
; to 0x0 - 0x2.

colour = $FF0000

	LOA AX,#colour
	FLOA $0,AT ; AT = write 24-bit value to destination
	JMP 0
