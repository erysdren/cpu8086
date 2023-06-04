[bits 16]
[org 0x0100]

[segment .text]

main:
	push str_hello				; pointer to string
	call print					; call print function
	mov dx, str_goodbye			; pointer to string
	call print					; call print function
	jmp exit

exit:
	mov ah, 4ch					; exit
	int 21h						; call dos

print:
	push bp						; store the current stack frame
	mov bp, sp					; preserve sp into bp for argument references
	mov dx, [bp+4]				; move the contents of bp+4 into dx
	mov ah, 9h					; print
	int 21h						; call dos
	mov sp, bp					; restore the stack
	pop bp						; restore bp
	ret							; return to caller

[segment .data]

str_hello: db 'hello, world',13,10,'$'
str_goodbye: db 'goodbye, world',13,10,'$'
