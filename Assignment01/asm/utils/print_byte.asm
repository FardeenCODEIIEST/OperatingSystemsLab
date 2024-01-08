
print_byte:
	pusha		; push all register contents in stack
	mov cx,0	; cx is counter
	
	print_byte_loop:
		cmp cx,2		; If all 4 character are checked
		je end_print_byte	; then go to hex_end
		
		mov ax,dx		
		and ax, 0x000f		; remove the first 3 hex digits
		add al, 0x30		; Since ASCII we need to add '0' to it, ascii of '0' is 0x30
		cmp al, 0x039		; if character is not number, then it must be 'A' to 'F'
		jle step_2		; It is a number so jump to step_2
		add al,7		; Since we have added 0x30 to the character, 'A' will have
					; ASCII value of 0x30+0x0A= 0x3A, but we know ASCII value
					; of 'A' is 0x41, so we add offset of 0x07
		
		step_2:
			mov bx,BYTE_OUT+1	; Address to store the result is BYTE_OUT
			sub bx, cx		; To select which letter we want to change in BYTE_OUT
			mov [bx], al		; al has the ascii value of the hex digit
			ror dx,4		; rotating right 4 bits dx register -> to get second last digit
			
			add cx,1		; increase the counter
			jmp print_byte_loop
	end_print_byte:
		mov bx, BYTE_OUT	; Load the BYTE_OUT in BX register
		call print_string
		
		popa			; restore all register
		ret

;; Data
BYTE_OUT:	db '00',0


