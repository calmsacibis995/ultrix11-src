/ SCCSID: @(#)sci.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ ULTRIX-11 system call interface for standalone programs.
/ See /usr/sys/sas/README.
/
/ Fred Canter

bad_cmd = 140000
emtcode	= 140002
rtnaddr	= 140004
rq_ctid	= 140006
scentry	= 140020

/ This code resides in the program segment of the
/ standalone program.
/ When the program calls one of the functions that are now
/ in the syscall segment, the following routines begin the
/ calling sequence:
/
/	Set the emtcode of the function to be called.
/
/	Save the return address.
/
/	Transfer control to the interface code at 140000+.

.globl _lseek, _getc, _getw, _read
.globl _write, _open, _close

_lseek:
	mov	$104000,*$emtcode
	mov	(sp)+,*$rtnaddr
	jmp	*$scentry

_getc:
	mov	$104001,*$emtcode
	mov	(sp)+,*$rtnaddr
	jmp	*$scentry

_getw:
	mov	$104002,*$emtcode
	mov	(sp)+,*$rtnaddr
	jmp	*$scentry

_read:
	mov	$104003,*$emtcode
	mov	(sp)+,*$rtnaddr
	jmp	*$scentry

_write:
	mov	$104004,*$emtcode
	mov	(sp)+,*$rtnaddr
	jmp	*$scentry

_open:
	mov	$104005,*$emtcode
	mov	(sp)+,*$rtnaddr

/ The first argument to open is a pointer a string containing
/ the name of the file to be opened.
/ Because of remapping, we must copy the string into a buffer
/ after the syscall interface code (140000+sic_end), and pass open
/ a pointer to that buffer.

	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	$_sic_end,r0	/ calc real address of sic_end
	sub	$_sicode,r0
	add	$140000,r0
	mov	r0,-(sp)	/ save it for later use
	mov	10(sp),r1	/ addr of file name string
	mov	$512.,r2	/ absolute max # of characters
1:
	movb	(r1)+,(r0)+	/ copy string to buffer @ sic_end
	beq	2f
	sob	r2,1b
2:
	mov	(sp)+,6(sp)	/ change string addr passed to open
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	jmp	*$scentry

_close:
	mov	$104006,*$emtcode
	mov	(sp)+,*$rtnaddr
	jmp	*$scentry

/ The following is the actual system call interface code.
/ This code is relocated to virtual address 140000
/ by the start up code (srt0.s).
/ The syscall interface code does the following:
/
/ 1.	Remaps the first six memory management segmentation registers
/	to point to the syscall segment of the standalone program.
/
/ 2.	Sets the I/O segmentation flag for read/write calls, so that
/	the data will find its way into the correct buffer.
/	_segflag is known as *$ioseg in the interface code.
/
/ 3.	Does an EMT, which transfers control to the syscall segment (srt1.s).
/
/ 4.	When the call returns, remap back to the program segment and
/	return via the saved return address.

KISA0	= 172340
ioseg	= 1000

.globl	_sicode, _sic_end

_sicode:
	0			/ bad_cmd
	0			/ emtcode
	0			/ rtnaddr
	0			/ rq_ctid
	0			/ not currently used
	0			/ not currently used
	0			/ not currently used
	0			/ not currently used
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	$6,r0
	mov	$KISA0,r1
	mov	$2000,r2
1:
	mov	r2,(r1)+
	add	$200,r2
	sob	r0, 1b
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	mov	$1,*$ioseg		/ _segflag = 1 (default)
	cmp	*$emtcode,$104003
	bne	1f
	clr	*$ioseg			/ read call, segflag = 0
1:
	cmp	*$emtcode,$104004
	bne	1f
	clr	*$ioseg			/ write call, segflag = 0
1:
	mov	*$emtcode,1f
1:
	0
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	$6,r0
	mov	$KISA0,r1
	clr	r2
1:
	mov	r2,(r1)+
	add	$200,r2
	sob	r0, 1b
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	mov	*$rtnaddr,pc
_sic_end:
	0			/ This area is used as a buffer for syscall
				/ argument passing, see _open above.
