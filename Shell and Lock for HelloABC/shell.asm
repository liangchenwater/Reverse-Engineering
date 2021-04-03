.386
code segment use16
assume cs:code
shell:
call next
next:
pop bx
sub bx, offset next-offset shell ;cs:bx is the addr of shell
mov bp, es  ;es==ds==the base addr of psp
add bp, 10h ;length of psp==100h bytes, bp is the addr of first segment
;1.1 calculate the length of the code to be decoded
mov cx, cs:[bx+head+4]
mov eax, 0
cal_length:
cmp cx, 0
je cal_length_end
add eax, 200h
dec cx
jmp cal_length
cal_length_end:
mov cx, cs:[bx+head+2]
cmp cx, 0
je  last_section_full
; if the last section is not full, length=200h*([head+4]-1)+[head+2]
movzx ecx, cx
add eax, ecx
sub eax, 200h
; if the last section is full, length=200h*[head+4]
last_section_full:
mov dx, cs:[bx+head+8]
movzx edx, dx
mov cl, 4
shl edx, cl
sub eax, edx ;length of program loaded in mem= length-length of exehead, stored in eax 
push es
mov  es, bp
mov di, 0
;1.2 decode, es:di shows the addr of the code
decode_next:
cmp eax, 0
je decode_end
xor byte ptr es:[di], 33h
cmp di, 0FFFFh
jne not_change_base_addr
mov dx, es
add dx, 1000h
mov es, dx ;in case the code is too long to be held in one segment
not_change_base_addr:
inc di
dec eax
jmp decode_next
decode_end:
mov cx, cs:[bx+head+6] ; cx==# of instructions to be relocated
cmp cx, 0
je reloc_end
mov si, cs:[bx+head+18h]
lea si, [bx+head+si] ;cs:si points to the relocation table
;2. relocation
reloc_next:
mov di, cs:[si] 
mov dx, cs:[si+2] ;(dx+bp):di is the addr of the byte to be relocated
add dx, bp
mov es, dx
add es:[di], bp
add si, 4
dec cx
jnz reloc_next
reloc_end:
pop es
;3. set ss:sp
mov cx, cs:[bx+head+0Eh]
add cx, bp
mov ss, cx
mov sp, cs:[bx+head+10h]
;4 set cs:ip
mov cx, cs:[bx+head+16h]
add cx, bp
push cx
mov cx, cs:[bx+head+14h]
push cx
retf
head label word
;+02 bytes in the last section
;+04 section occupied
;+06 #instructions to be relocated
;+08 the section length of the exehead
;+0E delta_ss
;+10 sp
;+14 ip
;+16 delta_cs
;+18 the offset addr of relocation table
;[+18] offset
;[+18]+2 base
;...
;...
code ends
end shell
