# Start the CPU: switch to 32-bit protected mode, jump into C
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00

#.section .text
#.code16			# Assemble for 16-bit mode


.section .text
.code16
.globl _start
_start:
    	#Set up data segment registers(DS, ES, SS)
    	movw $0x7c0, %ax 	# drop us at physical address 0x7c00
    	addw $0x20, %ax	# MBR
    	movw %ax, %ss	# SS <- stack segment after MBR
    	movw $0x1000, %sp	# stack pointer at 0x1000

    	movw $0x7c0, %ax
    	movw %ax, %ds	# DS <- data segment
	.set OFFSET, my_msg - _start
	movw $OFFSET, %si
    
	movb $0xe, %ah
	mov $0, %dx

print_loop:
    	lodsb
    	cmp $0, %al
	je done
	int $0x10
	jmp print_loop

done:
	hlt
	jmp done

my_msg:
    	.asciz "Hello from the bootloader!\n"

remainder:
    	.set MBR_REMAINDER, 510- (remainder - _start)
    	.fill MBR_REMAINDER, 1, 0x0
    	.word 0xaa55
