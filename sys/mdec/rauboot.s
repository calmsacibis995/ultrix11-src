/ SCCSID: @(#)rauboot.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ ULTRIX-11 Block Zero Bootstrap for MSCP Disks
/
/ UDA50 - RA60/RA80/RA81
/ RQDX1 - RX50/RD51/RD52
/ RC25  - RC25/RCF25
/
/ The a.out header must be stripped off (makefile does it).
/ Can boot from any unit.
/ On entry boot leaves:
/	r0 = unit #
/	r1 = aip register address
/
/ ****** MSCP BOOTSTRAP ANOMALIES ******
/
/ 2.	On any data or initialization error, the boot will hang on a branch
/	self instruction instead of restarting the boot.
/ WHY - was it only RDRX and not UDA also, in old uboots?
/
/ ***************************************
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
BC	= 512.*CLSIZE	/ Byte count for disk transfers
IGSHFT	= -4		/ Adjust for NDIRIN in iget (-3 or -4)
IGMASK1	= !7777		/ "	"	"	"   (!17777 or 7777)
IGMASK2	= !17		/ "	"	"	"   (!7 or !17)
/
/ **********************************************************

nop	= 240
s1	= 4000
go	= 1

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
	clr	r4
	mov	sp,r5
	cmp	pc,r5
	bhis	2f
1:
	mov	(r4)+,(r5)+
	cmp	r5,$end
	blo	1b
	jmp	(sp)

/ Clear core to make things clean,
/ has the pleasent side effect of initializing the MSCP
/ communications area and message packets to zero.
2:
	clr	(r4)+
	cmp	r4,sp
	blo	2b

/ MSCP controller initialization

	mov	r0,bdunit	/ save unit # booted from
	mov	r1,udaip	/ save aip register address
	clr	(r1)+		/ start uda init sequence
				/ move pointer to udasa register
	mov	$s1,r5		/ set uda state test bit to step 1
	mov	$1f,r4		/ address of init seq table
	br	2f		/ branch around table
1:
	100000			/ UDA_ERR, init step 1
	ring			/ address of ringbase
	0			/ hi ringbase address
	go			/ UDA go bit
2:
	tst	(r1)		/ error ?
	bmi	.		/ yes, hang - can't restart !!!
	bit	r5,(r1)		/ current step done ?
	beq	2b		/ no
	mov	(r4)+,(r1)	/ yes, load next step info from table
	asl	r5		/ change state test bit to next step
	bpl	2b		/ if all steps not done, go back
				/ r5 now = 100000, UDA_OWN bit
	mov	$36.,cmdhdr	/ command packet length
				/ don't set response packet length,
				/ little shakey but it works.
	mov	r0,udacmd+4.	/ load unit number
	mov	$11,udacmd+8.	/ on-line command opcode
	mov	$ring,r2	/ initialize cmd/rsp ring
	mov	$udarsp,(r2)+	/ address of response packet
	mov	r5,(r2)+	/ set UDA owner
	mov	$udacmd,(r2)+	/ address of command packet
	mov	r5,(r2)+	/ set UDA owner
	mov	-2(r1),r0	/ start UDA polling
3:
	tst	ring+2		/ wait for response, UDA_OWN goes to zero
	bmi	3b
/ Pass boot device type ID and unit number to Boot:
	mov	udarsp+28.,bdmtil	/ media type ID lo
	mov	udarsp+30.,bdmtih	/ media type ID hi
	br	rstrt1

rstart:				/ restart here if file not found
	movb	$'.,names+4	/ change to boot.bu if boot can't be loaded
	movb	$'b,names+5
	movb	$'u,names+6
rstrt1:
	mov	$buf,r0		/ clean up for restart
6:
	clr	(r0)+
	cmp	r0,sp
	blo	6b

/ now start reading the inodes
/ starting at the root and
/ going through directories
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
	cmp	r4,r2	/tstb	(r3)
	blo	4b	/bne	4b
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
/ load boot device type info into r0 -> r4
/ relocate core around
/ assembler header
1:
	mov	bdunit,r0	/ unit number
	mov	$2,r1		/ boot device type code 2 = RA (MSCP)
	mov	bdmtil,r2	/ media type ID
	mov	bdmtih,r3
	mov	udaip,r4	/ MSCP controller CSR address
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

/ MSCP DISK driver
/ low order address in dno,
/ high order address in r0

rblk:
	mov	r1,-(sp)
	mov	dno,r1
.if	CLSIZE-1
	ashc	$CLSHFT,r0		/ double blk number if 1K filsys
.endif
/	mov	$36.,cmdhdr		/ length of command packet
	mov	$41,udacmd+8.		/ read opcode
	mov	$BC,udacmd+12.		/ byte count
	mov	$buf,udacmd+16.		/ buffer descriptor
	mov	r1,udacmd+28.		/ block number low
	mov	(sp)+,r1
	mov	r0,udacmd+30.		/ block number hi
	mov	$100000,ring+2		/ set UDA owner of response
	mov	$100000,ring+6		/ set UDA owner of command
	mov	*udaip,r0		/ start UDA polling
1:
	tst	ring+2			/ wait for response
	bmi	1b
	tstb	udarsp+10.		/ does returned status = SUCCESS ?
	beq	2f			/ yes, return
	jmp	rstart			/ no, error (try boot.bu)
2:
	rts	pc

/            . b u	(try boot.bu if boot can't be loaded)
names: <boot\0\0\0\0\0\0\0\0\0\0\0>
end:
udaip = ..-256.-BSIZE
cmdint = udaip+2.
rspint = cmdint+2.
ring = rspint+2.
rsphdr = ring+8.
udarsp = rsphdr+4.
cmdhdr = udarsp+48.
udacmd = cmdhdr+4.
bdunit = udacmd+48.
bdmtil = bdunit+2.
bdmtih = bdmtil+2.
buf = bdmtih+2.
inod = buf+BSIZE
addr = inod+ADDROFF
bno = inod+INOSIZ
dno = bno+2.
