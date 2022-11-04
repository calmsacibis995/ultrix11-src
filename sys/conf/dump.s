/ SCCSID: @(#)dump.s	3.1	7/8/87
/
/////////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.	/
/   All Rights Reserved. 						/
/   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  	/
/////////////////////////////////////////////////////////////////////////
/
/ ULTRIX-11 Crash Dump Code
/ Must be run through /lib/cpp before assembling
/
/ Fred Canter

#include "mch.h"

nop	= 240
halt	= 0
reset	= 5
blocks  = 16.
IO	= 177600
SSR0	= 177572
SSR3	= 172516
UBMR0	= 170200
KISA0	= 172340
KISA5	= 172352
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
#ifdef	SEP_ID
KDSA5	= 172372
KDSA6	= 172374
#else	SEP_ID
KDSA5	= KISA5
KDSA6	= KISA6
#endif	SEP_ID

/ Mag tape core dump
/ save registers in low core and
/ write all core onto mag tape.
/ entry is thru 1000 abs

/ ***********************************************
/ *						*
/ *	The core dump code must be the first	*
/ *	code in this file and must be in	*
/ *	data space, if CPU has separate I & D.	*
/ *						*
/ ***********************************************

#ifdef	SEP_ID
.data
#endif	SEP_ID
.globl	dump, _ubmaps, _io_bae, _rn_ssr3
dump:

/ save regs r0,r1,r2,r3,r4,r5,r6,KDA6 or KIA6
/ starting at abs location 4

	inc	$-1	/ save reg's on first core
	bne	1f	/ dump attempt only.
	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
				/ 040 = aps, saved by trap()
	mov	$42,r0		/ 042 = saved ka6
	cmp	$350,(r0)	/ ka6 already saved by trap(), -- panic trap?
	bne	2f		/ yes, 042 normally = 0350, don't save KA6
	mov	*$KDSA6,(r0)	/ no, not panic trap, save current ka6
2:
	tst	(r0)+		/ bump pointer to location 044
	mov	sp,(r0)+	/ 044 = current stack pointer
	mov	*$KISA0,(r0)+	/ 046 = first I space PAR
	mov	*$KDSA5,(r0)+	/ 050 = ka5, data mapping register
1:

/ dump all of core (ie to first mt error)
/ onto mag tape. (9 track or 7 track 'binary')

/ The core dump tape hardware addresses are
/ defined in the mch0.s header file.

#if	HTDUMP

	/register usage is as follows

	/reg 0 holds the CSR address for the tm02/3.
	/reg 1 points to UBMAP register 0 low
	/reg 2 is used to contain and calculate memory pointer
	/ for UBMAP register 0 low
	/reg 3 is used to contain and calculate memory pointer
	/ for UBMAP register 0 high
	/reg 4, r4 = 1 for map used, r4 = 0 for map not used.
	/reg 5 is used as an interation counter when mapping is enabled


	clr	r4		/clear map used indicator
	tst	_ubmaps		/unibus map present ?
	beq	2f		/no
	mov	$_io_bae,r0	/yes, is BAE register present?
	tstb	HT_BMAJ(r0)
	bne	2f		/yes, don't need to use unibus map
				/no, will use map

	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	r4		/indicate that UB mapping is needed
	mov	$UBMR0,r1	/point to  map register 0
	clr	r2		/init for low map reg
	clr	r3		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable 22 bit mapping
	mov	r2,(r1)		/load map reg 0 low
	mov	r3,2(r1)	/load map reg 0 high
2:
	/this section of code initializes the TM02/3

	mov	$HTCS1,r0	/get tm02/3 CSR addr
	mov	$40,10(r0)	/tm02/3 subsystem clear
	mov	$1300,32(r0)	/800 BPI + pdp11 mode
	clr	4(r0)		/clear unibus address
	mov	$1,(r0)		/nop command to tm02/3

	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ if mapping not needed then just let the
	/ hardware address registers increment.

3:
	mov	$-8192.,6(r0)	/set frame count
	mov	$-4096.,2(r0)	/set word count
	movb	$61,(r0)	/set write comand + go
				/set ext. mem. bits to 0
1:
	tstb	(r0)		/wait for tm02/3 ready
	bge	1b
	bit	$1,(r0)		/wait for go bit clear
	bne	1b
	bit	$40000,(r0)	/any error ?
	beq	2f		/no, continue xfer
	bit	$4000,10(r0)	/yes, must be NXM error
	beq	.		/hang here if not NXM
	mov	$27,(r0)	/error is NXM, write EOF
	halt			/halt on good dump !
2:
	tst	r4		/mapping?
	beq	3b		/branch if not
	add	$20000,r2	/bump low map
	adc	r3		/carry to high map
	mov	r2,(r1)		/load map reg 0 low
	mov	r3,2(r1)	/load map reg 0 high
	clr	4(r0)		/set bus addr to 0
	br	3b		/do some more
#endif	HTDUMP

#if	TKDUMP
	/ TMSCP magtape core dump code

	/Register usage:
	/ r0, r1, r2: by driver
	/ r3 : contain and calculate memory pointer for UBMAP
	/      register 1 low
	/ r4:  contain and calculate memory pointer for UBMAP
	/      register 1 high
	/ r5:  points to UBMAP register 1


	/ Controller initialization

	s1	= 4000
	go	= 1


	bit	$20,_rn_ssr3	/22bit mapping enabled?
	beq	1f		/if not do not set the bits
	mov	$20,*$SSR3	/enable 22 bit mapping

1:
	clr	tk_ubm		/clear map used flag
	tst	_ubmaps		/unibus map present?
	beq	9f		/no, map setup not needed
	mov	$_io_bae,r0	/yes, BAE reg. present?
	tstb	TK_BMAJ(r0)	/
	bne	9f		/yes, dont need to use unibus map

	/This section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 points to the first 8K
	/UBMAP reg 1 gets updated to point to the current
	/memory area to be copied.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	tk_ubm		/set map used flag
	mov	$UBMR0,r5	/point to  map register 0
	clr	r3		/init for low map reg
	clr	tk_high		/init for high map reg
	clr	(r5)+		/load map reg 0 low
	clr	(r5)+		/load map reg 0 high
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable map and 22 bit addressing
	mov	r3,(r5)		/yes, load map reg 1 low
	mov	tk_high,2(r5)	/     load map reg 1 high

9:
	mov	dtk_csr,r1	/ controller I/O page address
	clr	(r1)+		/ start controller init sequence
				/ move pointer to SA register
	mov	$s1,tk_init	/ set cntlr state test bit to step 1
	mov	$1f,r4		/ address of init seq table
	br	2f		/ branch around table
1:
	100000			/ TK_ERR, init step 1
	tkring			/ address of ringbase
	0			/ hi ringbase address
	go			/ TK go bit
2:
	tst	(r1)		/ error ?
	bmi	.		/ yes, hang on init error
	bit	tk_init,(r1)	/ current step done ?
	beq	2b		/ no
	mov	(r4)+,(r1)	/ yes, load next step info from table
	asl	tk_init		/ change state test bit to next step
	bpl	2b		/ if all steps not done, go back
				/ tk_init now = 100000, TK_OWN bit
	mov	$400,tkchdr+2	/ tape VCID = 1
	mov	$36.,tkchdr	/ command packet length
				/ don't set response packet length,
				/ little shakey but it works.
	mov	$0,tkcmd+4.	/ load drive number
	mov	$11,tkcmd+8.	/ on-line command opcode
	mov	$20000,tkcmd+10.	/ clear serious exception
	mov	$tkring,r2	/ initialize cmd/rsp ring
	mov	$tkrsp,(r2)+	/ address of response packet
	mov	tk_init,(r2)+	/ set TK owner
	mov	$tkcmd,(r2)+	/ address of command packet
	mov	tk_init,(r2)+	/ set TK owner
	mov	tk_high,r4	/save high map reg value in r4 for later use
	mov	-2(r1),r0	/ start TK polling
3:
	tst	tkring+2	/ wait for response, TK_OWN goes to zero
	bmi	3b
	tstb	tkrsp+10.	/ status = SUCCESS ?
	bne	.

	/ TMSCP magtape driver

	clr	tk_ba
	clr	tk_xba
1:
/	mov	$32.,tkchdr		/ length of command packet
	mov	$42,tkcmd+8.		/ write opcode
	mov	$8192.,tkcmd+12.	/ byte count
	mov	tk_ba,tkcmd+16.		/ buffer descriptor, lo bus addr
	mov	tk_xba,tkcmd+18.	/ buffer descriptor, hi bus addr
	mov	$100000,tkring+2	/ set TK owner of response
	mov	$100000,tkring+6	/ set TK owner of command
	mov	*dtk_csr,r0		/ start TK polling
2:
	tst	tkring+2		/ wait for response
	bmi	2b
	tstb	tkrsp+10.		/ does returned status = SUCCESS ?
	beq	2f			/ yes, no error continue dump
	cmpb	$151,tkrsp+10.		/ (NXM) host buffer access error ?
	bne	.			/ no, hang on bad dump
					/ write two tape marks
	mov	$2, tk_cnt		/ count
/	mov	$16.,tkchdr		/ length of command packet
	mov	$44,tkcmd+8.		/ write tape mark opcode
3:
	mov	$100000,tkring+2	/ set TK owner of response
	mov	$100000,tkring+6	/ set TK owner of command
	mov	*dtk_csr,r0		/ start TK polling
4:
	tst	tkring+2		/ wait for response
	bmi	4b
	tstb	tkrsp+10.		/ does returned status = SUCCESS ?
	bne	.			/ no, hang on bad dump
	dec	tk_cnt			/ decrement count
	bne	3b			/ write two tape marks ?
					/ yes, rewind the tape
/	mov	$24.,tkchdr		/ length of command packet
	mov	$45,tkcmd+8.		/ reposition opcode
	mov	$20002,tkcmd+10.	/ rewind & clear serious exception
	mov	$0,tkcmd+12.		/ zero lo byte count
	mov	$0,tkcmd+14.		/ zero hi byte count
	mov	$0,tkcmd+16.		/ zero lo tape mark count
	mov	$0,tkcmd+18.		/ zero hi tape mark count
	mov	$100000,tkring+2	/ set TK owner of response
	mov	$100000,tkring+6	/ set TK owner of command
	mov	*dtk_csr,r0		/ start TK polling
3:
	tst	tkring+2		/ wait for response
	bmi	3b
	tstb	tkrsp+10.		/ does returned status = SUCCESS ?
	bne	.			/ no, hang on bad dump

	halt				/ halt on good dump

2:

	tst	tk_ubm		/mapping ?
	beq	3f		/no, increment bus address
	add	$20000,r3	/bump low map
	adc	r4		/carry to high map
	mov	r3,(r5)		/load map reg 1 low
	mov	r4,2(r5)	/load map reg 0 high
	mov	$20000,tk_ba	/load memory address
	br	1b		/do some more
3:
	add	$8192.,tk_ba		/ advance memory address
	bcc	1b			/ and extended address
	inc	tk_xba			/ if necessary
	br	1b			/ do some more
tk_ba:	0		/ memory address
tk_xba: 0		/ entended memory address
dmp_tk: 0		/ dump device: tk
tk_ubm: 0		/ unibus map used flag
tk_high: 0		/ temp. storage for high map rep for ubmap
tk_init: 0		/ controller state
tk_cnt: 0		/ no. of tape marks
dtk_csr: TKAIP		/ tells Boot: (auto-csr select)
			/ Controller TMSCP communications area
tkcint:	.=.+2.		/ command ring transition
tkrint:	.=.+2.		/ response ring transition
tkring:	.=.+8.		/ ring base
tkrhdr:	.=.+4.		/ response header
tkrsp:	.=.+48.		/ response packet
tkchdr:	.=.+4.		/ command header
tkcmd:	.=.+48.		/ command packet
#endif	TKDUMP

#if	TMDUMP
	/register useage is as follows

	/reg 0 holds the tm11 CSR address
	/reg 1 points to UBMAP register 0 low
	/reg 2 is used to contain and calculate memory pointer
	/ for UBMAP register 0 low
	/reg 3 is used to contain and calculate memory pointer
	/ for UBMAP register 0 high
	/reg 4, r4 = 1 for map used, r4 = 0 for map not used.
	/reg 5 is used as an interation counter when mapping is enabled


	clr	r4		/clear UB map used indicator
	tst	_ubmaps		/unibus map present ?
	beq	2f		/no, skip map init
	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	r4		/indicate that UB mapping is needed
	mov	$UBMR0,r1	/point to  map register 0
	clr	r2		/init for low map reg
	clr	r3		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable 22 bit mapping
	mov	r2,(r1)		/load map reg 1 low
	mov	r3,2(r1)	/load map reg 1 high
2:
	/this section of code initializes the TM11

	mov	$MTC,r0		/get tm11 CSR address
	mov	$60004,(r0)	/write command, no go
	clr	4(r0)		/set bus addr to 0
	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ if mapping not needed then just let
	/ bus address register increment.

3:
	mov	$-8192.,2(r0)	/set byte count
	inc	(r0)		/start xfer
1:
	tstb	(r0)		/wait for tm11 ready
	bge	1b
	tst	(r0)		/any error ?
	bge	2f		/no, continue xfer
	bit	$200,-2(r0)	/yes, must be NXM error
	beq	.		/hang if not NXM error
	reset			/error is NXM,
	mov	$60007,(r0)	/write EOF
	halt			/halt on good dump
2:
	tst	r4		/mapping?
	beq	3b		/branch if not
	add	$20000,r2	/bump low map
	adc	r3		/carry to high map
	mov	r2,(r1)		/load map reg 0 low
	mov	r3,2(r1)	/load map reg 0 high
	clr	4(r0)		/set bus address to 0
	br	3b		/do some more
#endif	TMDUMP

#if	TSDUMP

	/register useage is as follows

	/reg 0 points to UBMAP register 1 low
	/reg 1 is used to calculate the current memory address
	/ for each 8192 byte transfer.
	/reg 2 is used to contain and calculate memory pointer
	/ for UBMAP register 1 low
	/reg 3 is used to contain and calculate memory pointer
	/ for UBMAP register 1 high
	/reg 4 points to the command packet
	/reg 5 is used as an interation counter when mapping is enabled


	tst	_ubmaps		/unibus map present ?
	beq	2f		/no, skip map init
	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 points to low memory for the TS11 command,
	/characteristics, and message buffers.
	/UBMAP reg 1 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	setmap		/indicate that UB mapping is needed
	mov	$UBMR0,r0	/point to  map register 0
	clr	r2		/init for low map reg
	clr	r3		/init for high map reg
	clr	(r0)+		/load map reg 0 low
	clr	(r0)+		/load map reg 0 high
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable 22 bit mapping
	mov	r2,(r0)		/load map reg 1 low
	mov	r3,2(r0)	/load map reg 1 high
2:
	/this section of code initializes the TS11

	tstb	*$TSSR		/make sure
	bpl	2b		/drive is ready
	mov	$comts,r4	/point to command packet
	add	$2,r4		/set up mod 4
	bic	$3,r4		/alignment
	mov	$140004,(r4)	/write characteristics command
	mov	$chrts,2(r4)	/characteristics buffer
	clr	4(r4)		/clear ext mem addr (packet)
	clr	tsxma		/clear extended memory save loc
	mov	$10,6(r4)	/set byte count for command
	mov	$mests,*$chrts	/show where message buffer is
	clr	*$chrts+2	/clear extended memory bits here too
	mov	$16,*$chrts+4	/set message buffer length
	mov	r4,*$TSDB	/start command
	clr	r1		/init r1 beginning memory address
1:
	tstb	*$TSSR		/wait for ready
	bpl	1b		/not yet
	mov	*$TSSR,tstcc	/error condition (SC) ?
	bpl	2f		/no error

	    / NXM test moved here to help out TK25
	bit	$4000,*$TSSR	/is error NXM ?
	bne	8f
	bic	$!16,tstcc	/yes error, get TCC
	cmp	tstcc,$10	/recoverable error ?
	bne	.		/no, hang (not sure of good dump)
	mov	$101005,(r4)	/yes, load write data retry command
	clr	4(r4)		/clear packet ext mem addr
	mov	r4,*$TSDB	/start retry
	br	1b
	/bit	$4000,*$TSSR	/is error NXM ?
	/beq	.		/no, hang (not sure of good dump)
8:
	mov	$140013,(r4)	/load a TS init command
	mov	r4,*$TSDB	/to clear NXM error
6:
	tstb	*$TSSR		/wait for ready
	bpl	6b
	mov	$1,6(r4)	/set word count = 1
	mov	$100011,(r4)	/load write EOF command
	mov	r4,*$TSDB	/do write EOF
7:
	tstb	*$TSSR		/wait for ready
	bpl	7b
	halt			/halt after good dump
9:
	br	1b
2:
	/If mapping is needed this section calculates the
	/ base address to be loaded into map reg 1
	/ the algorithm is (!(r5 - 21))*1000) | 20000
	/ the complement is required because an SOB loop
	/ is being used for the counter
	/This loop causes 20000 bytes to be written
	/before the UBMAP is updated.
	/
	/Now 8K bytes written each time. So no need for a loop: GMM

	tst	setmap		/UBMAP ?
	beq	3f		/no map
	mov	r2,(r0)		/load map reg 1 low
	mov	r3,2(r0)	/load map reg 1 high
	bis	$20000,r1	/select map register 1
	clr	4(r4)		/clear extended memory bits
