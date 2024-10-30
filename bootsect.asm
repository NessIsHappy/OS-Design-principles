; Setting download address for code and setting 16-bit mode
org 0x7C00
use16

; Basic program to output a string
start:
	mov ax, cs
	mov ds, ax
	mov ss, ax
	mov sp, start
	mov bx, loading_str
	call puts
	call clean_terminal
	call load_kernel
	jmp color

load_kernel:
	mov ax, 0x1000
	mov es, ax
	mov ah, 0x02
	mov dl, 0x01
	mov dh, 0x00
	mov cl, 0x01
	mov al, 123
	mov bx, 0x00
	int 0x13

; Function to output a string by symbols
puts:
	mov al, [bx]
	test al, al
	jz end_puts
	mov ah, 0x0e
	int 0x10
	add bx, 1
	jmp puts

end_puts:
	ret

clear_terminal:
	mov ah, 3
	int 0x10
	ret

color:
	mov ah, 0
	int 0x16
	mov bh, al
	mov al, bh
	mov ah, 0x0e
	int 0x10
	call clear_terminal
	jmp finish

; Going to protected mode
finish:
	mov [0x9000], al
	cli
	lgdt [gdt_info]
	in al, 0x92
	or al, 2
	out 0x92, al
	mov eax, cr0
	or al, 1
	mov cr0, eax
	jmp 0x8:protected_mode


loading_str:
	db "Choose color: 1 for green, 2 for blue, 3 for red, 4 for yellow, 5 for gray and 6 for white.", 0

; Descriptor table
gdt:
	db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
	db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00

gdt_info:
	dw gdt_info - gdt
	dw gdt, 0

; Setting up 32-bit mode and establishing protected mode
use32
protected_mode:
	mov ax, 0x10
	mov es, ax
	mov ds, ax
	mov ss, ax
	call 0x10000

; Filling a sector with zeros till the end
times (512 - ($ - start) - 2) db 0
db 0x55, 0xAA