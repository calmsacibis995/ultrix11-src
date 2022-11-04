
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)talk_ctl.h	3.0	(ULTRIX-11)	4/22/86
 */

#include "ctl.h"
#include "talk.h"
#include <errno.h>

extern int errno;
#ifdef pdp11
#define	daemon_addr	dmn_addr
#define	daemon_port	dmn_port
#endif pdp11

extern struct sockaddr_in daemon_addr;
extern struct sockaddr_in ctl_addr;
extern struct sockaddr_in my_addr;
extern struct in_addr my_machine_addr;
extern struct in_addr his_machine_addr;
extern u_short daemon_port;
extern int ctl_sockt;
extern CTL_MSG msg;
