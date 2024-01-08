[org 0x7c00]	; setting starting address to be 0x7c00
[bits 16]	; setting mode to 16 bit real mode

; Initialising stack
mov bp,0x8000
mov sp,bp

;Read from the second sector
mov bx, 0x9000	; ES:BX --> pointer to buffer
mov dh, 2	; 
mov cl, 2	; Number of sectors to read is 2 : boot sector and sector containing bubble_sort
call disk_read


; Far jump to second sector
jmp 0x9000

jmp $

%include "../utils/print_string.asm"
%include "../utils/disk_read.asm"

times 510 -($-$$) db 0 	; Padding with 0 
dw 0xaa55

