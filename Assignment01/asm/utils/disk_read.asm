;;; Standard Disk Function
;; Parameters in dx,bx,es

disk_read:
	pusha	; Saving all register values in stack
	push dx	; Saving content of dx register

	mov ah,0x02	; Setting for reading of disk sectors using int 0x13
	mov al,dh	; Number of sectors to read
	mov ch,0x0	; Setting track number/cylinder number to 0
	mov dh,0x00	; Setting head number to 0
	mov dl,0x00	; Setting drive to A
	
	int 0x13	; disk read sector interrupt
	
	pop dx		; restore content of dx register
	popa

	ret
