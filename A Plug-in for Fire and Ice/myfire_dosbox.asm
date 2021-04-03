dgroup group data, code
data segment
psp dw 0
data ends

code segment
assume cs:code, ds:data
int_8h:
	cmp cs:[fixed], 1
	je goto_old_8h ;if the code in fire is modified, go to old int_8h
	push ax
	push bx
	push ds
	mov ah,62h
	int 21h
	mov ax,bx
	add ax,10h ;get code segment addr
	mov ds,ax
	cmp dword ptr ds:[0AB85h], 0741247FFh
	jne skip ;if code at AB85 is not inc word [bx+12]
	cmp byte ptr ds:[0AB89h], 01Fh
	jne skip ;if code at AB88 is not je ABA9
	mov word ptr ds:[0AB85h], 9090h
	mov byte ptr ds:[0AB87h], 90h ; change inc word [bx+12] to nop
	mov cs:[fixed], 1
skip:
	pop ds
	pop bx
	pop ax
goto_old_8h:
	jmp dword ptr cs:[old_8h]
old_8h dw 0,0
psp_addr dw 0,0
fixed db 0	
	
main:
	mov ax,data
	mov ds,ax
	xor ax,ax
	mov es,ax
	mov bx, 8*4
	push es:[bx]
	pop cs:old_8h[0]
	push es:[bx+2]
	pop cs:old_8h[2]
	cli
	mov ax,offset int_8h
	mov es:[bx], ax
	mov es:[bx+2], cs
	sti
	mov dx, offset dgroup:main
	add dx,100h
	add dx, 0Fh
	mov cl, 4
	shr dx, cl ;len=(len+15)/16
	mov ah,31h
	int 21h

code ends
end main
