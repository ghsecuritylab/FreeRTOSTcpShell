
	.section	.text.memcpy4,"ax",%progbits
	.arch armv7m
	.thumb
	.syntax unified
	.global	memcpy4

memcpy4:                @ r0 = dst, r1 = src, r2 = cb
    mov r3, r0          @ r3 = dst pointer
	mov r4, r1          @ r4 = src pointer
	add r5, r1, r2      @ r5 = end of source

loop:
    cmp r5, r4
	it eq
	moveq pc, lr        @ nothing left to copy so return dst
	ldrb r6, [r4]       @ r6 = current byte
	add r4, #1
	strb r6, [r3]       
	add r3, #1
	b loop              @ keep copying
