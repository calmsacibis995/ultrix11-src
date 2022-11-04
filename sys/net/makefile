# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

CFILES= af.c if.c if_de.c if_ether.c if_loop.c if_n1.c if_n2.c \
	if_qe.c if_to_proto.c \
	in.c in_cksum.c in_pcb.c in_proto.c ip_icmp.c ip_if.c \
	ip_input.c ip_output.c raw_cb.c raw_ip.c raw_usrreq.c \
	route.c tcp_debug.c tcp_input.c tcp_output.c tcp_subr.c \
	tcp_timer.c tcp_usrreq.c udp_usrreq.c

OBJ1 =	af.o if.o if_loop.o if_to_proto.o in_cksum.o in_proto.o \
	ip_if.o raw_cb.o raw_ip.o raw_usrreq.o route.o \
	tcp_timer.o tcp_usrreq.o

# These are ones that have to have the hton.ed run on them.
EDOBJ =	if_de.o if_ether.o if_n1.o if_n2.o if_qe.o in.o in_pcb.o ip_icmp.o  \
	ip_input.o ip_output.o tcp_debug.o tcp_input.o tcp_output.o \
	tcp_subr.o udp_usrreq.o
OBJECTS = ${OBJ1} ${EDOBJ}

CFLAGS = -O -DKERNEL -DSEP_ID

all: ${OBJECTS}

${EDOBJ}:
	cc -S ${CFLAGS} `basename $@ .o`.c
	ed - `basename $@ .o`.s < ntoh.ed
	as - -V -o $@ `basename $@ .o`.s
	rm -f `basename $@ .o`.s
	@echo

in_cksum.o:
	cc -S ${CFLAGS} in_cksum.c
	ed - in_cksum.s < cksum.ed
	as - -V -o in_cksum.o in_cksum.s
	rm -f in_cksum.s
	@echo

tags:	${CFILES}
	ctags ${CFILES}

sccsinfo:
	@sccs info

depend:
	grep '^#include' ${CFILES} | grep -v devmaj | \
	sed -e 's/:[^<]*<\([^>]*\)>.*/: \/usr\/include\/\1/' \
	    -e 's/:[^"]*"\([^"]*\)".*/: \1/' \
	    -e 's/\.c/.o/' >xtmp
	for i in ${CFILES}; do echo `basename $$i .c`.o: $$i; done >>xtmp
	sort xtmp | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	rm -f xtmp
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp makefile makefile.bak
	ed - makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend uses it

