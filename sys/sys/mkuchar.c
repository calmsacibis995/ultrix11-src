
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)mkuchar.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>

main()
{
	printf(".globl	_uchar\n");
	printf(".text\n");
	printf("_uchar:\n");
	printf("~~uchar:\n");
	printf("	jsr	r5,csv\n");
	printf("	tst	%o+_u\n", &((struct user *)0)->u_sbuf);
	printf("	jeq	1f\n");
	printf("	jsr	pc,_symchar\n");
	printf("	tst	r0\n");
	printf("	jge	2f\n");
	printf("1:\n");
	printf("	mov	%o+_u,(sp)\n", &((struct user *)0)->u_dirp);
	printf("	inc	%o+_u\n", &((struct user *)0)->u_dirp);
	printf("	jsr	pc,*$_fubyte\n");
	printf("	cmp	$-1,r0\n");
	printf("	jne	1f\n");
	printf("	movb	$16,%o+_u\n", &((struct user *)0)->u_error);
	printf("	jbr	2f\n");
	printf("1:\n");
	printf("	bit	$200,r0\n");
	printf("	jeq	2f\n");
	printf("	movb	$26,%o+_u\n", &((struct user *)0)->u_error);
	printf("2:\n");
	printf("	jmp	cret\n");

	printf("/\n");
	printf("/ srchd - search for entry for nami\n");
	printf("/\n");
	printf("_srchd:\n");
	printf("	mov	r2,-(sp)\n");
	printf("	mov	4(sp),r0	/ r0 - successive directory entries\n");
	printf("	mov	6(sp),r1	/ r1 - what we want to match\n");
	printf("	mov	10(sp),r2	/ r2 is address of do\n");
	printf("1:\n");
	printf("	tst	(r0)		/ check inode\n");
	printf("	beq	4f		/ if the inode is zero\n");
	printf("	cmp	2(r0),(r1)	/ compare first two chars\n");
	printf("	bne	2f		/ if no match\n");
	printf("	cmp	4(r0),2(r1)	/ compare next two chars\n");
	printf("	bne	2f		/ no match\n");
	printf("	cmp	6(r0),4(r1)\n");
	printf("	bne	2f\n");
	printf("	cmp	10(r0),6(r1)\n");
	printf("	bne	2f\n");
	printf("	cmp	12(r0),10(r1)\n");
	printf("	bne	2f\n");
	printf("	cmp	14(r0),12(r1)\n");
	printf("	bne	2f\n");
	printf("	cmp	16(r0),14(r1)\n");
	printf("	beq	3f		/ if a match\n");
	printf("2:\n");
	printf("	add	$%d.,r0\n", sizeof (struct direct));
	printf("	bit	$0%o,r0	/ BMASK test for block offset\n", BMASK);
	printf("	bne	1b		/ more to go...\n");
	printf("	clr	r0		/ indicate failed search\n");
	printf("3:\n");
	printf("	mov	(sp)+,r2	/ restore r2\n");
	printf("	rts	pc\n");
	printf("4:\n");
	printf("	tst	(r2)		/ test to see if already flagged\n");
	printf("	bne	2b		/ if we have\n");
	printf("	mov	r0,(r2)		/ otherwise, write this entry in\n");
	printf("	add	$%d.,(r2)\n", sizeof (struct direct));
	printf("	br	2b\n");
	exit(0);
}