3:
	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ if mapping not needed then just calculate the
	/ next 8192 byte address pointer

	mov	r1,2(r4)	/load mem address
	mov	tsxma,4(r4)	/load ext mem address
	mov	$8192.,6(r4)	/set byte count
	mov	$100005,(r4)	/set write command
	mov	r4,*$TSDB	/initiate xfer
	tst	setmap		/mapping?
	beq	4f		/branch if not
	add	$20000,r2	/bump low map
	adc	r3		/carry to high map
	br	1b		/do some more
4:
	add	$8192.,r1	/bump address for no mapping
	adc	tsxma		/carry to extended memory bits
	br	1b		/do again

/ The following TS11 command and message buffers,
/ must be in initialized data space instead of
/ bss space. This allows them to be mapped by the
/ first M/M mapping register, which is the only one
/ used durring a core dump.

tsxma:	0	/ts11 extended memory address bits
setmap:	0	/UB map usage indicator
tstcc:	0	/ts11 temp location for TCC
comts:		/ts11 command packet
	0
	0
	0
	0
	0
chrts:		/ts11 characteristics
	0
	0
	0
	0
mests:		/ts11 message buffer
	0
	0
	0
	0
	0
	0
	0

#endif	TSDUMP

#if	RLDUMP
	/register usage is as follows

	/ r0 - disk driver
	/ r1 - disk driver
	/ r2 - disk driver
	/ r3 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 low
	/ r4 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 high
	/ r5 - points to UBMAP register 0 low

	seek	= 6
	rdhdr	= 10
	write	= 12
	sec	= 20.

	mov	_rn_ssr3,r5	/get saved contents of M/M status register 3
	bic	$!60,r5		/throw away separate I & D bits
	tst	r5		/was unibus map and/or 22 bit mapping enabled ?
	beq	2f		/no, forget about them
	mov	r5,*$SSR3	/yes, make sure still enabled
	bit	$40,_rn_ssr3	/unibus map needed ?
	beq	2f		/no, don't need MM either

	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	mov	$UBMR0,r5	/point to  map register 0
	clr	r3		/init for low map reg
	clr	r4		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	bit	$40,_rn_ssr3	/unibus map needed ?
	beq	2f		/no, don't use it
	mov	r3,(r5)		/yes, load map reg 1 low
	mov	r4,2(r5)	/     load map reg 1 high
