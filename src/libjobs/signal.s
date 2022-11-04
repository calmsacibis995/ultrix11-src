/ SCCSID: @(#)signal.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
SIGDORTI = 1000
rtt	= 6
iot	= 4
emt	= 104000
.signal	= 48.
.globl	_sigsys, __sigcatch, _mvectors, cerror
.globl	__ovno, __novno
NSIG = 32
_sigsys:
	mov	r5,-(sp)
	mov	sp,r5
	mov	6(r5),-(sp)	/ push signal action
	mov	4(r5),-(sp)	/ push signal number
	sys	.signal+200
	bes	1f
	cmp	(sp)+,(sp)+
	mov	(sp)+,r5
	rts	pc
1:
	cmp	(sp)+,(sp)+
	jmp	cerror
	
_mvectors:		/ these vectors are treated as an array in C
	jsr	r0,1f	/ dummy	there is no signal 0
	jsr	r0,1f	/ for signal 1
	jsr	r0,1f	/ for signal 2 etc...
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f
	jsr	r0,1f  / for signal 32
1:
	mov	__ovno,-(sp)	/ save previous overlay
	mov	r1,-(sp)	/ r0 is already on stack from jsr r0,...
	sub	$_mvectors+4,r0	/ if sig is == 1, r0 will be _mvectors + 8
	asr	r0 		/ divide by 4 to get signal number
	asr	r0 
	mov	r0,-(sp)	/ make an extra copy so sigcatch can clobber it
	mov	r0,-(sp)	/ this one is sigcatch's argument
	jsr	pc,*$__sigcatch	/ we know sigcatch will save regs 2-5!
	tst	(sp)+		/ discard argument
	mov	(sp)+,r1	/ get back signal number
	tst	r0		/ if zero we do nothing - just rtt
	bne	1f
	mov	(sp)+,r1
	mov	(sp)+,r0	/ previous overlay
	cmp	r0,__ovno	/ check against current overlay
	beq	3f
2:
	mov	r0,__novno
	emt			/ change overlays
	cmp	r0,__novno	/ did we change?
	bne	2b		/ nope, go back and try again
	mov	r0,__ovno	/ yep, record the new ovno
3:
	mov	(sp)+,r0	/ restore last of registers
	rtt
/ here we are re-enabling the signal with the value returned by sigcatch
1:
	bis	$SIGDORTI,r1	/ tell kernel to simulate rti upon return
	mov	4(sp),-(sp)	/ mov saved r0 down
	mov	2(sp),-(sp)	/ mov saved r1 down, leaving two words for arg
	mov	r0,10(sp)	/ mov action on stack.
	mov	6(sp),r0	/ previous overlay...
	cmp	r0,__ovno	/ check against current overlay
	beq	3f
2:
	mov	r0,__novno
	emt			/ change overlays
	cmp	r0,__novno	/ did we change?
	bne	2b		/ nope, go back and try again
	mov	r0,__ovno	/ yep, record the new ovno
3:
	mov	r1,6(sp)	/ mov sig num to top (after r0 r1 popped)
	mov	(sp)+,r1
	mov	(sp)+,r0	/ restore last of registers
	tst	(sp)+		/ clear saved ovno
				/ now all registers are totally restored
				/ and signal syscall args are on top of stack
				/ so that when system simulates the rti it
				/ will first pop off args to the syscall
	sys	.signal+200	/ set signals and rti (kernel shouldn't
				/ smash ANY register here
	iot			/ should never happen - lets find out!
