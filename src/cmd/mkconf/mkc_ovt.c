
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkc_ovt.c	3.0	4/21/86";
#include	"mkconf.h"
struct	ovtab ovt [] =
{
/*
 *	overlay 1
 * Mostly system calls, will be added to later.
 * Mem driver is about the only thing that
 * will fit.
 * This overlay should be as full as possible,
 * so as not to waste memory space.
 */
	"sys1",     1, 1, -1, "\t../ovsys/sys1.o \\",
	"sys2",     1, 1, -1, "\t../ovsys/sys2.o \\",
	"sys3",     2, 1, -1, "\t../ovsys/sys3.o \\",
	"sys4",     2, 1, -1, "\t../ovsys/sys4.o \\",
/*
 *	overlay 2
 * This overlay contains some system stuff
 * plus bio, will be filled in with tape drivers
 * and what ever else will fit.
 */
	"machdep",  1, 2, -1, "\t../ovsys/machdep.o \\",
	"sig",      1, 2, -1, "\t../ovsys/sig.o \\",
	"fio",      1, 2, -1, "\t../ovsys/fio.o \\",
	"nami",     2, 2, -1, "\t../ovsys/nami.o \\",
	"bio",      2, 2, -1, "\t../ovdev/bio.o \\",
/*
 *	overlay 3
 * This overlay has the big disk drivers and the
 * disk sort routines. These drivers may not be
 * configured, but in any case this overlay will
 * get as much as possible of the overflow from
 * overlays 1 & 2.
 *
 * If all three disk drivers are configured (hp, hk, & hm)
 * this overlay would overflow, in that case "hm" is
 * changed to overlay 8 on the fly and loaded where ever it
 * will fit on the next pass.
 *
 * The "ra" driver has been moved to overlay 3 because it needs
 * dsort.o for the Micro/pdp-11 only.  If any of the other drivers
 * are convigured, then "ra" probably will not fit in OV3. This does
 * not really matter because "ra" only actualy uses dsort.o on the
 * Micro/pdp-11 (RQDX1), and in that case "ra" will be the only
 * driver convigured in OV3.
 *
 * dsort.o must always be first in OV3.
 */
	"dsort",   0, 3, -1, "\t../ovdev/dsort.o \\",
	"hp",      0, 3, -1, "\t../ovdev/hp.o \\",
	"hk",      0, 3, -1, "\t../ovdev/hk.o \\",
	"ra",      0, 3, -1, "\t../ovdev/ra.o \\",

/*
 *	overlay 4
 * Initially empty, will be filled in with
 * overflow from previous overlays.
 */
	"main",     1, 4, -1, "\t../ovsys/main.o \\",

/*
 *	overlay 5
 * The overlay holds most of the tty drivers and
 * associated routines, not much room for fill.
 * 6/8/83 - DZ moved to overlay 8 (5 got too big)
 */
	"tty",      1, 5, -1, "\t../ovdev/tty.o \\",
	"sys",      1, 5, -1, "\t../ovdev/sys.o \\",
	"kl",       1, 5, -1, "\t../ovdev/kl.o \\",
	"dhdm",     0, 5, -1, "\t../ovdev/dhdm.o \\",
	"dh",       0, 5, -1, "\t../ovdev/dh.o \\",
	"dhfdm",    0, 5, -1, "\t../ovdev/dhfdm.o \\",
	"partab",   1, 5, -1, "\t../ovdev/partab.o \\",
/*
 *	overlay 8
 * This is not a real overlay, all of the
 * modules here will be used to fill out
 * overlays 1 thru 7.
 * Contains the mem driver, pipe.o, prim.o, dz driver,
 * and the magtape drivers.
 * Contains all smaller disk drivers.
 * Contains LP driver and misc. comm. device drivers.
 * Also contains u1 u2 u3 u4, user device drivers !
 */
/* NOTE: order may change as tk.o size shrinks */
	"ttynew",   1, 16, -1, "\t../ovdev/ttynew.o \\",
	"fpsim",    0, 16, -1, "\t../ovsys/fpsim.o \\",
	"if_qe",    0, 16, -1, "\t../ovnet/if_qe.o \\",
	"if_de",    0, 16, -1, "\t../ovnet/if_de.o \\",
	"if_n1",    0, 16, -1, "\t../ovnet/if_n1.o \\",
	"if_n2",    0, 16, -1, "\t../ovnet/if_n2.o \\",
	"rdwri",    1, 16, -1, "\t../ovsys/rdwri.o \\",
	"alloc",    1, 16, -1, "\t../ovsys/alloc.o \\",
	"tk",       0, 16, -1, "\t../ovdev/tk.o \\",
	"ts",       0, 16, -1, "\t../ovdev/ts.o \\",
	"uh",       0, 16, -1, "\t../ovdev/uh.o \\",
	"rl",       0, 16, -1, "\t../ovdev/rl.o \\",
	"ht",       0, 16, -1, "\t../ovdev/ht.o \\",
	"iget",	    1, 16, -1, "\t../ovsys/iget.o \\",
	"dz",       0, 16, -1, "\t../ovdev/dz.o \\",
	"pty",      0, 16, -1, "\t../ovdev/pty.o \\",
	"sys_v7m",  1, 16, -1, "\t../ovsys/sys_v7m.o \\",
	"hx",       0, 16, -1, "\t../ovdev/hx.o \\",
	"errlog",   1, 16, -1, "\t../ovsys/errlog.o \\",
	"ubmap",    0, 16, -1, "\t../ovsys/ubmap.o \\",
	"tm",       0, 16, -1, "\t../ovdev/tm.o \\",
	"text",     1, 16, -1, "\t../ovsys/text.o \\",
	"subr",     1, 16, -1, "\t../ovsys/subr.o \\",
	"rp",       0, 16, -1, "\t../ovdev/rp.o \\",
	"du",       0, 16, -1, "\t../ovdev/du.o \\",
	"lp",       0, 16, -1, "\t../ovdev/lp.o \\",
	"rk",       0, 16, -1, "\t../ovdev/rk.o \\",
	"ureg",     1, 16, -1, "\t../ovsys/ureg.o \\",
	"pipe",     1, 16, -1, "\t../ovsys/pipe.o \\", /* was in root */
	"acct",     1, 16, -1, "\t../ovsys/acct.o \\",
	"malloc",   1, 16, -1, "\t../ovsys/malloc.o \\",
	"mem",      1, 16, -1, "\t../ovdev/mem.o \\",
	"ioctl",    1, 16, -1, "\t../ovsys/ioctl.o \\",
	"dn",       0, 16, -1, "\t../ovdev/dn.o \\",
	"sys_berk", 1, 16, -1, "\t../ovsys/sys_berk.o \\",
	"syslocal", 1, 16, -1, "\t../ovsys/syslocal.o \\",
	"select",   1, 16, -1, "\t../ovsys/select.o \\",
	"ipc",      0, 16, -1, "\t../ovsys/ipc.o \\",
	"maus",     0, 16, -1, "\t../ovsys/maus.o \\",
	"shuffle",  0, 16, -1, "\t../ovsys/shuffle.o \\",
	"msg",      0, 16, -1, "\t../ovsys/msg.o \\",
	"sem",      0, 16, -1, "\t../ovsys/sem.o \\",
	"flock",    0, 16, -1, "\t../ovsys/flock.o \\",
	"ct",       0, 16, -1, "\t../ovdev/ct.o \\",
	"fakenet",  0, 16, -1, "\t../ovsys/fakenet.o \\",
	"u1",       0, 16, -1, "\t../ovdev/u1.o \\",
	"u2",       0, 16, -1, "\t../ovdev/u2.o \\",
	"u3",       0, 16, -1, "\t../ovdev/u3.o \\",
	"u4",       0, 16, -1, "\t../ovdev/u4.o \\",
	0
};