2:
	mov	$dumplo,rl_blk	/set up start block # (for restart)
	clr	rl_ba
	clr	rl_xba
	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

3:
	/ rl01 & rl02 disk driver.
	/ low order address in rl_blk,
	/ high order in r0.

	clr	r0
	mov	rl_blk,r1
	mov	dmp_csr,r2
	mov	$rdhdr,(r2)
1:
	tstb	(r2)
	bpl	1b
	div	$sec,r0
	ash	$6,r0		/ cylinder << 7
	asl	r1		/ sector
	bis	r1,r0
	mov	r0,rl_tmp
	bic	$177,rl_tmp
	mov	r2,r1
	mov	6(r1),r2
	bic	$177,r2
	sub	rl_tmp,r2
	bcc	1f
	neg	r2
	bis	$4,r2		/ go to larger cylinder number
1:
	inc	r2
	bit	$100,r0
	beq	1f
	bis	$20,r2
1:
	mov	r2,4(r1)
	mov	$seek,(r1)
1:
	tstb	(r1)
	bpl	1b
	/mov	dmp_csr+8.,r1
	add	$8.,r1		/* OHMS - fix 8/20/84 */
	mov	$-256.,-(r1)
	mov	r0,-(r1)
	mov	rl_ba,-(r1)
	/ Load the BAE if it is there.  It only exists on the RLV12, so
	/ we can verify it's existance by the absence of the UNIBUS map
	/ and having 22 bit mapping turned on.
	/ The UNIBUS check was missing:  Dave Borman - 8/1/85
	bit	$40,_rn_ssr3	/UNIBUS map being used?
	bne	6f		/yes, skip the BAE
	bit	$20,_rn_ssr3	/22 bit mapping?
	beq	6f		/no
	mov	rl_xba,6(r1)	/yes, load bus addr ext reg
