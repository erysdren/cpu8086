[bits 16]
[org 0x0100]

[segment .text]

main:
	mov dx, str_hello			; pointer to string
	call print					; call print function
	mov dx, str_goodbye			; pointer to string
	call print					; call print function
	jmp exit

exit:
	mov ah, 4ch					; exit
	int 21h						; call dos

print:
	mov ah, 9h					; print
	int 21h						; call dos
	ret							; return to caller

[segment .data]

str_hello: db 'hello, world',13,10,'$'
str_goodbye: db 'goodbye, world',13,10,'$'
