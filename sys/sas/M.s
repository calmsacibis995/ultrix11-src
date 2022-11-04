/ SCCSID: @(#)M.s	3.0	5/12/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ ULTRIX-11 V2.1
/ Startup code for two-stage bootstrap (boot)
/ and system disk load program (sdload).

/ Modified for use with I space only CPUs
/ (11/23,11/24, 11/34, 11/40, & 1160)
/ as well as I & D space CPUs
/ (11/44,11/45, & 11/70).
/
/ Modified to allow unibus disks to be
/ the "root" device (rl01/2,rk06/7, & rm02/3).
/
/ Modified for use with overlay kernel.
/
/ Modified for determining CPU hardware features present
/ and passing them to unix in locore.
/
/ Fred Canter 12/24/83


/ non-UNIX instructions
mfpi	= 6500^tst
stst	= 170300^tst
mfps	= 106700^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
wait	= 1
rti	= 2
rtt	= 6
halt	= 0
reset	= 5
mfpt	= 7
trap	= 104400

tks	= 177564
tkb	= 177566

.globl	_end
.globl	_main
.globl	_sepid,_ubmaps, _cputype, _nmser, _cdreg
.globl	_maxmem,_maxseg, _el_prcw, _rn_ssr3, _mmr3, _cpereg
.globl	_bdcode,_bdunit, _bdmtil, _bdmtih, _bdcsr
.globl	_sdl_bdn,_sdl_bdu, _sdl_ldn, _sdl_ldu
.globl	_devsw
	jmp	start

/ trap vectors

	sbtrap;340	/Bus error
	sbtrap;341	/Illegal instruction
	sbtrap;342	/BPT
	sbtrap;343	/IOT
	sbtrap;344	/Power fail
	sbtrap;345	/EMT
	tstart;346	/Trap instruction (started by Boot:)

.=100^.
	_devsw		/ address of devsw[] table for passing device
			/ CSR addresses from the Boot: program.
			/ replaced by stray vector catcher.

.=114^.
	sbtrap;352	/Memory parity
.=240^.
	sbtrap;347	/Programmed interrupt request
	sbtrap;350	/Floating point
	sbtrap;351	/Memory management

.=1000^.

nxaddr: 0	/ 1 = nonexistent address,after trap
trapok:	0	/ 0 = halt on bus error trap
		/ 1 = rtt on bus error trap & set nxaddr
_sepid: 0	/ 1 = separate I & D space CPU
_ubmaps: 0	/ 1 = unibus map present
_cputype: 45.	/ Assume 11/45 CPU initially
_cdreg: 1	/ Assume CPU has display register
_nmser: 0	/ Number of memory system error registers
_maxmem: 0	/ Total # of 64 byte memory segments
_maxseg: 61440.	/ Memory limit (4MB - I/O page),changed to 65408 for Q22 CPUs
_el_prcw: 0	/ Parity CSR configuration word
_rn_ssr3: 0	/ bits 0->5,saved M/M SSR3
		/ bits 6->15,OS version number (see machdep.c)
_mmr3: 0	/ address of M/M status reg 3 (0 if no SSR3)
_cpereg: -1	/ -1 = no cpu error reg,0 = cpu error reg present
_bdcode: 0	/ Boot device type code
_bdunit: 0	/ Boot device unit number
_bdmtil: 0	/ lo - Boot device media type ID (mscp devices only)
_bdmtih: 0	/ hi	"
_bdcsr:	0	/ Boot device CSR address

/ The following globel symbols are used to pass boot/load device
/ information from the boot program to the sdload program and,
/ in some cases,from sdload back to boot.
/ This works because both programs use the M.s startup code,
/ which ensures the address of the four locations is the
/ same in both programs.
/
/ The boot program uses the following two locations to pass the
/ boot device ID to sdload. This tells sdload where to find the
/ boot and stand-alone programs. The sdload program uses these
/ locations to tell boot where to find the kernel to boot.
_sdl_bdn: 0	/ 2 char boot device name (ht,ts, tm, rx, md)
_sdl_bdu: 0	/ boot device unit number

/ The boot program uses the following two locations to pass the
/ load device ID to sdload. This tells sdload where to find the
/ root and /usr file systems.
_sdl_ldn: 0	/ 2 char load device name (ht,ts, tm, rx)
_sdl_ldu: 0	/ load device unit number

saveps:	0	/ Save the PS on a trap
saveua: 0
saveud: 0

stackov: -1		/Stack overflow indicator
.=.+128.
tstart:			/Started by Boot: instead of block zero boot
	clr	r1	/insure no auto-boot (bad boot device code)
start:
	reset
	mov	$sbtrap,*$34
	mov	$340,PS
	mov	$tstart,sp

/ Save boot device code & unit number for possible auto-boot.

	mov	r0,_bdunit
	mov	r1,_bdcode
	mov	r2,_bdmtil
	mov	r3,_bdmtih
	mov	r4,_bdcsr

/ Load stray vector catchers in unused vector locations

	clr	r0
4:
	mov	$sbtrap,(r0)+
	mov	$357,(r0)+
	br	2f
1:
	tst	(r0)+
	tst	(r0)+
2:
	cmp	r0,$1000
	bge	3f
	tst	2(r0)
	bne	1b
	br	4b
3:
/ Check for CPU error present

	clr	nxaddr
	inc	trapok
	tst	*$CPER
	tst	nxaddr
	bne	1f
	clr	_cpereg
1:

/ Check for separate I & D space CPU,
/ by attempting to set bits 0-2 of 
/ memory management status register 3.
/ If SSR3 is not present or if bits 0-2
/ can't be set,then the CPU does not have 
/ separate I & D space.

	mov	$1,_sepid	/ Assume separate I & D space CPU
	clr	nxaddr
	inc	trapok		/ Allow bus error trap
	mov	$7,*$SSR3	/ Will trap if no SSR3
	tst	nxaddr		/ did a trap occur ?
	bne	1f		/ yes,no SSR3, skip following
	mov	$SSR3,_mmr3	/ save SSR3 address
	tst	*$SSR3		/ no,SSR3 exists, did sep. I&D bits set ?
	bne	2f		/ yes,separate I & D space CPU
	mov	$23.,_cputype
	br	3f
1:
	mov	$40.,_cputype
3:
	clr	_sepid		/ no,non separate I & D space CPU
2:
	clr	trapok

/ If seperate I & D space CPU,
/ set kernel I+D to physical 0 and IO page.
/ If I space only CPU,
/ set kernel I to physical 0 and IO page.

	clr	r1
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)
	tst	_sepid	/ If I space only CPU
	beq	1f	/ Don't set up all MM reg's
	clr	r1
	mov	$KDSA0,r3
	mov	$KDSD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)