6:
	mov	dmp_dn,r0
	swab	r0
	mov	rl_xba,r2
	bic	$!3,r2
	ash	$4,r2
	bis	r2,r0
	bis	$write,r0
	mov	r0,-(r1)
1:
	tstb	(r1)
	bge	1b
	tst	(r1)		/error ?
	bpl	2f		/no, continue dump
	bit	$20000,(r1)	/yes, is it NXM ?
	beq	.		/no, bad dump !
5:
	halt			/yes, good dump !
2:
	inc	rl_blk		/ increment block number
	cmp	$dumphi,rl_blk	/end of dump area ?
	ble	5b		/halt if so !
	add	$512.,rl_ba	/advance memory address
	adc	rl_xba		/and BAE if necessary
	bit	$40,_rn_ssr3	/Are we using the UNIBUS map?
	beq	3b		/if not go do some more
	dec	mapcnt		/if the loop count is still positive
	bne	3b		/then go do some more
	mov	$20,mapcnt	/reset loop count
	add	$20000,r3	/bump low map
	adc	r4		/carry to high map
	mov	r3,(r5)		/load map reg 0 low
	mov	r4,2(r5)	/load map reg 0 high
	clr	rl_ba		/set bus address to 0
	br	3b		/do some more
			/ Next two locations used by crash dump copy,
			/ command to locate dump area.
