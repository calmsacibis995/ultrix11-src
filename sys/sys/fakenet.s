/ SCCSID: @(#)fakenet.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ These are dummy functions that are used to resolve references
/ when the networking code is not included in the kernel.  They
/ are in assembly so that we can make them all reference the same
/ place, rather than take up uneccessary text space.
.text
.globl	_netinit
_netinit:
	jsr	r5,csv
	jmp	cret
.globl	_soo_select
_soo_select:
	jsr	r5,csv
	mov	$1,r0
	jmp	cret

.globl	_soclose
.globl	_soo_ioctl
.globl	_soreceive, _sosend, _soo_stat, _socket, _bind, _listen
.globl	_accept, _connect, _socketpair, _setsockopt, _gsockopt
.globl	_gsockname, _getpeername, _shutdown, _sendit, _recvit

_soclose:
_soo_ioctl:
_soreceive:
_sosend:
_soo_stat:
_socket:
_bind:
_listen:
_accept:
_connect:
_socketpair:
_setsockopt:
_gsockopt:
_gsockname:
_getpeername:
_shutdown:
_sendit:
_recvit:
	jsr	r5,csv
	jsr	pc,_nosys
	jmp	cret
.data