/ Set user I+D to physical 64K (words) and IO page.

1:
/	mov	$4000,r1	/ BIGKERNEL
	mov	$6000,r1	/ BIGKERNEL
	mov	$UISA0,r3
	mov	$UISD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)
	tst	_sepid	/ If I space only CPU
	bne	1f
	jmp	ubmtst	/ Don't set up all MM reg's
1:
/	mov	$4000,r1	/ BIGKERNEL
	mov	$6000,r1	/ BIGKERNEL
	mov	$UDSA0,r3
	mov	$UDSD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)

/ Enable 22 bit mapping and seperate I & D
/ space for kernel and user modes.
/ This is not done if CPU is I space only.

	mov	$25,*$SSR3	/ 22-bit mapping
	bit	$20,*$SSR3
	bne	1f
	jmp	ubmtst
1:
	clr	nxaddr
	inc	trapok
	mfpt			/ ask for processor type
	tst	nxaddr		/ does CPU have mpft instruction ?
	beq	1f		/ yes
	mov	$70.,_cputype	/ no,must be 11/70
	mov	$4,_nmser
	mov	$3,*$MSCR
	jmp	ubmtst
1:
	clr	trapok
	clr	nxaddr
	cmp	r0,$1		/ 11/44 ?
	bne	2f		/ no
	mov	$44.,_cputype	/ yes
	mov	$1,*$MSCR
	clr	_cdreg
	mov	$2,_nmser
	jmp	ubmtst
2:
	cmp	r0,$5		/ J11 ?
	bne	4f		/ no
	mov	$73.,_cputype	/ yes,assume 11/73 for now
	mov	$65408.,_maxseg	/ Raise memory size limit for Q22 bus
	clr	_nmser
	clr	nxaddr
	inc	trapok		/ allow bus error trap
	mov	$1,*$MSCR	/ set force miss on error
	clr	trapok
	tst	nxaddr		/ does CPU have cache?
	bne	1f		/ no
	mov	$2,_nmser	/ yes,save number of registers