rl_dmplo: dumplo	/ starting block of dump area (in swap area)
rl_dmphi: dumphi	/ highest possible block of dump area
rl_blk: dumplo		/ current block number in dump area
rl_ba:	0		/ memory address
rl_xba:	0		/ ext. memory addr
rl_tmp:	0
mapcnt: 20		/unibus map loop counter
dmp_dn:  dumpdn		/ tells Boot: (auto-unit select)
dmp_csr: DSKCSR		/ tells Boot: (auto-csr select)
#endif	RLDUMP

#if	RADUMP
	/ MSCP disk core dump code

	/ Register usage:
	/ r0, r1, r2: by driver
	/ r3 : contain and calculate memory pointer for UBMAP
	/      register 1 low
	/ r4:  contain and calculate memory pointer for UBMAP
	/      register 1 high
	/ r5:  points to UBMAP register 1

	/ Controller initialization

	s1	= 4000
	go	= 1

	mov	dmp_dn,ra_dn	/ set starting unit number
	bit	$20,_rn_ssr3	/ 22bit mapping enabled?
	beq	1f		/ if not do not set the bits
	mov	$20,*$SSR3	/ enable 22 bit mapping

1:
	clr	ra_ubm		/ clear map used flag
	tst	_ubmaps		/ unibus map present?
	beq	9f		/ no, map setup not needed

	/This section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	ra_ubm		/ set map used flag
	mov	$UBMR0,r5	/ point to  map register 0
	clr	r3		/ init for low map reg
	clr	ra_high		/ init for high map reg
	clr	(r5)+		/ load map register 0 low
	clr	(r5)+		/ load map register 0 high
	mov	$77406,*$KISD0	/ set KISDR0
	mov	$77406,*$KISD7	/ set KISDR7
	clr	*$KISA0		/ point KISAR0 to low memory
	mov	$IO,*$KISA7	/ point KISAR7 to IO page
	inc	*$SSR0		/ turn on memory mngt
	mov	$60,*$SSR3	/ enable map and 22 bit addressing
	mov	r3,(r5)		/ yes, load map reg 1 low
	mov	ra_high,2(r5)	/      load map reg 1 high

9:
	mov	dmp_csr,r1	/ controller I/O page address
	clr	(r1)+		/ start controller init sequence
				/ move pointer to SA register
	mov	$1000.,r0	/ wait at least 100 Usec bedfore checking SA
1:
	nop
	sob	r0,1b
	mov	$s1,ra_init	/ set cntlr state test bit to step 1
	mov	$1f,r4		/ address of init seq table
	br	2f		/ branch around table
1:
	100000			/ UDA_ERR, init step 1
	ring			/ address of ringbase
	0			/ hi ringbase address
	go			/ UDA go bit
2:
	tst	(r1)		/ error ?
	bmi	.		/ yes, hang on init error
	bit	ra_init,(r1)	/ current step done ?
	beq	2b		/ no
	mov	(r4)+,(r1)	/ yes, load next step info from table
	asl	ra_init		/ change state test bit to next step
	bpl	2b		/ if all steps not done, go back
				/ ra_init now = 100000, UDA_OWN bit
	mov	$36.,cmdhdr	/ command packet length
				/ don't set response packet length,
				/ little shakey but it works.
	mov	ra_dn,udacmd+4.	/ load drive number
	mov	$11,udacmd+8.	/ on-line command opcode
	mov	$ring,r2	/ initialize cmd/rsp ring
	mov	$udarsp,(r2)+	/ address of response packet
	mov	ra_init,(r2)+	/ set UDA owner
	mov	$udacmd,(r2)+	/ address of command packet
	mov	ra_init,(r2)+	/ set UDA owner
	mov	ra_high,r4	/ save high map reg value in r4 for later use
	mov	-2(r1),r0	/ start UDA polling
3:
	tst	ring+2		/ wait for response, UDA_OWN goes to zero
	bmi	3b
#if	RXDUMP
	/ If unit (ra_dn) is not RX50/RX33 try next unit.
	/ Make sure we don't clobber a winchester.

	tst	udarsp+10.	/ on-line successful?
	bne	4f		/ no, try next unit
	mov	udarsp+28.,r0	/ get media type ID
	bic	$!177,r0	/ only check number portion of ID
	cmp	$50.,r0		/ RX50 ?
	beq	2f		/ yes
	cmp	$33.,r0		/ RX33 ?
	beq	2f		/ yes
4:
	cmp	$3,ra_dn	/ did we just try init 3?
	beq	.		/ yes, that all folks!
	inc	ra_dn
	br	9b
2:
#endif	RXDUMP

	/ MSCP disk driver

	mov	$dumplo,ra_blk	/ set up start block # (for restart) 
	clr	ra_ba
	clr	ra_xba
	mov	$dumphi,ra_hidmp	/ get max. dmphi value