/*
 * Overlay table for 0431 kernel,
 * search is linear.
 */

struct ovtab sovt[] =
{
/*
 * The root text segment has overflowed, so we needed to
 * pull something out... Dave Borman
 * AGAIN! 1/25/86 -- Fred Canter (pulled out dsort)
 * NOTE: dsort always configured, all disks but RP driver use it!
 */
	"tty",		1, 16, -1, "\t../dev/tty.o \\",
	"dsort",	1, 16, -1, "\t../dev/dsort.o \\",
/*
 * Group #1: large disk drivers
 */
	"hp",		0, 16, -1, "\t../dev/hp.o \\",
	"hk",		0, 16, -1, "\t../dev/hk.o \\",
	"ra",		0, 16, -1, "\t../dev/ra.o \\",
	"rp",		0, 16, -1, "\t../dev/rp.o \\",
/*
 * Group #2: small disk drivers
 */
	"rl",		0, 16, -1, "\t../dev/rl.o \\",
	"rk",		0, 16, -1, "\t../dev/rk.o \\",
	"hx",		0, 16, -1, "\t../dev/hx.o \\",
/*
 * Group #3: magtape drivers
 */
	"ht",		0, 16, -1, "\t../dev/ht.o \\",
	"ts",		0, 16, -1, "\t../dev/ts.o \\",
	"tm",		0, 16, -1, "\t../dev/tm.o \\",
	"tk",		0, 16, -1, "\t../dev/tk.o \\",
/*
 * Group #4: LP and comm. devices (likely to be used)
 */
/*	"kl",		0, 16, -1, "\t../dev/kl.o \\",	*/
	"dhdm",		0, 16, -1, "\t../dev/dhdm.o \\",
	"dh",		0, 16, -1, "\t../dev/dh.o \\",
	"dhfdm",	0, 16, -1, "\t../dev/dhfdm.o \\",
	"lp",		0, 16, -1, "\t../dev/lp.o \\",
	"dz",		0, 16, -1, "\t../dev/dz.o \\",
	"uh",		0, 16, -1, "\t../dev/uh.o \\",
	"pty",		0, 16, -1, "\t../dev/pty.o \\",
/*
 * Group #5: network devices. (likely to be used with networking)
 */
	"if_qe",	0, 16, -1, "\t../net/if_qe.o \\",
	"if_de",	0, 16, -1, "\t../net/if_de.o \\",
	"if_n1",	0, 16, -1, "\t../net/if_n1.o \\",
	"if_n2",	0, 16, -1, "\t../net/if_n2.o \\",
/*
 * Group #6: comm. devices (unlikely used) + user devices
 */
	"ct",		0, 16, -1, "\t../dev/ct.o \\",
	"dn",		0, 16, -1, "\t../dev/dn.o \\",
	"du",		0, 16, -1, "\t../dev/du.o \\",
	"u1",		0, 16, -1, "\t../dev/u1.o \\",
	"u2",		0, 16, -1, "\t../dev/u2.o \\",
	"u3",		0, 16, -1, "\t../dev/u3.o \\",
	"u4",		0, 16, -1, "\t../dev/u4.o \\",
	0
};

