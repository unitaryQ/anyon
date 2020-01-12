[bits 32]

%macro intr 1
[global intr%1]
intr%1:
cli
push dword 0 ; dummy 32bit
push dword %1; index 32bit
jmp common_entry
%endmacro

%macro intre 1
[global inte%1] ; already pushed an error code by hardware 
intre%1:
cli
push dword %1; index 32bit
jmp common_entry
%endmacro

intr 0 ; div 0
intr 1
intr 2
intr 3
intr 4
intr 5
intr 6
intr 7
intre 8
intr 9
intre 10
intre 11
intre 12
intre 13
intre 14
intr 15
intr 16
intr 17
intr 18
intr 19
intr 20
intr 21
intr 22
intr 23
intr 24
intr 25
intr 26
intr 27
intr 28
intr 29
intr 30
intr 31
intr 32 ; timer
intr 33 ; keyboard
intr 34 ; cascade
intr 35
intr 36
intr 37
intr 38
intr 39
intr 40
intr 41
intr 42
intr 43
intr 44
intr 45
intr 46
intr 47
intr 128 ; syscall

[global intr_lst] ; export the address of each handler as an array
intr_lst:
dd intr0 ; div 0
dd intr1
dd intr2
dd intr3
dd intr4
dd intr5
dd intr6
dd intr7
dd intre8
dd intr9
dd intre10
dd intre11
dd intre12
dd intre13
dd intre14
dd intr15
dd intr16
dd intr17
dd intr18
dd intr19
dd intr20
dd intr21
dd intr22
dd intr23
dd intr24
dd intr25
dd intr26
dd intr27
dd intr28
dd intr29
dd intr30
dd intr31
dd intr32 ; timer
dd intr33 ; keyboard
dd intr34 ; cascade
dd intr35
dd intr36
dd intr37
dd intr38
dd intr39
dd intr40
dd intr41
dd intr42
dd intr43
dd intr44
dd intr45
dd intr46
dd intr47
dd intr128 ; syscall

common_entry:

[extern common_handler]
pushad
push es
push fs
push ds
push gs

mov eax, 0x10; kernel data segment
mov ds, ax
mov es, ax

; we'd better change code & data segment to ring0, but not now
; pointer to interrupt stack layout
;|gs|ds|fs|es|8 eflags|int code|err code|eip|cs <|esp|ss|>
push esp ;esp is an arg
call common_handler
pop esp

lift_ret:
pop gs
pop ds
pop fs
pop es
popad
add esp, 8
iret

[global lift_kernel_thread]

;|interrupt_stack_t
;|pcb arg
;|dummy eip
;|dummy ebp <- ebp
;|...
;|kernel_to_user caller eip
lift_kernel_thread:
mov esp, ebp
add esp, 0x0c ;set esp to the man-made interrupt stack
jmp lift_ret
