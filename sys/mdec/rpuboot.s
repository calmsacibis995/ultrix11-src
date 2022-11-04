/ SCCSID: @(#)rpuboot.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ ULTRIX-11 Block Zero Bootstrap for RP02/3 Disks
/ Can only boot from unit zero.
/ Assumes disk at standard address (0176710).
/
/ Fred Canter
/
/ disk boot program to load and transfer
/ to a unix entry

/ ****** CONSTANTS FOR FILE SYSTEM LOGICAL BLOCK SIZE ******
/
/ For 512  byte logical blocks:
/ CLSIZE= 1, NDIRIN= 10., INOFF= 15., IGSHFT= -3, IGMASK1= !17777, IGMASK2= !7
/ For 1024 byte logical blocks:
/ CLSIZE= 2, NDIRIN= 4., INOFF= 31., IGSHFT= -4, IGMASK1= !7777, IGMASK2= !17
/
CLSIZE	= 2.		/ Physical disk blocks per logical block (cluster)
CLSHFT	= 1.		/ Shift to multiply by CLSIZE (doubles block number)
BSIZE	= 512.*CLSIZE	/ Logical block size (512 or 1024 bytes)
INOSIZ	= 64.		/ Size of an inode in bytes
NDIRIN	= 4.		/ Number of direct inode addresses (10 or 4)
ADDROFF	= 12.		/ Offset of first address in inode
INOPB	= BSIZE\/INOSIZ	/ Number of inodes per logical block (8 or 16)
INOFF	= 31.		/ Inode offset (INOPB * (SUPERB+1)) - 1  (15 or 31)
WC	= -256.*CLSIZE	/ Word count for disk transfers
IGSHFT	= -4		/ Adjust for NDIRIN in iget (-3 or -4)
IGMASK1	= !7777		/ "	"	"	"   (!17777 or 7777)
IGMASK2	= !17		/ "	"	"	"   (!7 or !17)
/
/ **********************************************************

nop	= 240
reset	= 5

core = 28.
.. = [core*2048.]-512.

/ establish sp and check if running below
/ intended origin, if so, copy
/ program up to 'core' K words.
start:
	nop		/ DEC boot block standard
	br	1f	/ "
1:
	mov	$..,sp
	clr	r0
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
1:
	mov	(r0)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)

/ clear core to make things clean
2:
	clr	(r0)+
	cmp	r0,sp
	blo	2b

/ initialize rp

	clr	*$rpcs		/ selects drive zero

/ now start reading the inodes
/ starting at the root and
/ going through directories
1:
	mov	$names,r1
	mov	$2,r0
1:
	clr	bno
	jsr	pc,iget
	tstb	(r1)
	beq	1f
2:
	jsr	pc,rmblk
		br rstart
	mov	$buf,r2
3:
	mov	r1,r3
	mov	r2,r4
	add	$16.,r2
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)+
	bne	5f
	cmp	r4,r2
	blo	4b
	mov	-16.(r2),r0
	add	$14.,r1
	br	1b
5:
	cmp	r2,$buf+BSIZE
	blo	3b
	br	2b

/ read file into core until
/ a mapping error, (no disk address)
1:
	clr	r1
1:
	jsr	pc,rmblk
		br 1f
	mov	$buf,r2
2:
	mov	(r2)+,(r1)+
	cmp	r2,$buf+BSIZE
	blo	2b
	br	1b
/ relocate core around
/ assembler header
1:
	clr	r0		/ boot device unit number
	mov	$6,r1		/ boot device code
	mov	$rpds,r4	/ boot device CSR address
	clr	r5
/	cmp	(r5),$407	/ boot will always have the a.out header
/	bne	2f		/ but, it will not always be 0407!
1:
	mov	20(r5),(r5)+
	cmp	r5,sp
	blo	1b
/ enter program
2:
	clr	pc

rstart:
	movb	$'.,names+4	/ try boot.bu if boot can't be loaded
	movb	$'b,names+5
	movb	$'u,names+6
	jmp	start

/ get the inode specified in r0
iget:
	add	$INOFF,r0
	mov	r0,r5
	ash	$IGSHFT,r0
	bic	$IGMASK1,r0
	mov	r0,dno
	clr	r0
	jsr	pc,rblk
	bic	$IGMASK2,r5
	mul	$INOSIZ,r5
	add	$buf,r5
	mov	$inod,r4
1:
	mov	(r5)+,(r4)+
	cmp	r4,$inod+INOSIZ
	blo	1b
	rts	pc

/ read a mapped block
/ offset in file is in bno.
/ skip if success, no skip if fail
/ the algorithm only handles a single
/ indirect block. that means that
/ files longer than NDIRIN+128 blocks cannot
/ be loaded.
rmblk:
	add	$2,(sp)
	mov	bno,r0
	cmp	r0,$NDIRIN
	blt	1f
	mov	$NDIRIN,r0
1:
	mov	r0,-(sp)
	asl	r0
	add	(sp)+,r0
	add	$addr+1,r0
	movb	(r0)+,dno
	movb	(r0)+,dno+1
	movb	-3(r0),r0
	bne	1f
	tst	dno
	beq	2f
1:
	jsr	pc,rblk
	mov	bno,r0
	inc	bno
	sub	$NDIRIN,r0
	blt	1f
	ash	$2,r0
	mov	buf+2(r0),dno
	mov	buf(r0),r0
	bne	rblk
	tst	dno
	bne	rblk
2:
	sub	$2,(sp)
1:
	rts	pc

cyl	= 0.
read	= 4
go	= 1

rpds	= 176710
rpcs	= 176714
rpda	= 176724
rpca	= 176722
rpba	= 176720
/ rp02/3 disk driver.
/ low order address in dno,
/ high order in r0.
rblk:
	mov	r1,-(sp)
	mov	dno,r1
.if	CLSIZE-1
	ashc	$CLSHFT,r0		/ double blk number if 1K filsys
.endif
	div	$20.*10.,r0
/	add	$cyl,r0
	mov	r0,*$rpca
	clr	r0
	div	$10.,r0
	swab	r0
	bis	r1,r0
	mov	r0,*$rpda
	mov	$rpba,r1
	mov	$buf,(r1)
	mov	$WC,-(r1)
	mov	$read+go,-(r1)
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	bpl	2f
	reset			/ error
	mov	(sp)+,r1
	jmp	rstart
2:
	mov	(sp)+,r1
	rts	pc

/	     . b u	(try boot.bu if boot can't be loaded)
names: <boot\0\0\0\0\0\0\0\0\0\0\0>
end:
inod = ..-256.-BSIZE
addr = inod+ADDROFF
buf = inod+INOSIZ
bno = buf+BSIZE
dno = bno+2
