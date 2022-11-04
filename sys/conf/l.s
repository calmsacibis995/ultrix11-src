/ low core

.data
ZERO:

br4 = 200
br5 = 240
br6 = 300
br7 = 340

. = ZERO+0
	svi0; br7+0.		/ stray vector thru zero logger
				/ loc 0 = 4, says SID kernel

/ trap vectors
	trap; br7+0.		/ bus error
	trap; br7+1.		/ illegal instruction
	trap; br7+2.		/ bpt-trace trap
	trap; br7+3.		/ iot trap
	trap; br7+4.		/ power fail
	ovint; br5+5.		/ emulator trap
	start;br7+6.		/ system  (overlaid by 'trap')

. = ZERO+40

	svi0; br7+8.
	svi0; br7+9.
	svi0; br7+10.
	svi0; br7+11.

. = ZERO+60
	klin; br4
	klou; br4
	svi0; br7+14.
	svi0; br7+15.

. = ZERO+100
	kwlp; br6
	kwlp; br6
	svi100; br7+2.

. = ZERO+114
	trap; br7+10.		/ 11/70 parity

. = ZERO+120
	dein; br5+0.
	svi100; br7+5.
	svi100; br7+6.
	svi100; br7+7.
	svi100; br7+8.
	svi100; br7+9.

. = ZERO+150
	raio; br5+1.

. = ZERO+154
	raio; br5+0.

. = ZERO+160
	rlio; br5
	svi100; br7+13.
	svi100; br7+14.
	svi100; br7+15.

. = ZERO+200
	lpou; br4

. = ZERO+204
	hpio; br5+1.
	svi200; br7+2.
	svi200; br7+3.
	svi200; br7+4.

. = ZERO+224
	htio; br5
	svi200; br7+6.
	svi200; br7+7.

. = ZERO+240
	trap; br7+7.		/ programmed interrupt
	trap; br7+8.		/ floating point
	trap; br7+9.		/ segmentation violation

. = ZERO+254
	hpio; br5+0.
	svi200; br7+12.
	svi200; br7+13.
	svi200; br7+14.
	svi200; br7+15.

/ floating vectors

. = ZERO+300
	dmin; br4+0.
	svi300; br7+1.

. = ZERO+310
	dhin; br5+0.
	dhou; br5+0.
	svi300; br7+4.
	svi300; br7+5.

. = ZERO+330
	dzin; br5+0.
	dzou; br5+0.
	dzin; br5+1.
	dzou; br5+1.
	dzin; br5+2.
	dzou; br5+2.
	svi300; br7+12.
	svi300; br7+13.
	svi300; br7+14.
	svi300; br7+15.
	svi400; br7+0.
	svi400; br7+1.
	svi400; br7+2.
	svi400; br7+3.
	svi400; br7+4.
	svi400; br7+5.
	svi400; br7+6.
	svi400; br7+7.
	svi400; br7+8.
	svi400; br7+9.
	svi400; br7+10.
	svi400; br7+11.
	svi400; br7+12.
	svi400; br7+13.
	svi400; br7+14.
	svi400; br7+15.
	svi500; br7+0.
	svi500; br7+1.
	svi500; br7+2.
	svi500; br7+3.
	svi500; br7+4.
	svi500; br7+5.
	svi500; br7+6.
	svi500; br7+7.
	svi500; br7+8.
	svi500; br7+9.
	svi500; br7+10.
	svi500; br7+11.
	svi500; br7+12.
	svi500; br7+13.
	svi500; br7+14.
	svi500; br7+15.
	svi600; br7+0.
	svi600; br7+1.
	svi600; br7+2.
	svi600; br7+3.
	svi600; br7+4.
	svi600; br7+5.
	svi600; br7+6.
	svi600; br7+7.
	svi600; br7+8.
	svi600; br7+9.
	svi600; br7+10.
	svi600; br7+11.
	svi600; br7+12.
	svi600; br7+13.
	svi600; br7+14.
	svi600; br7+15.
	svi700; br7+0.
	svi700; br7+1.
	svi700; br7+2.
	svi700; br7+3.
	svi700; br7+4.
	svi700; br7+5.
	svi700; br7+6.
	svi700; br7+7.
	svi700; br7+8.
	svi700; br7+9.
	svi700; br7+10.
	svi700; br7+11.
	svi700; br7+12.
	svi700; br7+13.
	svi700; br7+14.
	svi700; br7+15.

. = ZERO+1000

/ Interface to stray vector error logging

.text
.globl _sv0, _sv100, _sv200, _sv300, _sv400, _sv500, _sv600, _sv700

/ BPT required for jump/vector -> zero error handling.
3	/ BPT - DO NOT MOVE !
0	/ Place holder, DO NOT MOVE !

svi0:	jsr	r0,call; jmp _sv0	/stray vector from 0-74
svi100:	jsr	r0,call; jmp _sv100
svi200:	jsr	r0,call; jmp _sv200
svi300:	jsr	r0,call; jmp _sv300
svi400:	jsr	r0,call; jmp _sv400
svi500:	jsr	r0,call; jmp _sv500
svi600:	jsr	r0,call; jmp _sv600
svi700:	jsr	r0,call; jmp _sv700

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.globl	call, trap

.globl	_klrint
klin:	jsr	r0,call; jmp _klrint
.globl	_klxint
klou:	jsr	r0,call; jmp _klxint

.globl	_trap
.globl	_inEMT
ovint:	jsr	r0,call; inc *_inEMT; jmp _trap

.globl	_clock
kwlp:	jsr	r0,call; jmp _clock


.globl	_deint
dein:	jsr	r0,call; jmp _deint


.globl	_raintr
raio:	jsr	r0,call; jmp _raintr

.globl	_rlintr
rlio:	jsr	r0,call; jmp _rlintr

.globl _lpintr
lpou:	jsr	r0,call; jmp _lpintr

.globl	_htintr
htio:	jsr	r0,call; jmp _htintr

.globl	_hpintr
hpio:	jsr	r0,call; jmp _hpintr

.globl	_dmint
dmin:	jsr	r0,call; jmp _dmint

.globl	_dhrint
dhin:	jsr	r0,call; jmp _dhrint
.globl	_dhxint
dhou:	jsr	r0,call; jmp _dhxint

.globl _dzrint
dzin:	jsr	r0,call; jmp _dzrint
.globl _dzxint
dzou:	jsr	r0,call; jmp _dzxint

/ Dummy instructions to force loading of certain data
/ structures first in BSS, so they can be mapped by the
/ first unibus map register (_ub_end used for size check).

.globl _cfree

	mov	_cfree,r0	/ WARNING: _cfree must be first



.globl _hp_bads

	mov	_hp_bads,r0

.globl _uda, _ra_rp, _ra_cp

	mov	_uda,r0

	mov	_ra_rp,r0

	mov	_ra_cp,r0

.globl _ub_end

	mov	_ub_end,r0