af.o: /usr/include/net/af.h /usr/include/sys/mbuf.h /usr/include/sys/param.h
af.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
af.o: /usr/include/sys/socketvar.h af.c
if.o: /usr/include/errno.h /usr/include/net/af.h /usr/include/net/if.h
if.o: /usr/include/netinet/in_systm.h /usr/include/sys/dir.h
if.o: /usr/include/sys/ioctl.h /usr/include/sys/param.h
if.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
if.o: /usr/include/sys/systm.h /usr/include/sys/user.h if.c
if_de.o: /usr/include/net/if.h /usr/include/net/netisr.h
if_de.o: /usr/include/net/route.h /usr/include/netinet/if_ether.h
if_de.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
if_de.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
if_de.o: /usr/include/sys/buf.h /usr/include/sys/errno.h
if_de.o: /usr/include/sys/ioctl.h /usr/include/sys/map.h
if_de.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
if_de.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
if_de.o: /usr/include/sys/systm.h /usr/include/vaxif/if_dereg.h
if_de.o: /usr/include/vaxif/if_uba.h /usr/include/net/if_de.h if_de.c
if_ether.o: /usr/include/errno.h /usr/include/net/if.h
if_ether.o: /usr/include/netinet/if_ether.h /usr/include/netinet/in.h
if_ether.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
if_ether.o: /usr/include/sys/ioctl.h /usr/include/sys/mbuf.h
if_ether.o: /usr/include/sys/param.h /usr/include/sys/socket.h
if_ether.o: /usr/include/sys/systm.h if_ether.c
if_loop.o: /usr/include/errno.h /usr/include/net/if.h /usr/include/net/netisr.h
if_loop.o: /usr/include/net/route.h /usr/include/netinet/in.h
if_loop.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
if_loop.o: /usr/include/netinet/ip_var.h /usr/include/sys/ioctl.h
if_loop.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
if_loop.o: /usr/include/sys/socket.h /usr/include/sys/systm.h if_loop.c
if_qe.o: /usr/include/errno.h /usr/include/net/if.h /usr/include/net/netisr.h
if_qe.o: /usr/include/net/if_qe.h
if_qe.o: /usr/include/net/route.h /usr/include/netinet/if_ether.h
if_qe.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
if_qe.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
if_qe.o: /usr/include/sys/buf.h /usr/include/sys/ioctl.h 
if_qe.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
if_qe.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
if_qe.o: /usr/include/sys/systm.h /usr/include/vaxif/if_qereg.h if_qe.c
if_to_proto.o: /usr/include/net/af.h /usr/include/net/if.h
if_to_proto.o: /usr/include/net/if_to_proto.h /usr/include/netinet/if_ether.h
if_to_proto.o: /usr/include/netinet/in.h /usr/include/sys/mbuf.h
if_to_proto.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
if_to_proto.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
if_to_proto.o: if_to_proto.c
in.o: /usr/include/net/af.h /usr/include/net/if.h /usr/include/net/route.h
in.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
in.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
in.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
in.o: /usr/include/sys/socketvar.h in.c
in_cksum.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
in_cksum.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h in_cksum.c
in_pcb.o: /usr/include/net/if.h /usr/include/net/route.h
in_pcb.o: /usr/include/netinet/in.h /usr/include/netinet/in_pcb.h
in_pcb.o: /usr/include/netinet/in_systm.h /usr/include/sys/dir.h
in_pcb.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
in_pcb.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
in_pcb.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h
in_pcb.o: /usr/include/sys/user.h in_pcb.c
in_proto.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
in_proto.o: /usr/include/netinet/tcp.h /usr/include/sys/domain.h
in_proto.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
in_proto.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h in_proto.c
ip_icmp.o: /usr/include/net/route.h /usr/include/netinet/icmp_var.h
ip_icmp.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
ip_icmp.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_icmp.h
ip_icmp.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
ip_icmp.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
ip_icmp.o: /usr/include/sys/systm.h ip_icmp.c
ip_if.o: /usr/include/net/if.h /usr/include/net/route.h
ip_if.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
ip_if.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
ip_if.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
ip_if.o: /usr/include/sys/socketvar.h ip_if.c
ip_input.o: /usr/include/errno.h /usr/include/net/if.h /usr/include/net/route.h
ip_input.o: /usr/include/netinet/in.h /usr/include/netinet/in_pcb.h
ip_input.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
ip_input.o: /usr/include/netinet/ip_icmp.h /usr/include/netinet/ip_var.h
ip_input.o: /usr/include/netinet/tcp.h /usr/include/sys/domain.h
ip_input.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
ip_input.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
ip_input.o: /usr/include/sys/systm.h ip_input.c
ip_output.o: /usr/include/errno.h /usr/include/net/if.h
ip_output.o: /usr/include/net/route.h /usr/include/netinet/in.h
ip_output.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
ip_output.o: /usr/include/netinet/ip_var.h /usr/include/sys/mbuf.h
ip_output.o: /usr/include/sys/param.h /usr/include/sys/socket.h
ip_output.o: /usr/include/sys/socketvar.h ip_output.c
raw_cb.o: /usr/include/errno.h /usr/include/net/if.h /usr/include/net/raw_cb.h
raw_cb.o: /usr/include/net/route.h /usr/include/netinet/in.h
raw_cb.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
raw_cb.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
raw_cb.o: /usr/include/sys/systm.h raw_cb.c
raw_ip.o: /usr/include/errno.h /usr/include/net/if.h /usr/include/net/raw_cb.h
raw_ip.o: /usr/include/net/route.h /usr/include/netinet/in.h
raw_ip.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
raw_ip.o: /usr/include/netinet/ip_var.h /usr/include/sys/mbuf.h
raw_ip.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
raw_ip.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h raw_ip.c
raw_usrreq.o: /usr/include/errno.h /usr/include/net/if.h
raw_usrreq.o: /usr/include/net/netisr.h /usr/include/net/raw_cb.h
raw_usrreq.o: /usr/include/net/route.h /usr/include/netinet/in_systm.h
raw_usrreq.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
raw_usrreq.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
raw_usrreq.o: /usr/include/sys/socketvar.h raw_usrreq.c
route.o: /usr/include/errno.h /usr/include/net/af.h /usr/include/net/if.h
route.o: /usr/include/net/route.h /usr/include/netinet/in_systm.h
route.o: /usr/include/sys/dir.h /usr/include/sys/ioctl.h
route.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
route.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
route.o: /usr/include/sys/systm.h /usr/include/sys/user.h route.c
tcp_debug.o: /usr/include/errno.h /usr/include/net/if.h
tcp_debug.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_debug.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_debug.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_debug.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_debug.h
tcp_debug.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_debug.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_debug.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_debug.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_debug.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_debug.o: /usr/include/sys/systm.h tcp_debug.c
tcp_input.o: /usr/include/errno.h /usr/include/net/if.h
tcp_input.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_input.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_input.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_input.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_debug.h
tcp_input.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_input.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_input.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_input.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_input.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_input.o: /usr/include/sys/systm.h tcp_input.c
tcp_output.o: /usr/include/errno.h /usr/include/net/route.h
tcp_output.o: /usr/include/netinet/in.h /usr/include/netinet/in_pcb.h
tcp_output.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
tcp_output.o: /usr/include/netinet/ip_var.h /usr/include/netinet/tcp.h
tcp_output.o: /usr/include/netinet/tcp_debug.h /usr/include/netinet/tcp_fsm.h
tcp_output.o: /usr/include/netinet/tcp_seq.h /usr/include/netinet/tcp_timer.h
tcp_output.o: /usr/include/netinet/tcp_var.h /usr/include/netinet/tcpip.h
tcp_output.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
tcp_output.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
tcp_output.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h
tcp_output.o: tcp_output.c
tcp_subr.o: /usr/include/errno.h /usr/include/net/if.h /usr/include/net/route.h
tcp_subr.o: /usr/include/netinet/in.h /usr/include/netinet/in_pcb.h
tcp_subr.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
tcp_subr.o: /usr/include/netinet/ip_icmp.h /usr/include/netinet/ip_var.h
tcp_subr.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_fsm.h
tcp_subr.o: /usr/include/netinet/tcp_seq.h /usr/include/netinet/tcp_timer.h
tcp_subr.o: /usr/include/netinet/tcp_var.h /usr/include/netinet/tcpip.h
tcp_subr.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
tcp_subr.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
tcp_subr.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h tcp_subr.c
tcp_timer.o: /usr/include/errno.h /usr/include/net/if.h
tcp_timer.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_timer.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_timer.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_timer.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_fsm.h
tcp_timer.o: /usr/include/netinet/tcp_seq.h /usr/include/netinet/tcp_timer.h
tcp_timer.o: /usr/include/netinet/tcp_var.h /usr/include/netinet/tcpip.h
tcp_timer.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
tcp_timer.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
tcp_timer.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h tcp_timer.c
tcp_usrreq.o: /usr/include/errno.h /usr/include/net/if.h
tcp_usrreq.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_usrreq.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_usrreq.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_usrreq.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_debug.h
tcp_usrreq.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_usrreq.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_usrreq.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_usrreq.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_usrreq.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_usrreq.o: /usr/include/sys/systm.h tcp_usrreq.c
udp_usrreq.o: /usr/include/errno.h /usr/include/net/if.h
udp_usrreq.o: /usr/include/net/route.h /usr/include/netinet/in.h
udp_usrreq.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
udp_usrreq.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_icmp.h
udp_usrreq.o: /usr/include/netinet/ip_var.h /usr/include/netinet/udp.h
udp_usrreq.o: /usr/include/netinet/udp_var.h /usr/include/sys/dir.h
udp_usrreq.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
udp_usrreq.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
udp_usrreq.o: /usr/include/sys/socketvar.h /usr/include/sys/user.h udp_usrreq.c
