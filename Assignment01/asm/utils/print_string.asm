; Starting Address in BX register

print_string:
	pusha	; push all register values to stack

	mov ah,0x0e	; setting in teletype mode	
	loop1:

		mov al,[bx]
		cmp al,0	; if 0 then we have finished string parsing
		je end_print

		int 0x10	; video mode interrupt
		inc bx		; increment the pointer
		jmp loop1

	end_print:	
		popa	; restoring all register values
		ret
