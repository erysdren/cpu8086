[bits 16]
[org 0x0100]

[segment .text]

	mov ah, 9
	mov dx, hello
	int 21h
	jmp exit
	mov ah, 9
	mov dx, nope
	int 21h

exit:
	mov ah, 9
	mov dx, end
	int 21h
	mov ah, 76
	int 21h

[segment .data]

hello: db 'hello, world',13,10,'$'
end: db 'reached exit',13,10,'$'
nope: db 'didnt reach exit',13,10,'$'
