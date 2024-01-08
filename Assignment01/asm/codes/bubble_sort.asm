[org 0x9000]	; Starting address of the second sector
[bits 16]	; Setting 16 bit real mode

jmp bubble_sort

;; print array utility
;;; Starting address in SI register and size of the array in CX register
print_array:
        mov cx, word[size]
	lea si,arr
        parse_loop:
                xor dx,dx
                mov dl,[si]
                call print_byte
                mov bx,SPACE
                call print_string
                inc si          ; increment the pointer
                loop parse_loop ; until cx is 0 (dec cx)
        ret

;; Code
bubble_sort:

	mov bx,MSG_2
	call print_string
	; Initial state of the array
	mov bx, before_message
	call print_string
	call print_array

	mov dx, word[size]
	outer_loop:
		mov cx,dx
		dec cx
		jz end_outer_loop ; Nothing more to compare array is sorted
		lea si,arr	 ; si points to the first element of the array
		
		inner_loop:
			mov al,[si]
			cmp al,[si+1]
			jl skip
			; bubble down the largest element
			xchg al,[si+1]
			mov [si],al
			
			skip:
				inc si
				loop inner_loop	; if cx is not 0, then jmp to inner_loop

		; One inner loop round is done
		dec dx	
		jnz outer_loop
	end_outer_loop:
		mov bx, after_message
		call print_string
		call print_array
		jmp $

;; Data
%include "../utils/print_byte.asm"
%include "../utils/print_string.asm"

before_message: db 'Original arary(Hexadecimal) is: ',13,10,0
after_message:	db 13,10,'Sorted array(Hexadecimal) is: ',13,10,0
arr:		db 8,2,1,3,4,44,2,1,9,10
size:		dw 0x0A
SPACE:		db ' ',0
MSG_2:		db 'Hello from the Kernel',13,10,0
times 512-($-$$) db 0	; padding rest of the space with 0

		 
		
		
		
		
