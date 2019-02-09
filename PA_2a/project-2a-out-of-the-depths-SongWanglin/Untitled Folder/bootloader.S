.code16
.global _start

_start:
	mov $msg, %si
	mov $0xe, %ah
print_char:
	lodsb
	cmp $0, %al
	je done
	int $0x10
	jmp print_char
done:
	hlt
msg:	.asciz "Hello from boot loader!\n"

.fill 510 - (.-_start), 1, 0
.word 0xaa55
