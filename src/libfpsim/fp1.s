/ SCCSID = @(#)fp1.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ fp1 -- floating point simulator

rti	= 2
bpt	= 3

m.ext = 200		/ long mode bit
m.lngi = 100		/ long integer mode

.globl	fptrap
.globl	ac0, ac1, ac2, ac3

fptrap:
	dec	reenter
	bge	1f
	4		/ reentered!
1:
	mov	(sp)+,spc
	mov	(sp)+,sps
	mov	r0,sr0
	mov	$sr1,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(r0),r5		/ pc
	dec	r5
	dec	r5
	mov	r5,calarg
	mov	$fpbuf,calarg+2
	sys	0; calsys
	mov	fpbuf,r5

again:
	sub	$8,sp		/ room for double push
	clr	trapins
	mov	r5,r4
	bic	$7777,r4
	cmp	r4,$170000
	beq	1f
	jmp	badins
1:
	bic	$100000,fpsr	/ clear fp error
	bic	$170000,r5
	mov	r5,r4
	bit	$7000,r4
	bne	class3
	bit	$700,r4
	bne	class2
	cmp	r4,$12
	blos	1f
	jmp	badins
1:
	asl	r4
	jmp	*agndat(r4)


class2:
	cmp	r5,$400
	bge	1f
	mov	$mod0rx,modctl
	mov	$mod242,modctl+2
	jsr	r1,fsrc
	br	2f
1:
	mov	$mod0f,modctl
	mov	$mod24f,modctl+2
	jsr	r1,fsrc
2:
	mov	r3,r5
	asl	r4
	asl	r4
	clrb	r4
	swab	r4
	asl	r4
	jsr	pc,*cls2dat(r4)
	jmp	sret


class3:
	cmp	r5,$5000
	blt	1f
	mov	r5,r2
	clrb	r2
	cmp	r2,$6400
	blt	2f
	sub	$1400,r2
2:
	cmp	r2,$5000
	bne	2f
	mov	$mod0rx,modctl
	mov	$mod242,modctl+2
	jsr	r1,fsrc
	br	3f
2:
	cmp	r2,$5400
	bne	2f
	mov	$mod0ra,modctl
	mov	$mod24i,modctl+2
	jsr	r1,fsrc
	br	3f
2:
	mov	$mod0f,modctl
	mov	$mod24d,modctl+2
	jsr	r1,fsrc
	br	3f
1:
	mov	$mod0f,modctl
	mov	$mod24f,modctl+2
	jsr	r1,fsrc
3:
	jsr	pc,freg
	mov	r2,r5
	clrb	r4
	swab	r4
	asl	r4
	jsr	pc,*cls3dat(r4)
	br	sret


i.cfcc:
	mov	fpsr,r0
	bic	$!17,r0
	mov	r0,sps
	br	ret

i.setf:
	bic	$m.ext,fpsr
	br	ret

i.setd:
	bis	$m.ext,fpsr
	br	ret

i.seti:
	bic	$m.lngi,fpsr
	br	ret

i.setl:
	bis	$m.lngi,fpsr
	br	ret

badins:
	inc	trapins
	br	ret1

sret:
	mov	$fpsr,r0
	bic	$17,(r0)
	tstb	1(r5)
	bpl	1f
	bis	$10,(r0)
	br	ret
1:
	bne	ret
	bis	$4,(r0)

ret:
	mov	ssp,sp
	mov	spc,calarg
	mov	$fpbuf,calarg+2
	sys	0; calsys
	mov	fpbuf,r5
	cmp	r5,$170000
	blo	ret1
	add	$2,spc
	jbr	again			/ if another fp, save trap

ret1:
	mov	$sr1,r0
	mov	(r0)+,r1
	mov	(r0)+,r2
	mov	(r0)+,r3
	mov	(r0)+,r4
	mov	(r0)+,r5
	mov	(r0)+,sp
	mov	sr0,r0
	mov	sps,-(sp)
	mov	spc,-(sp)
	tst	trapins
	bne	1f
	inc	reenter
	rti
1:
	bpt

freg:
	mov	r5,r2
	bic	$!300,r2
	asr	r2
	asr	r2
	asr	r2
	add	$ac0,r2
	rts	pc

fsrc:
	mov	r5,r3
	bic	$!7,r3			/ register
	asl	r3
	add	$sr0,r3
	mov	r5,r0
	bic	$!70,r0			/ mode
	asr	r0
	asr	r0
	jmp	*moddat(r0)


mod24f:
	mov	$4,r0
	bit	$m.ext,fpsr
	beq	1f
	add	$4,r0
1:
	rts	pc

mod24d:
	mov	$8,r0
	bit	$m.ext,fpsr
	beq	1f
	sub	$4,r0
1:
	rts	pc

mod242:
	mov	$2,r0
	rts	pc

mod24i:
	mov	$2,r0
	bit	$m.lngi,fpsr
	beq	1f
	add	$2,r0
1:
	rts	pc

mod0:
	jmp	*modctl

mod0f:
	sub	$sr0,r3			/ get fp ac
	cmp	r3,$6*2
	bhis	badi1
	asl	r3
	asl	r3
	add	$ac0,r3
	rts	r1

mod0ra:
	bit	$m.lngi,fpsr
	bne	badi1

mod0r:
	cmp	r3,$ssp
	bhis	badi1
mod0rx:
	rts	r1

mod1:
	cmp	r3,$spc
	beq	badi1
	mov	(r3),r3
	br	check

mod2:
	mov	(r3),-(sp)
	jsr	pc,*modctl+2
	cmp	r3,$spc
	bne	1f
	mov	$2,r0
	mov	(r3),calarg
	sys	0; calsys
	mov	fpbuf,pctmp
	mov	$pctmp,(sp)
1:
	add	r0,(r3)
	mov	(sp)+,r3
	br	check

mod3:
	cmp	r3,$spc
	bne	1f
	mov	(r3),calarg
	sys	0; calsys
	mov	fpbuf,-(sp)
	br	2f
1:
	mov	*(r3),-(sp)
2:
	add	$2,(r3)
	mov	(sp)+,r3
	br	check

mod4:
	cmp	r3,$spc		/ test pc
	beq	badi1
	jsr	pc,*modctl+2
	sub	r0,(r3)
	mov	(r3),r3
	br	check

mod5:
	cmp	r3,$spc
	beq	badi1
	sub	$2,(r3)
	mov	*(r3),r3
	br	check

mod6:
	mov	spc,calarg
	sys	0; calsys
	mov	fpbuf,-(sp)
	add	$2,spc
	add	(r3),(sp)
	mov	(sp)+,r3
	br	check

mod7:
	jsr	r1,mod6
	mov	(r3),r3
	br	check

badi1:
	jmp	badins

check:
	bit	$1,r3
	bne	badi1
	rts	r1

setab:
	mov	$asign,r0
	jsr	pc,seta
	mov	r3,r2
	mov	$bsign,r0

seta:
	clr	(r0)
	mov	(r2)+,r1
	mov	r1,-(sp)
	beq	1f
	blt	2f
	inc	(r0)+
	br	3f
2:
	dec	(r0)+
3:
	bic	$!177,r1
	bis	$200,r1
	br	2f
1:
	clr	(r0)+
2:
	mov	r1,(r0)+
	mov	(r2)+,(r0)+
	bit	$m.ext,fpsr
	beq	2f
	mov	(r2)+,(r0)+
	mov	(r2)+,(r0)+
	br	3f
2:
	clr	(r0)+
	clr	(r0)+
3:
	mov	(sp)+,r1
	asl	r1
	clrb	r1
	swab	r1
	sub	$200,r1
	mov	r1,(r0)+	/ exp
	rts	pc

norm:
	mov	$areg,r0
	mov	(r0)+,r1
	mov	r1,-(sp)
	mov	(r0)+,r2
	bis	r2,(sp)
	mov	(r0)+,r3
	bis	r3,(sp)
	mov	(r0)+,r4
	bis	r4,(sp)+
	bne	1f
	clr	asign
	rts	pc
1:
	bit	$!377,r1
	beq	1f
	clc
	ror	r1
	ror	r2
	ror	r3
	ror	r4
	inc	(r0)
	br	1b
1:
	bit	$200,r1
	bne	1f
	asl	r4
	rol	r3
	rol	r2
	rol	r1
	dec	(r0)
	br	1b
1:
	mov	r4,-(r0)
	mov	r3,-(r0)
	mov	r2,-(r0)
	mov	r1,-(r0)
	rts	pc

