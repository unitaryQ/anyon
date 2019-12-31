[bits 16]

org 0x7c00
E820MAGIC EQU 0x534d4150

boot:
cli
xor ax,ax
mov ds,ax
mov ss,ax
mov es,ax
mov sp,0x7c00

detect_map:
mov dword [ds:KLAYOUT], 0x0
mov edx, E820MAGIC
mov edi,KLAYOUTE
xor ebx,ebx

detect_map_1:
mov eax,0xe820
mov ecx,0x18
int 0x15
cmp eax, E820MAGIC
jz sign_ok
hlt

sign_ok:
cmp ebx,0
jz load_gdt
mov eax,[ds:KLAYOUT]
inc eax
mov [ds:KLAYOUT],eax
add edi,0x18
jmp detect_map_1

load_gdt:
lgdt [gdt_info]
mov eax,cr0
or al,1
mov cr0,eax
jmp 0x08:pmode

[bits 32]
; address where load ELF header
; remain 0x400 for bootloader
HEADERADDR  EQU 0x8000
KARGS       EQU 0x8200; args for kernel main
KTAIL       EQU 0x8200;
KLAYOUTP    EQU 0x8204;
KLAYOUT     EQU 0x8300; memory layout detected by int15 
KLAYOUTE    EQU 0x8304;
KSECTOR     EQU 0x02  ;kernel start sector on hd

pmode:
mov ax,0x10
mov ds,ax
mov es,ax
mov gs,ax
mov ss,ax
mov dword [ds:KTAIL], 0x0;

;read the second sector of bootloader
mov edi, 0x7e00
mov ebx, 0x01
call read_one_sector

; read one sector to 0x8000 to analyze ELF header
mov edi,HEADERADDR
mov ebx,KSECTOR
call read_one_sector

read:
xor ecx,ecx
mov cx,[ds:HEADERADDR+0x2c]    ; program header num: 2bytes
xor eax,eax
mov eax,[ds:HEADERADDR+0x1c]   ; program header offset: 4bytes
mov [ds:ph_ptr],eax
xor ebx, ebx
mov bx,[ds:HEADERADDR+0x2a]    ; program header size: 2bytes
mov [ds:ph_size],ebx

read_segment:
mov edi, [ds:HEADERADDR + eax + 0x08] ; buff(vaddr)
push ecx
mov ecx, [ds:HEADERADDR + eax + 0x10] ; filesize
mov ebx, ecx
and ebx, 0x1ff
cmp ebx,0x0
jz no_remain 
shr ecx,9
inc ecx
jmp read_segment_1
no_remain:
shr ecx,9

read_segment_1:
mov ebx, [ds:HEADERADDR + eax + 0x04] ; offset in file
shr ebx, 9
add ebx, KSECTOR                      ;sector counts from 1

mov edx, [ds:HEADERADDR + eax + 0x0]
cmp edx, 0x01
jnz read_segment_2  ; if not loadable, read next segment
xor edx,edx

cmp ecx, 0x0
jz read_segment_2   ; if filesize = 0 , read next segment

; update kernel length
update_len:
push edx
push ebx
mov edx, [ds:HEADERADDR + eax + 0x14] ;segment runtime size
add edx, [ds:HEADERADDR + eax + 0x08] ;segment end
mov ebx, [ds:KTAIL]
cmp ebx, edx
jg update_ok
mov [ds:KTAIL], edx

update_ok:
pop ebx
pop edx

call read_many_sectors

read_segment_2:  
pop ecx
mov eax,[ds:ph_ptr]
mov ebx,[ds:ph_size]
add eax,ebx
mov [ds:ph_ptr],eax ; update ph addr
dec ecx
cmp ecx,0x0
jnz read_segment
mov eax, [ds:HEADERADDR + 0x18]
mov edx, KARGS
mov ebx, KLAYOUT
mov [ds:KLAYOUTP], ebx
push edx
call eax

hlt 

read_many_sectors:
;ecx = count
;ebx = first sector 
;edi = buffer
r:
pushad
call read_one_sector
popad
inc ebx
add edi,0x200
dec ecx
cmp ecx,0x0
jnz r

ret

read_one_sector: 
; ebx = sector number
; edi = buffer
;LBA28 supports 28bit sector address
mov dx,0x1f2
mov al,1
out dx,al

mov dx,0x1f3
mov al,bl
out dx,al

mov dx,0x1f4
shr ebx,8
mov al,bl
out dx,al

mov dx,0x1f5
shr ebx,8
mov al,bl
out dx,al

mov dx,0x1f6
shr ebx,4
and bl,0x0f
mov al,bl
or al,0xE0      ; 1110_0000 ; bit6=1 => mode logical block ; bit4=0 => drive/card=0
out dx,al

mov dx,0x1f7
mov al,0x20
out dx,al

blocking:
in al,dx
test al,0x80
jne blocking    ; jne or je ?

mov ecx,0x100   ;256 words
mov dx,0x1f0
rep insw

ret

ph_ptr:
dd 0x0
ph_size:
dd 0x0

gdt_start:

gdt_null:
dq 0    ; write dd 0 dd 0 error?

gdt_code:
dw 0xffff
dw 0x0000
db 0x00
db 0x9a ; 1001_1010
db 0xcf ; 1100_1111
db 0x00

gdt_data:
dw 0xffff
dw 0x0000
db 0x00
db 0x92 ; 1001_0010
db 0xcf
db 0x00

gdt_end:

gdt_info:
dw (gdt_end-gdt_start-1)
dd gdt_start

times 510-($-$$) db 0
dw 0xAA55