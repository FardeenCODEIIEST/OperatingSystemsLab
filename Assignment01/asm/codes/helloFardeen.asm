[org 0x7c00]	; Starting address of the boot sector
[bits 16]	; 16-bit real mode

mov bx, str
; Starting address of the string in bx register
call print_string

jmp $

%include "../utils/print_string.asm"

str:
	db 'Hello , I am Fardeen',13,10,0

times 510-($-$$) db 0 
dw 0xaa55	; Boot Signature