#if	RXDUMP
	mov	udarsp+36.,ra_hidmp	/ floppy - use unit size not dmphi
					/ (lo word only, assumes RX < 64k blks)
#else
	sub	$blocks,ra_hidmp	/ subtract 16 since we write in 8K
					/ prevents writing past end of swap area
#endif	RXDUMP
1:
/	mov	$36.,cmdhdr		/ length of command packet
	mov	$42,udacmd+8.		/ write opcode
	mov	$8192.,udacmd+12.	/ byte count
	mov	ra_ba,udacmd+16.	/ buffer descriptor, lo bus addr
	mov	ra_xba,udacmd+18.	/ buffer descriptor, hi bus addr
	mov	ra_blk,udacmd+28.	/ block number low
	clr	udacmd+30.		/ block number hi
	mov	$100000,ring+2		/ set UDA owner of response
	mov	$100000,ring+6		/ set UDA owner of command
	mov	*dmp_csr,r0		/ start UDA polling
4:
	tst	ring+2			/ wait for response
	bmi	4b
	tstb	udarsp+10.		/ does returned status = SUCCESS ?
	beq	2f			/ yes, no error continue dump
/ *** appears that RQDX1 does not support sub-codes
/ *** new microcode fixed above !
#if	RXDUMP
	cmpb	$4,udarsp+10.		/ ignore attention caused
	bne	3f			/   by changing floppies
	mov	$100000,ring+2		/ set UDA owner of response
	mov	*dmp_csr,r0		/ start UDA polling
	br	4b
3:
#endif	RXDUMP
	cmp	$151,udarsp+10.		/ (NXM) host buffer access error ?
	bne	.			/ no, hang on bad dump
5:
	halt				/ yes, halt on good dump
2:
	add	$blocks,ra_blk		/ increment block number
	/cmp	*$ra_hidmp,ra_blk 	/ end of dump area ?
	cmp	ra_hidmp,ra_blk		/ end of dump area?
	bhi	3f 			/ continue dump if not
#if	RXDUMP
	movb	$052,*$177566		/ print * on terminal
	halt				/ wait for user to change diskettes
					/ user continues processor
	clr	ra_blk			/ need to start at the beginning
	mov	$11,udacmd+8.		/ on-line command opcode
	mov	$ring,r2		/ initialize cmd/rsp ring
	mov	$udarsp,(r2)+		/ address of response packet
	mov	ra_init,(r2)+		/ set UDA owner
	mov	$udacmd,(r2)+		/ address of command packet
	mov	ra_init,(r2)+		/ set UDA owner
	mov	*dmp_csr,r0		/ start UDA polling
4:
	tst	ring+2		/ wait for response, UDA_OWN goes to zero
	bmi	4b
	br	3f
#endif	RXDUMP
	br	5b 			/ halt if not RXDUMP & end of swap area

3:
	tst	ra_ubm		/ mapping ?
	beq	4f		/ no, continue dump
	add	$20000,r3	/ bump low map
	adc	r4		/ carry to high map
	mov	r3,(r5)		/ load map reg 0 low
	mov	r4,2(r5)	/ load map reg 0 high
	mov	$20000,ra_ba	/ load memory address
	br	1b		/ do some more
4:
	add	$8192.,ra_ba	/ advance memory address
	bcc	1b		/ and extended address
	inc	ra_xba		/ if necessary
	br	1b		/ do some more
			/ Next two locations used by crash dump copy,
			/ command to locate dump area.
ra_dmplo: dumplo	/ starting block of dump area (in swap area)
ra_dmphi: dumphi 	/ highest possible block of dump area
ra_blk: dumplo		/ current block number in dump area
ra_hidmp: 0		/ highest possible block of dump area
ra_ba:	0		/ memory address
ra_xba: 0		/ entended memory address
ra_ubm: 0		/ unibus map used flag
ra_high: 0		/ temp. storage for high map rep for ubmap
ra_init: 0		/ controller state
ra_dn:	0		/ read unit #, changed if unit is not RX50/RX33
dmp_dn:  dumpdn		/ tells Boot: (auto-unit select (unless RX50))
dmp_csr: DSKCSR		/ tells Boot: (auto-csr select)
#if	RXDUMP
dmp_rx: 0		/ tells Boot: (no auto-unit select)
#endif	RXDUMP
			/ Controller MSCP communications area
cmdint:	.=.+2.		/ command ring transition
rspint:	.=.+2.		/ response ring transition
ring:	.=.+8.		/ ring base
rsphdr:	.=.+4.		/ response header
udarsp:	.=.+48.		/ response packet
cmdhdr:	.=.+4.		/ command header
udacmd:	.=.+48.		/ command packet
#endif	RADUMP

#if	HKDUMP
	/register usage is as follows

	/ r0 - disk driver
	/ r1 - disk driver
	/ r2 - disk driver (CSR address)
	/ r3 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 low
	/ r4 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 high
	/ r5 - points to UBMAP register 0 low

	bit	$40,_rn_ssr3	/unibus map needed ?
	beq	1f		/no, don't need MM either

	/This section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	mov	$UBMR0,r5	/point to  map register 0
	clr	r3		/init for low map reg
	clr	r4		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable map and 22 bit addressing
	mov	r3,(r5)		/yes, load map reg 1 low
	mov	r4,2(r5)	/     load map reg 1 high
1:
	/ initialize the disk hardware
	/ and select the drive.
	/
	/ Attempt to select rk06 first,
	/ if the drive can't be selected as rk06
	/ then try to select it as an rk07,
	/ if that fails then hang !

	mov	dmp_csr,r2	/Disk CSR address
	mov	$40,10(r2)	/subsystem clear
