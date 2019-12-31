#Compilers and linker
GCC = gcc
CFLAGS = -nostdinc -fno-builtin -fno-stack-protector -fno-PIC 
CFLAGS += -m32 -g -c
CFLAGS += -Iinclude
ASM = nasm
ASMFLAGSKER = -f elf
ASMFLAGS = -f bin
LD = ld
LDFLAGS = -m elf_i386 
LDFLAGS += -Ttext 0x100000 -e kmain

#Kernel
KERNELDIR = kernel
KERNELSRC = $(wildcard $(KERNELDIR)/*.c)	
KERNELO   = $(patsubst %.c, %.o, $(KERNELSRC))
KERNELOBJ = core
KERNELASMSRC = $(wildcard $(KERNELDIR)/*.s)
KERNELASMOBJ = $(patsubst %.s, %.1, $(KERNELASMSRC))

#Boot
BOOTSRC = ./boot/bootloader.s	
BOOTOBJ = bootloader

#Image
IMGCNT = 40960 #20M
IMG = anyon.img

#Bochs
BOCHS = /home/hatikvah/bochs-2.6.10/bin/bochs
BOCHSRC = ./bochsrc

#Qemu
QEMU = qemu-system-i386
MEDIA = -hda
RAMSIZE = 128M
DEBUGFLAGS = -s -S

#Gdb
GDB = gdb
GDBFLAGS = --symbols=$(KERNELOBJ)
GDBFLAGS += -q
GDBFLAGS += --eval-command="target remote localhost:1234"
#GDBFLAGS += -tui

build: $(KERNELO) $(KERNELASMOBJ)
	$(ASM) $(ASMFLAGS) $(BOOTSRC) -o $(BOOTOBJ)
	$(LD)  $(LDFLAGS) $(KERNELASMOBJ) $(KERNELO) -o $(KERNELOBJ)

$(KERNELDIR)/%.o: $(KERNELDIR)/%.c
	$(GCC) $(CFLAGS) $< -o $@

$(KERNELDIR)/%.1: $(KERNELDIR)/%.s
	$(ASM) $(ASMFLAGSKER) $< -o $@

image:	
	dd if=/dev/zero of=$(IMG) bs=512 count=$(IMGCNT)
	dd if=$(BOOTOBJ) of=$(IMG) bs=512 count=2 seek=0 conv=notrunc	
	dd if=$(KERNELOBJ) of=$(IMG) bs=512 seek=2 conv=notrunc

qemu:
	$(QEMU) $(MEDIA) $(IMG) -m $(RAMSIZE)

qemu-debug:
	$(QEMU) $(DEBUGFLAGS) $(MEDIA) $(IMG) -m $(RAMSIZE)

gdb:
	$(GDB) $(GDBFLAGS)

bochs: 
	$(BOCHS) -f $(BOCHSRC)

all:
	make build
	make image
	make qemu

clean: 
	rm $(BOOTOBJ) $(IMG) $(KERNELOBJ) $(KERNELASMOBJ) $(KERNELO) *.lock
	
