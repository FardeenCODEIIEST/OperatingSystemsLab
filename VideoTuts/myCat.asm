global _start

section .text

_start:
	mov rax,2		; open syste call number
	mov rdi, [rsp+16]	; stack has access to arguments passed into program
	mov rsi, 0
	mov rdx, 0
	syscall
	; Use sendFile syscall
	mov rsi, rax
	mov rdi, 1
	mov rax, 40
	mov rdx, 0
	mov r10, 256	; restricting byte size to 256
	syscall
	jmp $
	
	