1:
	tstb	(r2)		/wait for controller ready
	bpl	1b
	mov	$23,hk_dtcmd	/rk06 drive type + write cmd
	mov	dmp_dn,10(r2)	/select unit
	mov	$3,(r2)		/pack ack for rk06
1:
	tstb	(r2)		/wait for ready
	bpl	1b
	tst	(r2)		/error ?
	bpl	4f		/no, drive is rk06 !
	bit	$40,14(r2)	/yes, drive type error ?
	beq	3f		/no, fatal error
	mov	$40,10(r2)	/yes, drive is not rk06 (subsystem clear)
1:
	tstb	(r2)		/wait for controller ready
	bpl	1b
	mov	$2023,hk_dtcmd	/rk07 drive type + write cmd
	mov	dmp_dn,10(r2)	/try rk07 unit select
	mov	$2003,(r2)	/pack ack rk07
2:
	tstb	(r2)		/wait for ready
	bpl	2b
	tst	(r2)		/error ?
	bpl	4f		/no, continue dump
3:
	br	.		/hang if can't select drive (bad dump)
4:
	mov	$dumplo,hk_blk	/set up start block # (for restart)
	clr	4(r2)		/clear bus address
	mov	$dumphi,hk_hidmp	/get max. dmphi value
	sub	$blocks,hk_hidmp	/subtract 16 since we write in 8K

	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ rk06 & rk07 disk driver.
	/ low order address in hk_blk,
	/ high order in r0.

6:
	clr	r0
	mov	hk_blk,r1
	div	$22.*3.,r0
	mov	r0,20(r2)	/hkdc
	clr	r0
	div	$22.,r0
	swab	r0
	bis	r1,r0
	mov	r0,6(r2)	/hkda
	mov	$-4096.,2(r2)	/hkwc
	bic	$1400,hk_dtcmd	/=============================================
	bit	$1000,(r2)	/
	beq	1f		/ this code is to preserve the extended
	bis	$1000,hk_dtcmd	/ address bits for non-unibus mapped machines
1:				/
	bit	$400,(r2)	/ OHMS 8/29/84
	beq	1f		/
	bis	$400,hk_dtcmd	/
1:				/=============================================
	mov	hk_dtcmd,(r2)	
1:
	tstb	(r2)
	bge	1b
	tst	(r2)
	bpl	2f
	bit	$4000,10(r2)	/yes, is it NXM ?
	beq	.		/no, bad dump !
5:
	halt			/yes, good dump !
2:
	add	$blocks,hk_blk  /increment block count
	cmp	hk_hidmp,hk_blk	/end of dump area ?
	blos	5b		/halt if so !
	bit	$40,_rn_ssr3	/mapping ?
	beq	6b		/no, continue dump
	add	$20000,r3	/bump low map
	adc	r4		/carry to high map
	mov	r3,(r5)		/load map reg 0 low
	mov	r4,2(r5)	/load map reg 0 high
	clr	4(r2)		/set bus address to 0
	br	6b		/do some more
			/ Next two locations used by crash dump copy,
			/ command to locate dump area.
hk_dmplo: dumplo	/ starting block of dump area (in swap area)
hk_dmphi: dumphi	/ highest possible block of dump area
hk_blk: dumplo		/ current block number in dump area
hk_dtcmd: 0		/ drive type + command
hk_hidmp: 0		/highest possible block of dump area
dmp_dn:  dumpdn		/ tells Boot: (auto-unit select)
dmp_csr: DSKCSR		/ tells Boot: (auto-csr select)
#endif	HKDUMP

#if	RPDUMP
	/register usage is as follows

	/ r0 - disk driver
	/ r1 - disk driver
	/ r2 - disk driver (CSR address)
	/ r3 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 low
	/ r4 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 high
	/ r5 - points to UBMAP register 0 low

	bit	$40,_rn_ssr3	/unibus map needed ?
	beq	1f		/no, don't need MM either

	/This section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	mov	$UBMR0,r5	/point to  map register 0
	clr	r3		/init for low map reg
	clr	r4		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable map and 22 bit addressing
	mov	r3,(r5)		/yes, load map reg 1 low
	mov	r4,2(r5)	/     load map reg 1 high
1:
	/ initialize the disk hardware

	mov	dmp_csr,r2	/Disk CSR address
	mov	dmp_dn,r0	/unit number
	swab	r0		/unit in hi byte
	inc	r0		/idle+go
	mov	r0,(r2)		/do it
	mov	$dumplo,rp_blk	/set up start block # (for restart)
	clr	10(r2)		/clear bus address
	mov	$dumphi,rp_hidmp	/get max. dmphi value
	sub	$blocks,rp_hidmp	/subtract 16 since we write in 8K

	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ rp03 disk driver.
	/ low order address in rp_blk,
	/ high order in r0.
6:
	clr	r0
	mov	rp_blk,r1
	div	$20.*10.,r0
	mov	r0,12(r2)	/rpca
	clr	r0
	div	$10.,r0
	swab	r0
	bis	r1,r0
	mov	r0,14(r2)	/rpda
	mov	$-4096.,6(r2)	/rpwc
	mov	dmp_dn,r1	/unit number
	swab	r1		/in hi byte
	bisb	4(r2),r1	/ OHMS maintain extended address bits
	bicb	$317,r1		/ OHMS
	bis	$3,r1		/write+go
	mov	r1,4(r2)	/load command in rpcs
