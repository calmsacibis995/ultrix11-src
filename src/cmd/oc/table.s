/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ c code tables-- compile to register
/

.globl	_regtab

.data
sccsid:
<@(#)table.s 3.0 4/22/86\0>
.even
_regtab=.
	106.;	cr106
	30.;	cr70
	31.;	cr70
	32.;	cr32
	33.;	cr32
	37.;	cr37
	38.;	cr37
	98.;	cr100
	99.;	cr100
	80.;	cr80
	40.;	cr40
	41.;	cr40	/ - like +
	42.;	cr42
	43.;	cr43
	14.;	cr43
	44.;	cr43
	45.;	cr45
	46.;	cr40
	55.; 	cr40
	48.;	cr40
	49.;	cr49
	70.;	cr70
	71.;	cr70
	72.;	cr72
	73.;	cr73
	74.;	cr74
	75.;	cr75
	76.;	cr72
	78.;	cr70
	85.;	cr70
	79.;	cr79
	102.;	cr102
	51.;	cr51
	52.;	cr52
	56.;	cr56
	57.;	cr57
	58.;	cr58
	59.;	cr59
	91.;	cr91
	82.;	cr82
	83.;	cr82
	84.;	cr82
	86.;	cr86
	87.;	cr86
	88.;	cr86
	16.;	cr16
	92.;	cr92
	17.;	cr43
	18.;	cr74
	109.; cr109
	0
.text

/ goto
cr102:
%a,n
	jmp	A1

%n*,n
	F*
	jmp	#1(R)

/ call
cr100:
%a,n
	jsr	pc,IA1

%n*,n
	F*
	jsr	pc,#1(R)

%n,n
	F
	jsr	pc,(R)

/ addressible
cr106:
%z,n
	clr	R

%zf,n
	clrf	R

%a,n
%ad,n
	movB1	A1,R

%af,n
	movof	A1,R

%n*,n
%nd*,n
	F*
	movB1	#1(R),R

%nf*,n
	F*
	movof	#1(R),R

%al,n
	mov	A1+,R+
	mov	A1,R

%nl*,n
	F*
	mov	#1+2(R),R+
	mov	#1(R),R

%n,n
	F

/ ++,-- postfix
cr32:
%a,1
	movB1	A1',R
	I'B1	A1''

%aw,n
	mov	A1',R
	I	A2,A1''

%e*,1
	F1*
	movB1	#1(R1),R
	I'B1	#1(R1)

%n*,1
	F*
	movB1	#1(R),-(sp)
	I'B1	#1(R)
	movB1	(sp)+,R

%ew*,n
	F1*
	mov	#1(R1),R
	I	A2,#1(R1)

%nw*,n
	F*
	mov	#1(R),-(sp)
	I	A2,#1(R)
	mov	(sp)+,R

%al,1
	F
	I	$1,A1+
	V	A1

%el*,1
	F1*
	mov	#1+2(R1),R+
	mov	#1(R1),R
	I	$1,#1+2(R1)
	V	#1(R1)

%nl*,1
	F*
	mov	#1+2(R),-(sp)
	mov	#1(R),-(sp)
	I	$1,#1+2(R)
	V	#1(R)
	mov	(sp)+,R
	mov	(sp)+,R+

/ - unary, ~
cr37:
%n,n
%nf,n
	F
	IBF	R

%nl,n
	F
	I	R
	I	R+
	V	R

/ =
cr80:
%a,n
%ad,nf
	S
	movB1	R,A1

%af,nf
	S
	movfo	R,A1

%nd*,af
	F*
	S
	movf	R,#1(R)

%n*,aw
	F*
	movB1	A2,#1(R)
	movB1	#1(R),R

%nf*,af
	F*
	S
	movfo	R,#1(R)

%n*,e
	F*
	S1
	movB1	R1,#1(R)
	movB1	R1,R

%ed*,nf
	S
	F1*
	movf	R,#1(R1)

%ef*,nf
	S
	F1*
	movfo	R,#1(R1)

%n*,n
%nd*,nf
	FS*
	S
	movB1	R,*(sp)+

%nf*,nf
	FS*
	S
	movfo	R,*(sp)+

%al,nl
	S
	mov	R+,A1+
	mov	R,A1

%el*,nl
	S
	F1*
	mov	R+,2+#1(R1)
	mov	R,#1(R1)

%nl*,nl
	FS*
	S
	mov	R,*(sp)
	add	$2,(sp)
	mov	R+,*(sp)+

/ field assign, value in reg.
cr16:
%a,n
	S
	bicB1	Z,A1'
	bisB1	R,A1''

%e*,n
%	[fas1]

%n*,n
	SS
	F*
	bicB1	Z,#1(R)
	bisB1	(sp),#1(R)
	mov	(sp)+,R

/ +, -, |, &~, <<
cr40:
%n,z
	F

%n,1
	F
	I'	R

%[add1:]
%n,aw
%nf,ad
	F
	IB2	A2,R

%[add2:]
%n,ew*
%nf,ed*
	F
	S1*
	IB2	#2(R1),R

%[add3:]
%n,e
%nf,ef
	F
	S1
	IBF	R1,R

%[add4:]
%n,nw*
%nf,nd*
	SS*
	F
	IB2	*(sp)+,R

%[add5:]
%n,n
%nf,nf
	SS
	F
	IBF	(sp)+,R

%nl,c
%nl,au
	F
	I	A2,R+
	V	R

%nl,eu
	F
	S1
	I	R1,R+
	V	R

%nl,al
	F
	I	A2,R
	I	A2+,R+
	V	R

%[addl1:]
%nl,el
	F
	S1
	I	R1+,R+
	V	R
	I	R1,R

%[addl2:]
%nl,nl
	SS
	F
	I	(sp)+,R
	I	(sp)+,R+
	V	R

/ ^ -- xor
cr49:
%n,e
%	[add3]

%n,n
	FS
	S
	xor	R,(sp)
	mov	(sp)+,R

%nl,el
%	[addl1]

%nl,nl
	SS
	F
	I	R,(sp)
	mov	(sp)+,R
	I	R+,(sp)
	mov	(sp)+,R+

/ >> (all complicated cases taken care of by << -)
cr45:
%n,1
	F
	asr	R

/ * -- R must be odd on integers
cr42:
%n,aw
%nf,ad
%	[add1]

%n,ew*
%nf,ed*
%	[add2]

%n,e
%nf,ef
%	[add3]

%n,n
%nf,nf
%	[add5]

/ / R must be odd on integers
cr43:
%n,aw
	F
	T
	I	A2,R-

%n,ew*
	F
	T
	S1*
	I	#2(R1),R-

%n,e
	F
	T
	S1
	I	R1,R-

%n,n
	SS
	F
	T
	I	(sp)+,R-

%nf,ad
%	[add1]

%nf,ed*
%	[add2]

%nf,ef
%	[add3]

%nf,nf
%	[add5]

/ =+, =-, =|, =&~
cr70:
%[addq1:]
%aw,aw
	I	A2,A1'
	mov	A1'',R

%[addq1a:]
%a,aw
%ad,ad
	movB1	A1',R
	IBF	A2,R
	movB1	R,A1''

%[addq2:]
%aw,nw*
	S*
	I	#2(R),A1'
	mov	A1'',R

%[addq3:]
%aw,n
	S
	I	R,A1'
	mov	A1'',R

%[addq4:]
%ew*,nw*
	S*
	F1*
	I	#2(R),#1(R1)
	mov	#1(R1),R

%[addq4a:]
%ad,ef
	movf	A1',R
	S1
	IBF	R1,R
	movf	R,A1''

%[addq5:]
%a,n
%ad,nf
	SS
	movB1	A1',R
	IBF	(sp)+,R
	movB1	R,A1''

%[addq6:]
%af,nf
	SS
	movof	A1',R
	IBF	(sp)+,R
	movfo	R,A1''

%[addq7:]
%ew*,n
	S
	F1*
	I	R,#1(R1)
	mov	#1(R1),R

%[addq8:]
%nw*,n
	SS
	F*
	I	(sp)+,#1(R)
	mov	#1(R),R

%[addq9:]
%n*,n
	FS*
	SS
	movB1	*2(sp),R
	IBF	(sp)+,R
	movB1	R,*(sp)+

%[addq9a:]
%nd*,nf
	SS
	F*
	movB1	#1(R),R
	IBF	(sp)+,R
	movB1	R,#1(R)

%[addq10:]
%nf*,nf
	SS
	F*
	movof	#1(R),R1
	IBF	(sp)+,R1
	movfo	R1,#1(R)
	movf	R1,R

%al,c
	I	A2,A1+
	V	A1
	F

%al,al
	I	A2+,A1+
	V	A1
	I	A2,A1
	F

%al,nl
	S
	I	R+,A1+
	V	A1
	I	R,A1
	F

%nl*,c
	F*
	I	A2,#1+2(R)
	V	#1(R)
	mov	#1+2(R),R+
	mov	#1(R),R

%nl*,al
	F*
	I	A2+,#1+2(R)
	V	#1(R)
	I	A2,#1(R)
	mov	#1+2(R),R+
	mov	#1(R),R

%nl*,nl
	SS
	F*
	I	(sp)+,#1(R)
	I	(sp)+,#1+2(R)
	V	#1(R)
	mov	#1+2(R),R+
	mov	#1(R),R

/ =*, =<< (for integer multiply, R must be odd)
cr72:
%a,aw
%ad,ad
%	[addq1a]

%ad,ef
%	[addq4a]

%a,n
%ad,nf
%	[addq5]

%af,nf
%	[addq6]

%n*,n
%	[addq9]

%nd*,nf
%	[addq9a]

%nf*,nf
%	[addq10]

/ =/ ;  R must be odd on integers
cr73:
%a,aw
	movB1	A1',R
	V	R-
	IBF	A2,R-
	movB1	R-,A1''

%a,n
	SS
	movB1	A1',R
	V	R-
	I	(sp)+,R-
	movB1	R-,A1''

%e*,n
	SS
	F1*
	movB1	#1(R1),R
	V	R-
	I	(sp)+,R-
	movB1	R-,#1(R1)

%n*,n
	FS*
	SS
	movB1	*2(sp),R
	V	R-
	I	(sp)+,R-
	movB1	R-,*(sp)+

%ad,ad
%	[addq1a]

%ad,ef
%	[addq4a]

%ad,nf
%	[addq5]

%af,nf
%	[addq6]

%nd*,nf
%	[addq9a]

%nf*,nf
%	[addq10]

/ =mod; R must be odd on integers
cr74:
%a,aw
	movB1	A1',R
	V	R-
	I	A2,R-
	movB1	R,A1''

%a,n
	SS
	movB1	A1',R
	V	R-
	I	(sp)+,R-
	movB1	R,A1''

%e*,n
	SS
	F1*
	movB1	#1(R1),R
	V	R-
	I	(sp)+,R-
	movB1	R,#1(R1)

%n*,n
	FS*
	SS
	movB1	*2(sp),R
	V	R-
	I	(sp)+,R-
	mov	R,*(sp)+

/ =^ -- =xor
cr79:
%aw,n
%	[addq3]

%ab,n
	SS
	movb	A1',R
	xor	R,(sp)
	mov	(sp)+,R
	movb	R,A1''

%n*,n
	FS*
	movB1	*(sp),-(sp)
	S
	xor	R,(sp)
	movB1	(sp)+,R
	movB1	R,*(sp)+

/ =>> (all complicated cases done by =<< -)
cr75:
%a,1
	asrB1	A1'
	movB1	A1'',R

%n*,1
	F*
	asrB1	#1(R)
	movB1	#1(R),R

/ << for longs
cr91:
%nl,aw
%	[add1]

%nl,ew*
%	[add2]

%nl,e
%	[add3]

%nl,nw*
%	[add4]

%nl,n
%	[add5]

/ int -> float
cr51:
%aw,n
	movif	A1,R

%nw*,n
	F*
	movif	#1(R),R

%n,n
	F
	movif	R,R

/ float, double -> int
cr52:
%nf,n
	F
	movfi	R,R

/ double (float) to long
cr56:
%nf,n
	F
	setl
	movfi	R,-(sp)
	mov	(sp)+,R
	mov	(sp)+,R+
	seti

/ long to double
cr57:
%al,n
	setl
	movif	A1,R
	seti

%nl*,n
	F*
	setl
	movif	#1(R),R
	seti

%nl,n
	FS
	setl
	movif	(sp)+,R
	seti

/ integer to long
cr58:
%eu,n
	F1!
	clr	R

%nu,n
	F
	mov	R,R1
	clr	R

%e,n
	F1!
	sxt	R

%n,n
	F
	mov	R,R1
	sxt	R

/ long to integer
cr59:
%al,n
	mov	A1+,R

%nl*,n
	F*
	mov	#1+2(R),R

/ *, /, remainder for longs.
cr82:
%nl,nl
	SS
	FS
	jsr	pc,I
	add	$10,sp

/ =*, =/, =rem for longs
/ Operands of the form &x op y, so stack space is known.
cr86:
%n,nl
	SS
	FS
	jsr	pc,I
	add	$6,sp

/ convert integer to character (sign extend)
cr109:
%n,n
	F
	movb	R,R

/
/ c code tables -- compile for side effects.
/ Also set condition codes properly (except for ++, --)
/

.globl	_efftab

.data
_efftab=.
	30.;	ci70
	31.;	ci70
	32.;	ci70
	33.;	ci70
	80.;	ci80
	70.;	ci70
	71.;	ci70	/ - like +
	78.;	ci78
	79.;	ci79
	85.;	ci78
	75.;	ci75
	76.;	ci76
	16.;	ci16
	116.;	ci116
	0
.text

/ =
ci80:
%[move1:]
%a,z
%ad,zf
	I'B1	A1

%[move2:]
%n*,z
%nd*,zf
	F*
	I'B1	#1(R)

%[move3:]
%a,aw
%ab,a
	IBE	A2,A1

%[move4:]
%ab,n*
%a,nw*
	S*
	IBE	#2(R),A1

%[move5:]
%a,n
	S
	IB1	R,A1

%[move6:]
%n*,aw
%nb*,a
	F*
	IBE	A2,#1(R)

%[move7:]
%n*,ew*
%nb*,e*
	F*
	S1*
	IBE	#2(R1),#1(R)

%[move8:]
%n*,e
	F*
	S1
	IB1	R1,#1(R)

%[move9:]
%e*,nw*
%eb*,n*
	S*
	F1*
	IBE	#2(R),#1(R1)

%[move10:]
%e*,n
	S
	F1*
	IB1	R,#1(R1)

%[move11:]
%n*,nw*
%nb*,n*
	FS*
	S*
	IBE	#2(R),*(sp)+

%[move12:]
%n*,n
	FS*
	S
	IB1	R,*(sp)+

%aw,nf
	S
	movfi	R,A1

%ew*,nf
	S
	F1*
	movfi	R,#1(R1)

%al,z
	clr	A1
	clr	A1+

%nl*,z
	F*
	clr	#1(R)
	clr	2+#1(R)

%[move13a:]
%al,aw
	I	A2,A1+
	V	A1

%al,nw*
	S*
	mov	#2(R),A1+
	V	A1

%al,n
	S
	mov	R,A1+
	V	A1

%al,nf
	S
	setl
	movfi	R,A1
	seti

%el*,nf
	S
	F1*
	setl
	movfi	R,#1(R1)
	seti

%[move13:]
%al,al
	I	A2,A1
	I	A2+,A1+
	V	A1

%[move14:]
%al,nl*
	S*
	I	#2(R),A1
	I	#2+2(R),A1+
	V	A1

%[move15:]
%al,nl
	S
	I	R,A1
	I	R+,A1+
	V	A1

%[move14a:]
%nl*,aw
	F*
	I	A2,#1+2(R)
	V	#1(R)

%[move16a:]
%nl*,al
	F*
	I	A2+,#1+2(R)
	V	#1(R)
	I	A2,#1(R)

%[move16:]
%el*,nl
	S
	F1*
	I	R+,#1+2(R1)
	V	#1(R1)
	I	R,#1(R1)

%nl*,n
	SS
	F*
	mov	(sp)+,#1+2(R)
	V	#1(R)

%[move17:]
%nl*,nl
	SS
	F*
	I	(sp)+,#1(R)
	I	(sp)+,#1+2(R)
	V	#1(R)

/ =| and =& ~
ci78:
%a,aw
%ab,a
%	[move3]

%a,n
%	[move5]

%n*,aw
%nb*,a
%	[move6]

%n*,ew*
%nb*,e*
%	[move7]

%n*,e
%	[move8]

%e*,nw*
%eb*,n*
%	[move9]

%e*,n
%	[move10]

%n*,nw*
%nb*,n*
%	[move11]

%n*,n
%	[move12]

%al,c
%al,au
%	[move13a]

%al,al
%	[move13]

%al,nl*
%	[move14]

%al,nl
%	[move15]

%nl*,c
%	[move14a]

%nl*,al
%	[move16a]

%el*,nl
%	[move16]

%nl*,nl
%	[move17]

/ =^
ci79:
%al,nl
%	[move15]

%el*,nl
%	[move16]

%nl*,nl
	FS*
	S
	I	R,*(sp)
	mov	(sp)+,R
	I	R+,2(R)

/ =+
ci70:
%n*,z
%a,z

%a,1
	I'B1	A1

%aw,aw
%	[move3]

%aw,nw*
%	[move4]

%aw,n
%	[move5]

%n*,1
%	[move2]

%ew*,nw*
%	[move9]

%a,nw*
	S*
	movB1	A1',R1
	I	#2(R),R1
	movB1	R1,A1''

%a,n
	S
	movB1	A1',R1
	I	R,R1
	movB1	R1,A1''

%ew*,n
%	[move10]

%nw*,n
%	[move12]

%n*,n
	SS
	F*
	movB1	#1(R),R1
	I	(sp)+,R1
	movB1	R1,#1(R)

%al,c
%al,au
%	[move13a]

%al,al
%	[move13]

%al,nl*
%	[move14]

%al,nl
%	[move15]

%nl*,c
%nl*,au
%	[move14a]

%nl*,al
%	[move16a]

%el*,nl
%	[move16]

%nl*,nl
%	[move17]

/ =>> (all harder cases handled by =<< -)
ci75:
%a,1
	asrB1	A1

%n*,1
	F*
	asrB1	#1(R)

/ =<<
ci76:
%a,1
	aslB1	A1

%n*,1
	F*
	aslB1	#1(R)

%r,aw
	ash	A2,A1

%r,nw*
	S*
	ash	#2(R),A1

%r,n
	S
	ash	R,A1

/ =<< for longs
cr92:
%al,aw
	F
	ashc	A2,R
	mov	R,A1
	mov	R+,A1+

%al,n
	SS
	F
	ashc	(sp)+,R
	mov	R,A1
	mov	R+,A1+

%nl*,n
	FS*
	SS
	mov	2(sp),R
	mov	2(R),R+
	mov	(R),R
	ashc	(sp)+,R
	mov	R,*(sp)
	add	$2,(sp)
	mov	R+,*(sp)+

/ field = ...
ci16:
%a,a
	bicB1	Z,A1'
	bisB1	A2,A1''

%a,n
	S
	bicB1	Z,A1'
	bisB1	R,A1''

%n*,a
	F*
	bicB1	Z,#1(R)
	bisB1	A2,#1(R)

%[fas1:]
%e*,n
	S
	F1*
	bicB1	Z,#1(R1)
	bisB1	R,#1(R1)

%n*,e
	F*
	S1
	bicB1	Z,#1(R)
	bisB1	R1,#1(R)

%n*,n
	SS
	F*
	bicB1	Z,#1(R)
	bisB1	(sp)+,#1(R)

/
/ c code tables-- set condition codes
/

.globl	_cctab

.data
_cctab=.
	106.;	cc60
	28.;	rest
	55.;	rest
	34.;	rest
	35.;	rest
	36.;	rest
	37.;	rest
	40.;	rest
	41.;	rest
	42.;	rest
	43.;	rest
	81.;	cc81	/ & as in "if ((a&b)==0)"
	48.;	rest
	60.;	cc60
	61.;	cc60
	62.;	cc60
	63.;	cc60
	64.;	cc60
	65.;	cc60
	66.;	cc60
	67.;	cc60
	68.;	cc60
	69.;	cc60
	72.;	rest
	73.;	rest
	79.;	rest
	0
.text

/ relationals
cc60:
%a,z
%ad,zf
%	[move1]

%af,z
	movof	A1,R

%n*,z
%nd*,zf
%	[move2]

%nf*,z
	F*
	movof	#1(R),R

%n,z
%nf,zf
	FC

%aw,aw
%ab,ab
%	[move3]

%nw*,aw
%nb*,ab
%	[move6]

%n,aw
%nf,ad
%	[add1]

%nw*,ew*
%nb*,eb*
%	[move7]

%nw*,e
%	[move8]

%n,ew*
%nf,ed*
%	[add2]

%n,e
%nf,ef
%	[add3]

%nw*,nw*
%nb*,nb*
%	[move11]

%nw*,n
%	[move12]

%n,n
%nf,nf
%	[add5]

%al,z
	tst	A1
	X0
	tst	A1+
	X1

%al,c
%al,au
	tst	A1
	X0
	cmp	A1+,A2
	X1

%[lcmp1:]
%al,al
	I	A1,A2
	X0
	I	A1+,A2+
	X1

%nl*,z
	F*
	tst	#1(R)
	X0
	tst	#1+2(R)
	X1

%nl*,c
%nl*,au
	F*
	tst	#1(R)
	X0
	cmp	#1+2(R),A2
	X1

%[lcmp2:]
%nl*,al
	F*
	I	#1(R),A2
	X0
	I	#1+2(R),A2+
	X1

%nl,z
	F
	tst	R
	X0
	tst	R+
	X1

%nl,c
%nl,au
	F
	tst	R
	X0
	cmp	R+,A2
	X1

%[lcmp3:]
%nl,al
	F
	I	R,A2
	X0
	I	R+,A2+
	X1

%[lcmp4:]
%nl*,el*
	F*
	S1*
	I	#1(R),#2(R1)
	X0
	I	#1+2(R),#2+2(R1)
	X1

%[lcmp5:]
%nl,el*
	F
	S1*
	I	R,#2(R1)
	X0
	I	R+,#2+2(R1)
	X1

%[lcmp6:]
%nl,nl
	FS
	S
	mov	R,-(sp)
	mov	4(sp),R
	mov	(sp)+,2(sp)
	I	(sp)+,(sp)+
	X0
	I	R,R+
	X1

/ & as in "if ((a&b) ==0)"
cc81:
%a,a
%	[move3]

%n*,a
%	[move6]

%n,a
%	[add1]

%n,e
%	[add3]

%n,n
%	[add5]

%al,c
%al,au
	/bit	$0,A1
	/X0
	bit	A2,A1+
	X1

%nl*,c
%nl*,au
	F*
	/bit	$0,#2(R)
	/X0
	bit	A2,#2+2(R)
	X1

%al,al
%	[lcmp1]

%nl*,al
%	[lcmp2]

%nl,al
%	[lcmp3]

%nl*,el*
%	[lcmp4]

%nl,el*
%	[lcmp5]

%nl,nl
%	[lcmp6]

%nl,c
%nl,au
	F
	/bit	$0,R
	/X0
	bit	A2,R+
	X1

/ set codes right
rest:
%n,n
%nf,nf
	H

/
/ c code tables-- expression to -(sp)
/

.globl	_sptab

.data
_sptab=.
	106.;	cs106
	40.;	cs40
	41.;	cs40
	55.;	cs40
	48.;	cs40
	58.;	cs58
	56.;	cs56
	0
.text

/ name
cs106:
%z,n
%zf,n
	clrB1	-(sp)

%aw,n
	mov	A1,-(sp)

%nw*,n
	F*
	mov	#1(R),-(sp)

%al,n
	mov	A1+,-(sp)
	mov	A1,-(sp)

/ +, -, |, &~
cs40:
%a,1
	FS
	I'	(sp)

%a,aw
	FS
	I	A2,(sp)

%a,nw*
	FS
	S*
	I	#2(R),(sp)

%a,n
	FS
	S
	I	R,(sp)

/ integer to long
cs58:
%nu,n
	FS
	clr	-(sp)

%n,n
	FS
	sxt	-(sp)

/ float to long
cs56:
%nf,n
	F
	setl
	movfi	R,-(sp)
	seti

/ setup for structure assign
ci116:
%n,e
	F!
	S1!

%n,n
	SS
	F!
	mov	(sp)+,r1

