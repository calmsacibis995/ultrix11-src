/ SCCSID: @(#)csv.s	3.0	5/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C register save and restore -- version 7/75
/ modified by wnj && cbh 6/79 for overlayed text registers
/ modified by wf jolitz 2/80 to work and use emt syscall
/
/ we define ovcsv and ovcret which overlay routines call
/ even though ovcret is (currently) the same as cret
/ the loader finagles the .o files so this happens

.globl	csv
.globl	cret
.globl	ovcsv, ovcret
.globl  __ovno, __novno
.globl  _etext, ovhndlr
.data
__ovno:	0
__novno:	0
.text

emt= 0104000		/ overlays switched by emulator trap. ovno in r0.
halt= 0


/ csv for routines in overlays
/ the previous overlay is in __ovno, which is saved on the stack.
/ after it is saved, __ovno is set to the current overlay number
/ which has been put in r0 by the thunk.
ovcsv:
	mov	r5,r1
	mov	sp,r5
	cmp	$7,r0
	bge	1f
	halt
1:
	mov	__ovno,-(sp)
	mov	r0,__ovno
	jbr	1f

/ only root segment routines call csv, and when it is called
/ no overlays have been changed, so we just save the previous overlay
/ number on the stack. note that r0 is'nt set to the current overlay
/ because we were'nt called through a thunk.
csv:
	mov	r5,r1
	mov	sp,r5
	mov	__ovno,-(sp)	/ overlay is extra (first) word in mark
/ rest is old code common with ovcsv
1:
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)		/ jsr part is sub $2,sp
/
/ at this point, the stack frame looks like this:
/
/	_________________________
/	|  return addr to callee|
/	|_______________________|
/ r5->	| old r5	        |
/	|_______________________|
/	| previous ovnumber     |
/	|_______________________|
/	| old r4		|
/	|_______________________|
/	| old r3		|
/	|_______________________|
/ sp->	| old r2		|
/	|_______________________|
/


ovcret:		/ same as cret, i think
cret:
	mov	r5,r2
/ get the overlay out of the mark, and if it is non-zero
/ make sure it is the currently loaded one
	mov	-(r2),r4
	bne	1f		/ zero is easy
2:
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
/ not returning to root segment, so check that the right
/ overlay is loaded, and if not ask UNIX for help
1:
	cmp	r4,__ovno
	beq	2b		/ lucked out!
/ if return address is in root segment, then nothing to do
	cmp	2(r5),$_etext
	blos	2b
/ returning to wrong overlay --- do something!
	mov	r0,r3
	mov	r4,r0
3:
	mov	r0, __novno
	emt
	cmp	r0, __novno
	bne	3b
	mov	r4, __ovno
	mov	r3,r0
/ intr. routines may run between these, so should force segment __ovno
	br	2b
