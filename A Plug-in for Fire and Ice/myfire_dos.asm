
dgroup group data, code
data segment
old_addr dw 0, 0
addr dw 0, 0
old_1h dw 0, 0
old_21h dw 0, 0
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
	lds bx, dword ptr cs:[psp_addr]
	mov ax, ds:[bx]
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
	call getpsp
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

getpsp proc near
	push ax
	push bx
	push dx
	push es
	mov ah, 62h
	int 21h
	mov [psp], bx ;
	xor ax, ax
	mov es, ax
	mov bx, 4
	push es:[bx]
	pop old_1h[0]
	push es:[bx+2]
	pop old_1h[2] ;save old int_1h
	mov word ptr es:[bx], offset int_1h
	mov es:[bx+2], cs ;set new int_1h
	
	mov bx, 21h*4
	push es:[bx]
	pop old_21h[0]
	push es:[bx+2]
	pop old_21h[2] ;save old int_21h
	
	pushf
	push cs
	mov ax, offset return_here
	push ax
	mov ah, 62h ;get psp function
	pushf
	pop dx ;DX=FL
	or dx, 0100h
	push dx
	popf ;enable TF
	
	jmp dword ptr [old_21h]; after this instruction, int_1h will be executed
	;int 21h/ah=62h to get psp
	
return_here:
	mov bx, 4
	push old_1h[0]
	pop es:[bx]
	push old_1h[2]
	pop es:[bx+2]
	pop es
	pop dx
	pop bx
	pop ax
	ret
getpsp endp
	
int_1h:
	push bp
	mov bp,sp
	push ax
	push di
	push ds
	push es
	mov ax,data
	mov ds,ax
	push addr[0]
	pop old_addr[0]
	push addr[2]
	pop old_addr[2]
	push [bp+2]
	pop addr[0]; addr[0]=ip
	push [bp+4]
	pop addr[2]; addr[2]=cs
	cmp bx, [psp]
	jne int_1h_iret
found_psp:
	les di, dword ptr [old_addr]
	mov ax, es:[di+2] ;0330=old cs:[old ip+2]
	mov cs:psp_addr[0], ax
	mov ax, [bp-6] ;old DS
	mov cs:psp_addr[2], ax ; get psp_addr old ds:[0330]
	mov ax, [bp+6] ; TF
	and ax, not 100h ; to disable TF
	mov [bp+6], ax
int_1h_iret:
	pop es
	pop ds
	pop di
	pop ax
	pop bp
	iret
code ends
end main

	