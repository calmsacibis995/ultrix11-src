/ SCCSID: @(#)csv0.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C register save and restore routines
/ ovcsv is in a seperate module, saving us
/ about 22 bytes of text space.  Cret and ovcret
/ can't be easily seperated, so non-overlayed programs
/ will just have to suffer with the extra 38 bytes of text.
/	6/84 -Dave Borman

.globl	csv, cret, csv__com
.globl	ovcret
.globl  __ovno, __novno
.globl  _etext
.data
__ovno:	0
__novno:	0
.text

emt= 0104000		/ overlays switched by emulator trap. ovno in r0.
halt= 0

csv:
	mov	r5,r1
	mov	sp,r5
	mov	__ovno,-(sp)	/ overlay is extra (first) word in mark
/ rest is code common with ovcsv
csv__com:
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