/*
 * optional sys modules
 */
struct ovtab ssovt[] =
{
	"fpsim",	0, 16, -1, "\t../sys/fpsim.o \\",
	"ipc",		0, 16, -1, "\t../sys/ipc.o \\",
	"maus",		0, 16, -1, "\t../sys/maus.o \\",
	"msg",		0, 16, -1, "\t../sys/msg.o \\",
	"sem",		0, 16, -1, "\t../sys/sem.o \\",
	"flock",	0, 16, -1, "\t../sys/flock.o \\",
	"shuffle",	0, 16, -1, "\t../sys/shuffle.o \\",
	0
};

/*
 * Network overlay table
 */
struct ovtab netovt[] =
{
	"af",		1, 16, -1, "\t../ovnet/af.o \\",
	"if",		1, 16, -1, "\t../ovnet/if.o \\",
	"if_ether",	1, 16, -1, "\t../ovnet/if_ether.o \\",
	"if_loop",	1, 16, -1, "\t../ovnet/if_loop.o \\",
	"if_to_proto",	1, 16, -1, "\t../ovnet/if_to_proto.o \\",
	"in",		1, 16, -1, "\t../ovnet/in.o \\",
	"in_pcb",	1, 16, -1, "\t../ovnet/in_pcb.o \\",
	"in_proto",	1, 16, -1, "\t../ovnet/in_proto.o \\",
	"ip_if",	1, 16, -1, "\t../ovnet/ip_if.o \\",
	"in_cksum",	1, 16, -1, "\t../ovnet/in_cksum.o \\",
	"ip_input",	1, 16, -1, "\t../ovnet/ip_input.o \\",
	"ip_icmp",	1, 16, -1, "\t../ovnet/ip_icmp.o \\",
	"tcp_input",	1, 16, -1, "\t../ovnet/tcp_input.o \\",
	"ip_output",	1, 16, -1, "\t../ovnet/ip_output.o \\",
	"tcp_output",	1, 16, -1, "\t../ovnet/tcp_output.o \\",
	"raw_cb",	1, 16, -1, "\t../ovnet/raw_cb.o \\",
	"raw_ip",	1, 16, -1, "\t../ovnet/raw_ip.o \\",
	"raw_usrreq",	1, 16, -1, "\t../ovnet/raw_usrreq.o \\",
	"route",	1, 16, -1, "\t../ovnet/route.o \\",
	"tcp_debug",	1, 16, -1, "\t../ovnet/tcp_debug.o \\",
	"tcp_subr",	1, 16, -1, "\t../ovnet/tcp_subr.o \\",
	"tcp_timer",	1, 16, -1, "\t../ovnet/tcp_timer.o \\",
	"tcp_usrreq",	1, 16, -1, "\t../ovnet/tcp_usrreq.o \\",
	"udp_usrreq",	1, 16, -1, "\t../ovnet/udp_usrreq.o \\",
	"mbuf",		1, 16, -1, "\t../ovsys/mbuf.o \\",
	"mbuf11",	1, 16, -1, "\t../ovsys/mbuf11.o \\",
	"subr_net",	1, 16, -1, "\t../ovsys/subr_net.o \\",
	"sys_socket",	1, 16, -1, "\t../ovsys/sys_socket.o \\",
	"uipc_domain",	1, 16, -1, "\t../ovsys/uipc_domain.o \\",
	"uipc_socket",	1, 16, -1, "\t../ovsys/uipc_socket.o \\",
	"uipc_socket2",	1, 16, -1, "\t../ovsys/uipc_socket2.o \\",
	"uipc_syscall",	1, 16, -1, "\t../ovsys/uipc_syscall.o \\",
	0
};