1:
	clr	_cdreg		/ J11s do not have console display register
	mov	*$MREG,r1	/ get real CPU type from maint. reg.
	ash	$-4,r1
	bic	$!17,r1
	cmp	r1,$1		/ 11/73 ?
	beq	ubmtst		/ yes
	cmp	r1,$2		/ no,ORION ? (11/83 or 11/84)
	bne	3f		/ no
	mov	$83.,_cputype	/ yes,assume 11/83 for now
	br	ubmtst		/ if unibus map present will change to 11/84
3:
	cmp	r1,$3		/ KXJ11 ?
	beq	ubmtst		/ yes (don't know what to do,call it 73)
	cmp	r1,$4		/ 11/53 ? (KDJ11-D)
	bne	4f		/ no
	mov	$53.,_cputype	/ yes
	br	ubmtst
4:
/ SHOULD NEVER GET HERE
/ If we do something is wrong!
/ CPU has mfpt but is not J11 or 11/44 ?
/ If it's a 23 or 24 keep going.
	cmp	r0,$3		/ 11/23 or 11/24 ?
	bne	5f		/ no
	clr	_cdreg
	clr	_nmser
	mov	$23.,_cputype
	br	ubmtst
5:
.if MSMSG
/ SHOULD ABSOLUTELY NEVER GET HERE
/ If we to it's PANIC time!
/ Print UNKNOWN CPU error message, then halt.
/ User can deposit CPU type in R0 and continue,
/ BUT... (who knows what will happen)!
/ ONLY for boot, not sdload (boot loads sdload)
	mov	$3f,r0		/ print UNKNOWN CPU error message
1:
	movb	(r0)+,*$tkb
2:
	tstb	*$tks
	bpl	2b
	tstb	(r0)
	bne	1b
	br	1f
3:
	<\n\rUNKNOWN CPU: load CPU type in R0, continue at your own risk!\n\r\0>
	.even
.endif
1:
	halt
.if READMEM
	br	1b
.endif
	mov	r0,_cputype
ubmtst:
/ Test for unibus map and
/ initialize it if present.
	mov	$UBMR0,r0	/ address of first map reg
	clr	nxaddr
	inc	trapok		/ allow trap
	tst	(r0)		/ touch map reg 0
	tst	nxaddr		/ does it exist ?
	bne	2f		/ no,no unibus map
	cmp	_cputype,$83.	/ Is CPU an ORION ? (11/83 or 11/84)
	bne	3f		/ no
	inc	_cputype	/ yes,has unibus map must be 11/84
	mov	$61440.,_maxseg	/ Lower memory size limit for unibus
3:
	inc	_ubmaps		/ yes,set map present indicator
	mov	$31.,r1		/ initialize unibus map registers
	clr	r2		/ to first 256 kb of memory.
	clr	r3		/ ** - they all get changed later on
1:				/ ** - but what the heck!
	mov	r2,(r0)+
	mov	r3,(r0)+
	add	$20000,r2
	adc	r3
	sob	r1,1b
	bis	$40,*$SSR3	/ enable unibus map
2:
	clr	trapok
	mov	$30340,PS
	inc	*$SSR0

/ Complete CPU type determination.
	tst	_sepid
	bne	4f		/ already know what CPU type
	clr	_cdreg
	clr	_nmser
	cmp	$23.,_cputype
	bne	1f
	inc	trapok
	clr	nxaddr
	tst	*$CPER
	tst	nxaddr
	bne	4f		/ 11/23
	inc	_cputype	/ 11/24
	bis	$20,*$SSR3	/ set 22 bit mapping
	mov	$-1,_cpereg	/ ignore the CPU error register on the
				/ 11/24,it has no meaningfull bits
	br	4f
1:
	inc	trapok
	clr	nxaddr
	clr	r0
	mfps	r0
	tst	nxaddr
	bne	2f		/ reserved inst trap
	bit	$340,r0
	beq	2f
	mov	$34.,_cputype	/ 11/34
	inc	trapok
	clr	nxaddr
	tst	*$MSCR
	tst	nxaddr		/ KK11-A cache option present?
	bne	4f		/ no
	mov	$1,*$MSCR	/ yes,disable cache parity traps
	mov	$2,_nmser
	br	4f
2:
	inc	trapok
	clr	nxaddr
	tst	*$CPER
	tst	nxaddr
	bne	4f
	mov	$60.,_cputype	/ 11/60
	mov	$1,*$MSCR
	mov	$2,_nmser
4:
	clr	trapok

/ Clear all memory after `boot' and set maxmem
/ equal to the total number of 64 byte memory
/ segments available on the system.

.if MSMSG
	mov	$3f,r0		/ print "Sizing Memory..."
1:
	movb	(r0)+,*$tkb
2:
	tstb	*$tks
	bpl	2b
	tstb	(r0)
	bne	1b
	br	1f
3:
	<\n\rSizing Memory...  \0>
	.even
1:
.endif
	mov	*$UISA0,saveua
	mov	*$UISD0,saveud
	mov	$406,*$UISD0
	mov	$_end+63.,r0
	ash	$-6,r0
	bic	$!1777,r0
	mov	r0,*$UISA0
	jsr	pc,sizmem
	mov	*$UISA0,_maxmem

/ At this point,if the memory size is exactly 253952 bytes
/ then the CPU could be:
/	11/23		Without Q22 bus
/	11/23+		With Q22 bus
/	11/24		Without KT24
/ the trick is which one !

	cmp	$23.,_cputype	/ do we think this is an 11/23 CPU
	bne	7f		/ no,forget about extended addressing
	cmp	$7600,_maxmem	/ yes,does memory size equal 253952 bytes ?
	bne	7f		/ no,again no extended addressing
	bis	$20,*$SSR3	/ yes,enable 22 bit mapping
	mov	$10000,*$UISA0	/ look for > 256kb of memory
	clr	nxaddr		/ attempt to clear address 1000000
	inc	trapok
	clr	-(sp)
	mtpi	*$0
	tst	nxaddr		/ trap ?
	beq	2f		/ no,CPU is 11/23
	clr	nxaddr		/ yes
	inc	trapok
	tst	*$177524	/ does CPU have config reg at 777524 ?
	tst	nxaddr
	beq	6f		/ yes,CPU is 11/23+ with exactly 256kb
	inc	_cputype	/ no,CPU is 11/24 without KT24
	br	5f
2:
	cmp	*$0,$sbtrap	/ no,did the address wrap around to 0 ?
	beq	6f		/ no,CPU is 11/23+ with Q22 bus
				/ yes,CPU is 11/23 without Q22 bus
	mov	$sbtrap,*$0	/ restore location 0 contents
5:
	clr	*$SSR3		/ disable 22 bit mapping
	br	7f
6:
	mov	$7600,*$UISA0	/ 11/23+,continue memory sizing
	mov	$65408.,_maxseg	/ Raise memory limit for Q22 bus
	jsr	pc,sizmem
	mov	*$UISA0,_maxmem
7:
	clr	trapok
	mov	saveua,*$UISA0
	mov	saveud,*$UISD0

/ Auto configure code,only used with boot53 (ULCM-16 boot).
/
/ The ULTRIX-11 run time only system supports the ULCM-16 (11/53)
/ and a very limited set of peripherals. The auto configure code
/ determines how the ULCM-16 is configured and saves this information
/ so the kernel can be dynamically reconfigured,later on in boot.
/ Auto-conf operates on the assumption that the system's hardware
/ configuration obays a the following rules:
/
/ Number/type of devices allowed:
/
/	1 console,second SLU (both on CPU module)
/	1 DHV11,2 DZQ11, 2 DLV11-A/B/E/F, 2 DLV11-J
/	1 RQDX (4 drives max)
/	1 TK50
/	1 DEQNA
/
/ CSR address assignments:
/
/	777560	console DL
/	776500	Second SLU (on CPU module)
/	776510	First  DLV11-A/B/E/F
/	    20  Second "
/	    30	Third  "
/	776540	First  DLV11-J
/	   600	Second "
/	760440	Only   DHV11
/	760100	First  DZQ11
/	   110	Second "
/	772150	RQDX
/	774500	TK50
/	774440	DEQNA
/
/ Interrupt vector assignments:
/
/	RQDX = 154,TK50 = 260, DEQNA = 120, comm devices = 300
/	(start at 300,DLV, then DZQ, DHV)
/
/ The auto-conf code builds a series of config records and
/ saves them in _copybuf (in boot.c). The format of each
/ config record is shown below. A zero device type code
/ ends the series of config records.
/
/ config record format
/
/	byte 0 - Device type code
/	byte 1 - Number of controllers present
/	byte 2 - Device CSR address
/	byte 3 -
/	byte 4 - Number of registers
/	byte 5 -
/	byte 6 - Device interrupt vector address
/	byte 7 -
/
/ The config record (also called address map) are built
/ using the following three phases:
/
/  1.	Scan the I/O page and build an address map describing
/	each group of registers that respond (CSR & # of regs).
/
/  2.	Scan the address map to determine,based on the CSR sddress,
/	the type of each device and the number of controllers.
/
/  3.	Determine the interrupt vector for each device. RQDX,TK50, and
/	DEQNA all have software loadable vectors. Comm. device
/	vectors are found by causing the device to interrupt.
/
/ Device type codes (0 ends address map)
/ CAUTION,also defined in boot.c and setup53.c.

NO_DEV	= 1	/ not a device (group of CPU registers)
UN_DEV	= 2	/ unknown device
RA_DEV	= 3	/ RQDX3
DE_DEV	= 4	/ DEQNA
TK_DEV	= 5	/ TK50
UH_DEV	= 6	/ DHV11
DZ_DEV	= 7	/ DZV11 or DZQ11
KL_DEV	= 8.	/ DLV11 (second SLU + any DLV11-A/B/E/F)
DL_DEV	= 9.	/ DLV11-J (ok for KLs to overlap DLs)

.globl	_copybuf

/ Generate an I/O page address map containing the
/ first address and number of addresses for each
/ group of addresses that respond.
/ Flag any group of responding addresses with a length
/ less that 2 words or greater than 8 words as not a
/ device,most likely a group of processor or memory
/ management registers.
/
/   r0	points to current I/O page address
/   r1	save first address in group
/   r2	counts number of addresses in group
/   r3	I/O page length counter (4096 words)
/   r4	not used
/   r5	pointer to address map (_copybuf in boot.c)

.if AUTOCONF
	mov	$160000,r0	/ first address of I/O page
	mov	$4096.,r3	/ I/O page length in words
	clr	r2		/ clear # of registers responding
	mov	$_copybuf,r5	/ address of buffer in boot.c
1:
	clr	nxaddr		/ clear no response indicator
	inc	trapok		/ allow bus timeout trap
	tst	(r0)		/ touch I/O page address
	clr	trapok		/ disallow bus timeout trap
	tst	nxaddr		/ did address respond?
	beq	3f		/ yes
	tst	r2		/ no,is # of regs responding = zero?
	beq	2f		/ yes,go bump counters & loop back
				/ no,end of sequence of responding registers
				/     enter a record into the address map
	mov	$NO_DEV,(r5)	/ Assume no device,i.e., a group of CPU regs
				/ (could be DLV11s,we handle that case later)
	cmp	r1,$164000	/ is CSR in floating address range?
	blo	7f		/ yes,is a device
	cmp	r1,$176500	/ KL_DEV CSR? (will always be at least one)
	beq	7f		/ yes,assume it is a device
	cmp	r1,$176540	/ no,DL_DEV CSR (DLV11-J)
	beq	7f		/ yes,assume it is a device
	cmp	r2,$8.		/ no,# of responding registers greater than 8?
	bgt	5f		/ yes,not a device
	cmp	r2,$2		/ # of responding register less than 2?
	blt	5f		/ yes,not a device
7:
	mov	$UN_DEV,(r5)	/ is a device,don't know what type yet
				/ (also clears # of controllers byte)
5:
	tst	(r5)+		/ move pointer to next map entry
	mov	r1,(r5)+	/ address of first responding register
	mov	r2,(r5)+	/ number of registers responding
	clr	(r5)+		/ clear vector address
	clr	r2		/ # regs = 0 (end group of responding regs)
2:
	add	$2,r0		/ next I/O page address
	sob	r3,1b		/ loop until all I/O page address checked
	clr	(r5)+		/ end the address map by setting the
				/ device type and cntrl count to zero
	br	6f
3:				/ I/O page address responded
	tst	r2		/ is it first address in a group?
	bne	4f		/ no
	mov	r0,r1		/ yes,save first address in group
4:
	inc	r2		/ bump number of addresses in group
	br	2b		/ go check next address
6:
/ Scan thru the address map and see what devices
/ are present. Set device type code and number of
/ controllers. If the device has a fixed vector
/ (RA TK DE) also load the vector.
/
/   r0	scratch
/   r5	pointer to address map (_copybuf in boot.c)
/
	mov	$_copybuf,r5	/ set pointer to address map
1:
	tst	(r5)		/ end of address map?
	beq	7f		/ yes
	cmpb	(r5),$UN_DEV	/ do we think this is a device?
	beq	3f		/ yes,go see what type
				/ no,must be NO_DEV (CPU registers)
2:
	add	$8.,r5		/ move pointer to next map entry
	br	1b		/ loop back
3:				/ attempt to determine device type
	cmp	2(r5),$172150	/ RA CSR?
	bne	4f		/ no,not RA
	cmp	4(r5),$2	/ yes,# reg = 2
	bne	4f		/ no,not RA
	movb	$RA_DEV,(r5)	/ yes,is RA (better be!)
	movb	$1,1(r5)	/ one controller
	mov	$154,6(r5)	/ vector is 154
	br	2b
4:
	cmp	2(r5),$174500	/ TK CSR?
	bne	4f
	cmp	4(r5),$2
	bne	4f
	movb	$TK_DEV,(r5)
	movb	$1,1(r5)
	mov	$260,6(r5)	/ vector is 260
	br	2b
4:
	cmp	2(r5),$174440	/ DE CSR?
	bne	4f
	movb	$DE_DEV,(r5)
	mov	$120,6(r5)	/ vector is 120
6:
	mov	4(r5),r0	/ get # of regs
	asr	r0		/ convert to # controllers
5:
	asr	r0
	asr	r0
	movb	r0,1(r5)	/ save # of controllers
	br	2b
4:
	cmp	2(r5),$160100	/ DZ CSR?
	bne	4f
	movb	$DZ_DEV,(r5)
	mov	4(r5),r0	/ get # regs
	br	5b		/ convert to # cntlr and save
4:
	cmp	2(r5),$160440	/ UH CSR?
	bne	4f
	movb	$UH_DEV,(r5)
	br	6b
4:
	cmp	2(r5),$176500	/ KL CSR (DLV11-A/B/E/F)?
	bne	4f		/ no
	movb	$KL_DEV,(r5)	/ yes,say so
	mov	4(r5),r0	/ convert # regs to # cntlr
	br	5b
4:
	cmp	2(r5),$176540	/ DL CSR (DLV11-J)?
	bne	2b		/ no,must be unknown or unsupported device
	movb	$DL_DEV,(r5)	/ yes,say so
	mov	4(r5),r0
	br	5b
7:
.endif
	jmp	ivect

/ Determine the interrupt vector of each device and
/ save it in the address map.

/ The following are interrupt vector catchers.
/ They catch the interrupt,leave its vector
/ address in r4,then return from interrupt.

iv0:				/ Vectors from 0 -> 74
	mov	*$PS,r4		/ must save PSW first thing
	bic	$!17,r4		/ offset from base in cond. code bits
	asl	r4		/ convert to address offset
	asl	r4
/	add	$0,r4		/ add in base vector address
	rti
iv100:				/ Vectors from 100 -> 174
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$100,r4
	rti
iv200:				/ Vectors from 200 -> 274
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$200,r4
	rti
iv300:				/ Vectors from 300 -> 374
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$300,r4
	rti
iv400:				/ Vectors from 400 -> 474
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$400,r4
	rti
iv500:				/ Vectors from 500 -> 574
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$500,r4
	rti
iv600:				/ Vectors from 600 -> 674
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$600,r4
	rti
iv700:				/ Vectors from 700 -> 774
	mov	*$PS,r4
	bic	$!17,r4
	asl	r4
	asl	r4
	add	$700,r4
	rti

/ Table of vector catcher addresses,used to
/ load catchers into locore vector area.

vcaddr:
	iv0; iv100; iv200; iv300; iv400; iv500; iv600; iv700

/ Subroutine to wait for an interrupt.
/ On exit,r4 will contain the vector address if
/ the device interrupted or -1 if the it timed out.

ivwait:				/ DO NOT USE R3!
.if AUTOCONF
	mov	$-1,r4		/ set r4 to indicate timeout
	mov	$1,r0		/ # of times thru delay loop
	bic	$340,*$PS	/ lower priority so device can interrupt
1:
	mov	$177777,r1	/ delay loop count TODO(cache,inst timing)
2:
	tst	r4		/ catcher sets vector in r4 on interrupt
	bge	3f		/ interrupt occurred
	dec	r1		/ no interrupt yet,continue looping
	bne	2b
	sob	r0,1b
3:
	bis	$340,*$PS	/ raise priority back to 7
.endif
	rts	pc

ivect:				/ load vector catchers in locore
.if AUTOCONF
	clr	r0		/ pointer to vector area
1:
	cmp	2(r0),$357	/ stray vector catcher there now?
	bne	2f		/ no,machine trap vector - don't change
	mov	r0,r1		/ yes,replace with interrupt catcher
	bic	$77,r1		/ set pointer to table of catcher addresses
	ash	$-5,r1
	add	$vcaddr,r1
	mov	(r1),(r0)	/ load catcher addr from table to vector
	mov	r0,r1		/ load offset from base catcher address into
	ash	$-2,r1		/ condition code bits of new PSW (vect+2)
	bic	$!17,r1
	bis	$340,r1
	mov	r1,2(r0)
2:
	tst	(r0)+		/ move pointer to next vector
	tst	(r0)+
	cmp	r0,$1000	/ end of locore vector area?
	blt	1b		/ no,continue

/ Make each communications device reveal its vector by
/ forcing it to interrupt.

	mov	$_copybuf,r5	/ pointer to address map
1:
	cmpb	(r5),$KL_DEV	/ is device DLV11-J
	beq	2f		/ yes
	cmpb	(r5),$DL_DEV	/ is device DLV11?
	beq	2f		/ yes
	cmpb	(r5),$DZ_DEV	/ no,is device DZV11/DZQ11?
	beq	3f		/ yes
	cmpb	(r5),$UH_DEV	/ no,is device DHV11?
	beq	4f		/ yes
5:
	add	$8.,r5		/ no,move pointer to next map entry
	tst	(r5)		/ end of map?
	beq	1f		/ yes,TODO.......
	br	1b		/ no,continue
/ TODO: for just check first device, LATER check each device
/ in a group to make sure vectors are sequential.
2:				/ DLV11,make it interrupt
	mov	2(r5),r3	/ get base CSR address
	bis	$100,4(r3)	/ set xmit interrupt enable
	jsr	pc,ivwait	/ wait for interrupt, vector returned in r4
	bic	$100,4(r3)	/ clear xmit interrupt enable
7:
	tst	r4		/ did device interrupt?
	blt	6f		/ no,r4 = -1
	sub	$4,r4		/ yes,receive vector is first
6:
	mov	r4,6(r5)	/ save vector addr in map
	br	5b		/ go to next device
3:				/ DZV11/DZQ11,make it interrupt
	mov	2(r5),r3	/ get base CSR address
	bis	$1,4(r3)	/ set TCR bit for line 0
	bis	$40040,(r3)	/ set xmit interrupt enable
	jsr	pc,ivwait	/ wait for intr,vector returned in r4
	bic	$40000,(r3)	/ clear xmit intr enable
	br	7b		/ go save vector if device interrupted
4:				/ DHV11,make it interrupt
	mov	2(r5),r3	/ get base CSR address
	mov	$40000,(r3)	/ select line 0 for xmit & set TIE
	mov	$100040,2(r3)	/ xmit a space on line 0
	jsr	pc,ivwait	/ wait for interrupt,vector returned in r4
	bic	$40000,(r3)	/ clear xmit interrupt enable
	br	7b		/ go save vector if device interrupted
1:
/ Replace all of the interrupt vector catchers with
/ stray vector catchers,i.e., address of sbtrap.
/ Any vector not containing the address of sbtrap
/ is assumed to be an interrupt vector catcher.

	clr	r0		/ pointer to locore vector area
1:
	cmp	(r0),$sbtrap	/ is vector an interrupt catcher?
	beq	2f		/ no,don't change it
	mov	$sbtrap,(r0)	/ yes,replace it with stray vector catcher
	mov	$357,2(r0)
2:
	tst	(r0)+		/ move pointer to next vector
	tst	(r0)+
	cmp	r0,$1000	/ end of vector area?
	blt	1b		/ no,continue checking vectors
.endif

/ Enable all parity CSRs

	mov	$PCSR,r0
1:
	clr	nxaddr
	inc	trapok
	mov	$1,(r0)+
	tst	nxaddr
	bne	2f
	mov	$1,r2
	ash	r1,r2
	bis	r2,_el_prcw
2:
	inc	r1
	cmp	r1,$16.
	blt	1b
	clr	trapok

/ Save the release number and
/ SSR3 if it exists

	clr	nxaddr
	inc	trapok
	clr	r0
	mov	*$SSR3,r0
	bic	$!77,r0
	mov	r0,_rn_ssr3
	clr	trapok

/ copy program to user I space
	mov	$_end,r0
	asr	r0
	bic	$100000,r0
	clr	r1
1:
	mov	(r1),-(sp)
	mtpi	(r1)+
	sob	r0,1b

/ continue execution in user space copy

	mov	$160000,sp	/ so loading 407 program doesn't overwrite stack
	mov	$140340,-(sp)
	mov	$user,-(sp)
	rtt
user:
	mov	$_end+512.,sp
	mov	sp,r5

	jsr	pc,_main

	trap

	br	user

setseg:
	mov	$8,r0
1:
	mov	r1,(r3)+
	add	$200,r1
	mov	r2,(r4)+
	sob	r0,1b
	rts	pc

.globl	_setseg
_setseg:
	mov	2(sp),r1
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	tst	_sepid	/If CPU is I space only,
	bne	1f	/ use alternate mapping
	mov	$IO,-(r3)
1:
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

/ Disable separate I & D space,so that
/ non-separate I & D space unix monitors
/ and overlay test monitors can be booted
/ on separate I & D space CPU's.
/
/ _ssid and _snsid must not be called
/ on CPU's without SSR3,such as 11/34,
/ 11/40,& 11/60.

.globl _snsid
_snsid:
	bic	$7,*$SSR3
	bicb	$7,_rn_ssr3
	mov	$IO,*$KISA7	/ map to I/O space
	rts	pc

/ Enable separate I & D space,in case it
/ got turned off somehow.

.globl _ssid
_ssid:
	bis	$5,*$SSR3
	bisb	$5,_rn_ssr3
	rts	pc

/ clrseg(addr,count)
.globl	_clrseg
_clrseg:
	mov	4(sp),r0
	beq	2f
	asr	r0
	bic	$!77777,r0
	mov	2(sp),r1
1:
	clr	-(sp)
	mtpi	(r1)+
	sob	r0,1b
2:
	rts	pc


/ mtpi(word,addr)
.globl	_mtpi
_mtpi:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpi	(r0)+
	rts	pc

/ mfpi(addr),word returned in r0
.globl _mfpi
_mfpi:
	mov	2(sp),r1
	mfpi	(r1)
	mov	(sp)+,r0
	rts	pc

.globl	__rtt
__rtt:
	halt

/ Standalone bootstrap trap handler.
/ Any trap will call the trap handler (no return)
/ if trapok is zero.
/ If trapok is nonzero then bus error and
/ reserved instruction traps will set nxaddr and return.

.globl _trap

sbtrap:
	mov	*$PS,saveps
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	r5,-(sp)
	mov	sp,-(sp)
	sub	$4,(sp)
	cmpb	saveps,$340
	beq	3f
	cmpb	saveps,$341
	bne	1f
3:
	tst	trapok
	bne	2f
1:
	mov	saveps,-(sp)
	jsr	pc,_trap
	halt		/ _trap never returns
	br	.
2:
	inc	nxaddr
	clr	trapok
	tst	(sp)+
	mov	(sp)+,r5
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	rtt

/ Size memory by clearing or reading each word.
/ READMEM says size by reading (sdload - memory disk).
/ READMEM also says only need 512 KB of good memory for sdload,
/ allows installation on systems (like ours) with bad memory above 512KB.
/ UISA0 and UISD0 set by caller.

sizmem:
.if READMEM
	br	1f
.endif
	br	7f		/ Boot: - size memory by clearing
1:				/ sdload: - size memory by reading, so
	clr	nxaddr		/	    memory disk will not be erased.
	mov	$1,trapok
	mfpi	*$0
	tst	nxaddr
	bne	3f
	tst	(sp)+
	clr	r2
	mov	$32.,r1
2:
	mfpi	(r2)+
	tst	(sp)+
	sob	r1,2b
	inc	*$UISA0
	cmp	$8192.,*$UISA0	/ sdload only needs 512KB of good memory
	bhi	1b
	br	3f
7:
	clr	nxaddr
	mov	$1,trapok
	clr	-(sp)
	mtpi	*$0
	tst	nxaddr
	bne	3f
	clr	r2
	mov	$32.,r1
2:
	clr	-(sp)
	mtpi	(r2)+
	sob	r1,2b
	inc	*$UISA0
	cmp	_maxseg,*$UISA0	/ maxseg limits memory size (4MB - I/O page)
	bhi	7b		/ 11/44 - no NXM on 4MB CPU ???
3:
	rts	pc

PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
KDSA0	= 172360
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD5	= 172332
SISA0	= 172240
SISA1	= 172242
SISD0	= 172200
SISD1	= 172202
UISA0	= 177640
UISD0	= 177600
UDSA0	= 177660
UDSD0	= 177620
MSCR	= 177746	/ 11/70 memory control register
UBMR0	= 170200	/ unibus map register base address
CPER	= 177766	/ CPU error register address
PCSR	= 172100	/ Memory parity base CSR address
MREG	= 177750	/ Maintenacne register
IO	= 177600
SWR	= 177570

.data