1:
	tstb	4(r2)
	bge	1b
	tst	4(r2)
	bpl	2f
	bit	$4,2(r2)	/yes, is it NXM ?
	beq	.		/no, bad dump !
5:
	halt			/yes, good dump !
2:
	add	$blocks,rp_blk	/increment block count
	cmp	rp_hidmp,rp_blk	/end of dump area ?
	blos	5b		/halt if so !
	bit	$40,_rn_ssr3	/mapping ?
	beq	6b		/no, continue dump
	add	$20000,r3	/bump low map
	adc	r4		/carry to high map
	mov	r3,(r5)		/load map reg 0 low
	mov	r4,2(r5)	/load map reg 0 high
	clr	10(r2)		/set bus address to 0
	br	6b		/do some more
			/ Next two locations used by crash dump copy,
			/ command to locate dump area.
rp_dmplo: dumplo	/ starting block of dump area (in swap area)
rp_dmphi: dumphi	/ highest possible block of dump area
rp_blk: dumplo		/ current block number in dump area
rp_hidmp: 0		/highest possible block of dump area
dmp_dn:  dumpdn		/ tells Boot: (auto-unit select)
dmp_csr: DSKCSR		/ tells Boot: (auto-csr select)
#endif	RPDUMP

#if	HPDUMP
	/register usage is as follows

	/ r0 - disk driver
	/ r1 - disk driver
	/ r2 - disk driver (CSR address)
	/ r3 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 low
	/ r4 - is used to contain and calculate memory pointer
	/      for UBMAP register 0 high
	/ r5 - points to UBMAP register 0 low

	write	= 60
	preset	= 20
	go	= 1
	fmt22	= 10000

	clr	hp_ubm		/clear map used flag
	tst	_ubmaps		/unibus map present?
	beq	1f		/no, map setup not needed
	mov	$_io_bae,r0	/yes, rh70 controller?
	tstb	HP_BMAJ(r0)	/is BAE reg. present?
	bne	1f		/yes, RH70 - don't need map

	/This section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	hp_ubm		/set map used flag
	mov	$UBMR0,r5	/point to  map register 0
	clr	r3		/init for low map reg
	clr	r4		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable map and 22 bit addressing
	mov	r3,(r5)		/yes, load map reg 1 low
	mov	r4,2(r5)	/     load map reg 1 high
1:
	/ initialize the disk hardware
	/ and select the drive.
	/
	/ set disk geometry per drive type

	mov	dmp_csr,r2	/Disk CSR address
	mov	$40,10(r2)	/subsystem clear
	mov	dmp_dn,10(r2)	/unit select
	cmpb	26(r2),$24	/ Check drive type, set nsec & ntrk
	blt	9f		/ rp04/5/6
	mov	$32.,hp_nsec	/ rm02/3
	cmpb	26(r2),$27
	bne	8f
	mov	$608.,hp_nbpc	/ rm05
	br	9f
8:
	mov	$5,hp_ntrk
	mov	$160.,hp_nbpc
9:
	mov	$preset+go,(r2)
	mov	$fmt22,32(r2)
	mov	$dumplo,hp_blk	/set up start block # (for restart)
	clr	4(r2)		/clear bus address
	mov	$dumphi,hp_hidmp	/get max. dmphi value
	sub	$blocks,hp_hidmp	/subtract 16 since we write in 8K

	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ rm02/3/5 & rp04/5/6 disk driver
	/ low order address in hp_blk,
	/ high order in r0.

6:
	/ below is added to fix improper divide
	/ there are no high order bits in r0. If there were
	/ there would need to be an add carry below, where hp_blk
	/ is incremented.
	clr	r0
	mov	hp_blk,r1
	div	hp_nbpc,r0
	mov	r0,34(r2)	/hpca
	clr	r0
	div	hp_nsec,r0
	swab	r0
	bis	r1,r0
	mov	r0,6(r2)	/hpda
	mov	$-4096.,2(r2)	/hpwc
	movb	$write+go,(r2)	/hpcs1 - was a "mov" which clobbered
				/ext. address bits on non-unibus mapped mach.
1:
	tstb	(r2)		/wait for ready
	bge	1b
	bit	$40000,(r2)	/error?
	beq	2f		/no, continue dump
	bit	$4000,10(r2)	/yes, is it NXM?
	beq	.		/no, bad dump !
5:
	halt			/yes, good dump !
2:
	add	$blocks,hp_blk	/increment block count
	cmp	hp_hidmp,hp_blk	/end of dump area ?
	blos	5b		/halt if so !
	tst	hp_ubm		/mapping ?
	beq	6b		/no, continue dump
	add	$20000,r3	/bump low map
	adc	r4		/carry to high map
	mov	r3,(r5)		/load map reg 0 low
	mov	r4,2(r5)	/load map reg 0 high
	clr	4(r2)		/set bus address to 0
	br	6b		/do some more
			/ Next two locations used by crash dump copy,
			/ command to locate dump area.
hp_dmplo: dumplo	/ starting block of dump area (in swap area)
hp_dmphi: dumphi	/ highest possible block of dump area
hp_blk: dumplo		/ current block number in dump area
hp_ubm: 0		/ unibus map used flag
hp_hidmp: 0		/highest possible block of dump area
hp_nsec: 22.		/ sectors per track
hp_ntrk: 19.		/ tracks per cylinder
hp_nbpc: 418.		/ blocks per cylinder
dmp_dn:  dumpdn		/ tells Boot: (auto-unit select)
dmp_csr: DSKCSR		/ tells Boot: (auto-csr select)
#endif	HPDUMP
1:
	halt			/ halt if no core dump code
	br	1b		/ included in this monitor