/*
 * Split I/D Network overlay table
 */
struct ovtab snetovt[] =
{
	"af",		1, 16, -1, "\t../net/af.o \\",
	"if",		1, 16, -1, "\t../net/if.o \\",
	"if_ether",	1, 16, -1, "\t../net/if_ether.o \\",
	"if_loop",	1, 16, -1, "\t../net/if_loop.o \\",
	"if_to_proto",	1, 16, -1, "\t../net/if_to_proto.o \\",
	"in",		1, 16, -1, "\t../net/in.o \\",
	"in_pcb",	1, 16, -1, "\t../net/in_pcb.o \\",
	"in_proto",	1, 16, -1, "\t../net/in_proto.o \\",
	"ip_if",	1, 16, -1, "\t../net/ip_if.o \\",
	"in_cksum",	1, 16, -1, "\t../net/in_cksum.o \\",
	"ip_input",	1, 16, -1, "\t../net/ip_input.o \\",
	"ip_icmp",	1, 16, -1, "\t../net/ip_icmp.o \\",
	"tcp_input",	1, 16, -1, "\t../net/tcp_input.o \\",
	"ip_output",	1, 16, -1, "\t../net/ip_output.o \\",
	"tcp_output",	1, 16, -1, "\t../net/tcp_output.o \\",
	"raw_cb",	1, 16, -1, "\t../net/raw_cb.o \\",
	"raw_ip",	1, 16, -1, "\t../net/raw_ip.o \\",
	"raw_usrreq",	1, 16, -1, "\t../net/raw_usrreq.o \\",
	"route",	1, 16, -1, "\t../net/route.o \\",
	"tcp_debug",	1, 16, -1, "\t../net/tcp_debug.o \\",
	"tcp_subr",	1, 16, -1, "\t../net/tcp_subr.o \\",
	"tcp_timer",	1, 16, -1, "\t../net/tcp_timer.o \\",
	"tcp_usrreq",	1, 16, -1, "\t../net/tcp_usrreq.o \\",
	"udp_usrreq",	1, 16, -1, "\t../net/udp_usrreq.o \\",
	"mbuf",		1, 16, -1, "\t../sys/mbuf.o \\",
	"mbuf11",	1, 16, -1, "\t../sys/mbuf11.o \\",
	"subr_net",	1, 16, -1, "\t../sys/subr_net.o \\",
	"sys_socket",	1, 16, -1, "\t../sys/sys_socket.o \\",
	"uipc_domain",	1, 16, -1, "\t../sys/uipc_domain.o \\",
	"uipc_socket",	1, 16, -1, "\t../sys/uipc_socket.o \\",
	"uipc_socket2",	1, 16, -1, "\t../sys/uipc_socket2.o \\",
	"uipc_syscall",	1, 16, -1, "\t../sys/uipc_syscall.o \\",
	0
};
