[bits 32]

[global switch_proc]

;|to
;|from
;|eip of "from"'s return point after switch_proc
switch_proc:

mov eax, [ss:esp + 0x04]; eax = from
mov [ds:eax + 0x0c], ecx; save ecx
mov ecx, [ss:esp + 0x00]; ecx = eip
mov [ds:eax + 0x08], ecx; save eip
pushf
pop ecx                 ; ecx = eflags
mov [ds:eax + 0x00], ecx; save eflags
mov [ss:eax + 0x04], esp; save esp
mov [ss:eax + 0x10], edx; save edx
mov [ss:eax + 0x14], ebx; save ebx
mov [ss:eax + 0x18], ebp; save ebp
mov [ss:eax + 0x1c], esi; save esi
mov [ss:eax + 0x20], edi; save edi

mov eax, [ss:esp + 0x08]; eax = to
mov ecx, [ds:eax + 0x0c]; load ecx
mov edx, [ds:eax + 0x10]; load edx
mov ebx, [ds:eax + 0x14]; load ebx
mov ebp, [ds:eax + 0x18]; load ebp
mov esi, [ds:eax + 0x1c]; load esi
mov edi, [ds:eax + 0x20]; load edi
mov ecx, [ds:eax + 0x00]; ecx = eflags
push ecx
popf                    ; load eflags
mov esp, [ds:eax + 0x04]; load esp, change stack to the target process
mov eax, [ds:eax + 0x08]; eax = target process eip
mov [ss:esp + 0x00], eax; change return eip

ret