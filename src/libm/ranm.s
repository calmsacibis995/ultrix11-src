/ SCCSID: @(#)ranm.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
.text
.globl	_ranm
_ranm_:


.text
_ranm:
	jsr	r5,csv
	mov	r5,sr5
	jsr	pc,ranm
	mov	r0,t0
	mov	r1,t1
	mov	r2,t2
	mov	r3,t3
	movf	t0, fr0
	mov	sr5,r5
	jmp	cret

 

.bss
.even
t0:	.=.+2
t1:	.=.+2
t2:	.=.+2
t3:	.=.+2
sr5:	.=.+2
 
// rany: random number generator 
//created by d. lehmer and dave hutchinson
//period is about 2 billion 
//even the low order bits are wonderfully random
//coded up by d. w. krumme, june 1977 
 
//entry iran
//      initialize generators 
//      the six integer arguments are used as the starting numbers
//
.globl	_iran
.text
_iran:
	jsr	r5,csv
       mov     4(r5),r0 
       mov     6(r5),r1 
       mov     10(r5),r2 
       jsr     pc,2f 
       mov     r0,xno1 
       mov     r1,rno1 
       mov     r2,rno1+2 
       mov     12(r5),r0 
       mov     14(r5),r1 
       mov     16(r5),r2 
       jsr     pc,2f 
       mov     r0,xno2 
       mov     r1,rno2 
       mov     r2,rno2+2 
       inc     initfg

2: 
       bic     $100000,r0
       bic     $100000,r1
       bis     $1,r0 
       bis     $1,r2 
       rts     pc

// filtab
//       fill up the mixing table
 
filtab: 
       mov     $mixtab,r5
1:                             /alternate enerators in pairs 
       mov     $rno1,r4
       jsr     pc,tabone 
       jsr     pc,tabone 
       mov     $rno2,r4
       jsr     pc,tabone 
       jsr     pc,tabone 
       cmp     r5,$etable
       blo     1b
       clr     initfg
       rts     pc
tabone: 
       mov     r5,-(sp)
       mov     r4,r5 
       add     $4,r5 
       jsr     pc,modmul 
       mov     (sp)+,r5
       mov     r0,(r5)+
       mov     r1,(r5)+
       rts     pc

// entry ranm
//       random numbers with mixed up sequence 
 
.globl ranm 
ranm: 
       tst     initfg
       beq     1f
       jsr     pc,filtab       /on the first call, fill up the table 
1:     mov     $xmul1,r4 
       jsr     pc,mixint       /get high 31 bits 
       mov     r3,-(sp)        /and save 
       mov     r2,-(sp)
       mov     $xmul2,r4
       jsr     pc,mixint       /get low 31 bits
       mov     (sp)+,r0
       mov     (sp)+,r1
       ashc    $1,r2           /concatenate high and low 
                               /now normalize
       mov     $200,r5         /exponent zero
norm:   ashc    $1,r2 
       rol     r1
       rol     r0
       bmi     1f
       sob     r5,norm 
                               /here all would be zero 
                               /now form floating point
1: 
       movb    r1,r4 
       ashc    $-8.,r0 
       ashc    $-8.,r2 
       swab    r2
       clrb    r2
       bisb    r4,r2 
       swab    r2
       bic     $177600,r0
       swab    r5              /clears c-bit 
       ror     r5
       bis     r5,r0 
       rts     pc

// mixint
//       get a 31-bit integer by way of the mixing generator and table 
 
mixint: 
       mov     (r4)+,r0
       mul     (r4),r0         /run the mixing generator 
       bic     $100000,r1
       mov     r1,(r4)+
       mov     r4,r5           /r4:    rno 
       add     $4,r5           /r5:    rmul
       jsr     pc,modmul       /get next integer in r0,r1
       movb    -1(r4),r5       /get 7 bits from the mixing genertor
       ash     $2,r5 
	add	$mixtab,r5	/ run integer through table
       mov     (r5),r2 
       mov     r0,(r5)+
       mov     (r5),r3 
       mov     r1,(r5)+
       rts     pc              /return result in r2,r3 
.data 
.even
statev:					/ 1 + 12 = 13, + 256 = 269 words

initfg: 1 

mixtab: .=.+512.		/ 256 words
etable: 
 
xmul1:  	12555 
xno1:   	12555 
rno1:   	24223;046343 
rmul1:  	24223;046343           /680742115 = 7**23 (mod 2**31 - 1)
 
xmul2:  	13265 
xno2:   	13265 
rno2:   	1450;012656 
rmul2:  	1450;012656     /52958638 = 7**17 (mod 2**31 -1)
 
// since the low words of both multipliers have bit 15 clear,
// the corection factor code for them has been commented out below 
// if the multiliers are changed, be careful 



// multiply two 31-bit integers modulo 2**31 - 1 
// this procedure generates random integers exhausting [1,2**31-2]
// when the right multipliers are used 
 
.text
modmul: 
 
// the following code multiplies two 31-bit (positive) integers
//       producing a 62-bit result in r0,r1,r2,r3
// the numbers are wa + b and wc + d, where w=2**16
// the full 32 bit unsigned multiplication were desired, more correction 
//       factors would have to be included, so that each multiplication
//       would be done as bd is done here
// correction factors are necessary because the pdp-11 multiplier interprrets
//       negative numbers.  mul x,y gives the 32-bit results:  xy/ -x(w-y)/
//       -(w-x)y/ (w-x)(w-y)-w**2 0<x,y/ y<0<x/ x<0<y/ x,y<0 respectively
// a is at (r4), b at 2(r4)
// c is at (r5), d at 2(r5)
 
       mov     (r4),r0         /ac 
       mul     (r5)+,r0        /no correction
       mov     (r4),r2         /ad 
       mul     (r5),r2 
//       tst     (r5)            /d is known to be ok! 
//       bpl     1f
//       add     (r4),r2         /correct d<0
1: 
       add     r2,r1 
       adc     r0
       mov     r3,-(sp)        /keep this slot on the stack
 
       tst     (r4)+           /bc 
       mov     (r4),r2 
       mul     -(r5),r2
       tst     (r4)
       bpl     2f
       add     (r5),r2         /correct b<0
2: 
       add     r2,r1 
       adc     r0
       add     r3,(sp) 
       adc     r1
       adc     r0
 
       mov     (r4),r2         /bd 
       tst     (r5)+ 
       mul     (r5),r2 
       tst     (r4)
       bpl     3f
       add     (r5),r2         /correct b<0
3:/     tst     (r5)            /d is known to be ok! 
//       bpl     4f
//       add     (r4),r2         /correct d<0
4: 
       add     (sp)+,r2
       adc     r1
       adc     r0              /62-bit result in r0-r3 

//  the following code takes modulo 2**31 - 1 by casting out (2**31-1)'s in base 2**31
// r0 must have top two bits zero
// the result is left in r0,r1 
 
       rol     r2              /align to base 2**31
       rol     r1
       rol     r0
       ror     r2              /c-bit was clear here 
 
       add     r3,r1 
       adc     r0
       bpl     8f 
       add     $100000,r0
       adc     r1              /can't carry
 
8:
       add     r2,r0 
       bpl     9f 
       add     $100000,r0
       adc     r1
       adc     r0              /can't carry into bit 
9:                            /we should reduce 2**31-1 to 0 here 
                               /but it can't happen
 
// all done
// put the result at (r4)
// and restore r4
 
       mov     r1,(r4) 
       mov     r0,-(r4)
       rts     pc
 
///*********************************************************************
/// new added section.
// 3/6/80	j reeds. save and restore state of generators
// 
//	integer bigtab(269), smtab(12)
//
//	call ransav(bigtab)
//	call ranres(bigtab)	! save, retore complete state
//
//		these two not
//		implemented yet:
//	call ranmup(smtab)
//	call ranmdn(smtab)	! save and restore seeds, etc but not
//				!mixing table
//
.text
blkmov: 
       dec     r2
       blt     2f              /while (--r2 >= 0 BEGIN
       cmp     r3,$0
       beq     1f
       mov     (r0)+,(r1)+     /if (r3 == 0) *r0++ = *r1++
       br      blkmov
1: 
       mov     (r1)+,(r0)+     /else *r1++ = *r0++ 
       br      blkmov
2: 
       rts     pc              / END elihw 
.globl _ransav
_ransav: 
_ransav_:

	jsr	r5,csv
       mov     $1,r3           /r3 = 1 so params = r0 -> r1 = table
       br      ranrs2
.globl _ranres
_ranres_:

_ranres:
	jsr	r5,csv
       clr     r3              /r3 = 0 so table = r1 -> r0 = params
ranrs2:
/	cmp	(r5)+,$1	/ arg count == 1? if not, call is no-op
/	beq	1b
/	rts	pc
/1:
	mov	4(r5),r1		/ r1 is start of user array
	mov	$statev,r0	/ r0 is rng's statevector
	mov	$269.,r2	/ which is 269 words long

       jsr     pc,blkmov 

	jmp	cret

//.globl  ranmup
//ranmup: mov     $1,r3 
//       br      fred
//.globl  ranmdn
//ranmdn: clr     r3
//fred:
//       tst     (r5)+ 
//       mov     (r5),r1 
//       mov     $statev,r0 
//       mov     $12.,r2 
//       
//       jsr     pc,blkmov 
//       rts     pc
/	.end
	rts	pc

time = 13.
.text
rndmze:
_randomi:
_rndmze_:
	sys	time
	mov	r0,-(sp)
	mov	r1,-(sp)
	xor	r0,r1
	mov	r1,-(sp)
	ror	r0
	rol	r1
	mov	r0,-(sp)
	mov	r1,-(sp)
	xor	r0,r1
	mov	r1,-(sp)

	jsr	pc,_iran

	add	$12.,sp
	rts	pc
