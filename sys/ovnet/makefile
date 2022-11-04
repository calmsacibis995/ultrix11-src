
SCCSID="%W%	%G%"

CFILES= ../net/af.c ../net/if.c ../net/if_de.c ../net/if_ether.c \
	../net/if_loop.c ../net/if_n1.c ../net/if_n2.c \
	../net/if_qe.c ../net/if_to_proto.c \
	../net/in.c ../net/in_cksum.c ../net/in_pcb.c ../net/in_proto.c \
	../net/ip_icmp.c ../net/ip_if.c ../net/ip_input.c \
	../net/ip_output.c ../net/raw_cb.c ../net/raw_ip.c \
	../net/raw_usrreq.c ../net/route.c ../net/tcp_debug.c \
	../net/tcp_input.c ../net/tcp_output.c ../net/tcp_subr.c \
	../net/tcp_timer.c ../net/tcp_usrreq.c ../net/udp_usrreq.c

# OBJ1 are files that will have strings extracted
# from them, OBJ2 are files that won't.
OBJ1 =	af.o if.o if_loop.o if_to_proto.o ip_if.o raw_cb.o \
	raw_ip.o raw_usrreq.o route.o tcp_timer.o tcp_usrreq.o

# These are ones that have to have the hton.ed run on them.
EDOBJ =	if_de.o if_ether.o if_n1.o if_n2.o if_qe.o in.o in_pcb.o \
	ip_icmp.o ip_input.o ip_output.o tcp_debug.o tcp_input.o \
	tcp_output.o tcp_subr.o udp_usrreq.o

OBJ2 =	in_proto.o in_cksum.o

OBJECTS = ${OBJ1} ${EDOBJ} ${OBJ2}

CFLAGS = -O -N -DK_OV -DKERNEL

all: ${OBJECTS}

${OBJ1}:
	cc -E ${CFLAGS} ../net/`basename $@ .o`.c | xstr -
	cc -S xs.c
	ed - xs.s < asmfix1
	cc -S ${CFLAGS} x.c
	ed - x.s < asmfix2
	as - -V -o $@ x.s xs.s
	rm -f x.c x.s xs.c xs.s
	@echo

${EDOBJ}:
	cc -E ${CFLAGS} ../net/`basename $@ .o`.c | xstr -
	cc -S xs.c
	ed - xs.s < asmfix1
	cc -S ${CFLAGS} x.c
	ed - x.s < ntoh.ed
	ed - x.s < asmfix2
	as - -V -o $@ x.s xs.s
	rm -f x.c x.s xs.c xs.s
	@echo

in_proto.o:
	cc -c ${CFLAGS} ../net/in_proto.c

in_cksum.o:
	cc -S ${CFLAGS} ../net/in_cksum.c
	ed - in_cksum.s < ../net/cksum.ed
	as - -V -o in_cksum.o in_cksum.s
	rm -f in_cksum.s

depend:
	(grep '^#include' ${CFILES} | grep -v devmaj | \
	sed -e 's/:[^<]*<\([^>]*\)>.*/: \/usr\/include\/\1/' \
	    -e 's/:[^"]*"\([^"]*\)".*/: \1/' \
	    -e 's/\.c/.o/' \
	    -e 's,../[a-zA-Z]*/,,' ; \
	for i in ${CFILES}; do echo `basename $$i .c`.o: $$i; done ) | \
	sort | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp makefile makefile.bak
	ed - makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend uses it

af.o: ../net/af.c /usr/include/net/af.h /usr/include/sys/mbuf.h
af.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
af.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
if.o: ../net/if.c /usr/include/errno.h /usr/include/net/af.h
if.o: /usr/include/net/if.h /usr/include/netinet/in_systm.h
if.o: /usr/include/sys/dir.h /usr/include/sys/ioctl.h /usr/include/sys/param.h
if.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
if.o: /usr/include/sys/systm.h /usr/include/sys/user.h
if_de.o: ../net/if_de.c /usr/include/net/if.h /usr/include/net/netisr.h
if_de.o: /usr/include/net/route.h /usr/include/netinet/if_ether.h
if_de.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
if_de.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
if_de.o: /usr/include/sys/buf.h /usr/include/sys/errno.h
if_de.o: /usr/include/sys/ioctl.h /usr/include/sys/map.h
if_de.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
if_de.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
if_de.o: /usr/include/sys/systm.h /usr/include/vaxif/if_dereg.h
if_de.o: /usr/include/vaxif/if_uba.h
if_ether.o: ../net/if_ether.c /usr/include/errno.h /usr/include/net/if.h
if_ether.o: /usr/include/netinet/if_ether.h /usr/include/netinet/in.h
if_ether.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
if_ether.o: /usr/include/sys/ioctl.h /usr/include/sys/mbuf.h
if_ether.o: /usr/include/sys/param.h /usr/include/sys/socket.h
if_ether.o: /usr/include/sys/systm.h
if_loop.o: ../net/if_loop.c /usr/include/errno.h /usr/include/net/if.h
if_loop.o: /usr/include/net/netisr.h /usr/include/net/route.h
if_loop.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
if_loop.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
if_loop.o: /usr/include/sys/ioctl.h /usr/include/sys/mbuf.h
if_loop.o: /usr/include/sys/param.h /usr/include/sys/socket.h
if_loop.o: /usr/include/sys/systm.h
if_qe.o: ../net/if_qe.c /usr/include/errno.h /usr/include/net/if.h
if_qe.o: /usr/include/net/netisr.h /usr/include/net/route.h
if_qe.o: /usr/include/netinet/if_ether.h /usr/include/netinet/in.h
if_qe.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
if_qe.o: /usr/include/netinet/ip_var.h /usr/include/sys/buf.h
if_qe.o: /usr/include/sys/ioctl.h /usr/include/sys/mbuf.h
if_qe.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
if_qe.o: /usr/include/sys/socket.h /usr/include/sys/systm.h
if_qe.o: /usr/include/vaxif/if_qereg.h
if_to_proto.o: ../net/if_to_proto.c /usr/include/net/af.h /usr/include/net/if.h
if_to_proto.o: /usr/include/net/if_to_proto.h /usr/include/netinet/if_ether.h
if_to_proto.o: /usr/include/netinet/in.h /usr/include/sys/mbuf.h
if_to_proto.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
if_to_proto.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
in.o: ../net/in.c /usr/include/net/af.h /usr/include/net/if.h
in.o: /usr/include/net/route.h /usr/include/netinet/in.h
in.o: /usr/include/netinet/in_systm.h /usr/include/sys/mbuf.h
in.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
in.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
in_cksum.o: ../net/in_cksum.c /usr/include/netinet/in.h
in_cksum.o: /usr/include/netinet/in_systm.h /usr/include/sys/mbuf.h
in_cksum.o: /usr/include/sys/param.h
in_pcb.o: ../net/in_pcb.c /usr/include/net/if.h /usr/include/net/route.h
in_pcb.o: /usr/include/netinet/in.h /usr/include/netinet/in_pcb.h
in_pcb.o: /usr/include/netinet/in_systm.h /usr/include/sys/dir.h
in_pcb.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
in_pcb.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
in_pcb.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h
in_pcb.o: /usr/include/sys/user.h
in_proto.o: ../net/in_proto.c /usr/include/netinet/in.h
in_proto.o: /usr/include/netinet/in_systm.h /usr/include/netinet/tcp.h
in_proto.o: /usr/include/sys/domain.h /usr/include/sys/mbuf.h
in_proto.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
in_proto.o: /usr/include/sys/socket.h
ip_icmp.o: ../net/ip_icmp.c /usr/include/net/route.h
ip_icmp.o: /usr/include/netinet/icmp_var.h /usr/include/netinet/in.h
ip_icmp.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
ip_icmp.o: /usr/include/netinet/ip_icmp.h /usr/include/sys/mbuf.h
ip_icmp.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
ip_icmp.o: /usr/include/sys/socket.h /usr/include/sys/systm.h
ip_if.o: ../net/ip_if.c /usr/include/net/if.h /usr/include/net/route.h
ip_if.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
ip_if.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
ip_if.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
ip_if.o: /usr/include/sys/socketvar.h
ip_input.o: ../net/ip_input.c /usr/include/errno.h /usr/include/net/if.h
ip_input.o: /usr/include/net/route.h /usr/include/netinet/in.h
ip_input.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
ip_input.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_icmp.h
ip_input.o: /usr/include/netinet/ip_var.h /usr/include/netinet/tcp.h
ip_input.o: /usr/include/sys/domain.h /usr/include/sys/mbuf.h
ip_input.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
ip_input.o: /usr/include/sys/socket.h /usr/include/sys/systm.h
ip_output.o: ../net/ip_output.c /usr/include/errno.h /usr/include/net/if.h
ip_output.o: /usr/include/net/route.h /usr/include/netinet/in.h
ip_output.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
ip_output.o: /usr/include/netinet/ip_var.h /usr/include/sys/mbuf.h
ip_output.o: /usr/include/sys/param.h /usr/include/sys/socket.h
ip_output.o: /usr/include/sys/socketvar.h
raw_cb.o: ../net/raw_cb.c /usr/include/errno.h /usr/include/net/if.h
raw_cb.o: /usr/include/net/raw_cb.h /usr/include/net/route.h
raw_cb.o: /usr/include/netinet/in.h /usr/include/sys/mbuf.h
raw_cb.o: /usr/include/sys/param.h /usr/include/sys/socket.h
raw_cb.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h
raw_ip.o: ../net/raw_ip.c /usr/include/errno.h /usr/include/net/if.h
raw_ip.o: /usr/include/net/raw_cb.h /usr/include/net/route.h
raw_ip.o: /usr/include/netinet/in.h /usr/include/netinet/in_systm.h
raw_ip.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
raw_ip.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
raw_ip.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
raw_ip.o: /usr/include/sys/socketvar.h
raw_usrreq.o: ../net/raw_usrreq.c /usr/include/errno.h /usr/include/net/if.h
raw_usrreq.o: /usr/include/net/netisr.h /usr/include/net/raw_cb.h
raw_usrreq.o: /usr/include/net/route.h /usr/include/netinet/in_systm.h
raw_usrreq.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
raw_usrreq.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
raw_usrreq.o: /usr/include/sys/socketvar.h
route.o: ../net/route.c /usr/include/errno.h /usr/include/net/af.h
route.o: /usr/include/net/if.h /usr/include/net/route.h
route.o: /usr/include/netinet/in_systm.h /usr/include/sys/dir.h
route.o: /usr/include/sys/ioctl.h /usr/include/sys/mbuf.h
route.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
route.o: /usr/include/sys/socket.h /usr/include/sys/systm.h
route.o: /usr/include/sys/user.h
tcp_debug.o: ../net/tcp_debug.c /usr/include/errno.h /usr/include/net/if.h
tcp_debug.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_debug.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_debug.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_debug.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_debug.h
tcp_debug.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_debug.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_debug.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_debug.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_debug.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_debug.o: /usr/include/sys/systm.h
tcp_input.o: ../net/tcp_input.c /usr/include/errno.h /usr/include/net/if.h
tcp_input.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_input.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_input.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_input.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_debug.h
tcp_input.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_input.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_input.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_input.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_input.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_input.o: /usr/include/sys/systm.h
tcp_output.o: ../net/tcp_output.c /usr/include/errno.h /usr/include/net/route.h
tcp_output.o: /usr/include/netinet/in.h /usr/include/netinet/in_pcb.h
tcp_output.o: /usr/include/netinet/in_systm.h /usr/include/netinet/ip.h
tcp_output.o: /usr/include/netinet/ip_var.h /usr/include/netinet/tcp.h
tcp_output.o: /usr/include/netinet/tcp_debug.h /usr/include/netinet/tcp_fsm.h
tcp_output.o: /usr/include/netinet/tcp_seq.h /usr/include/netinet/tcp_timer.h
tcp_output.o: /usr/include/netinet/tcp_var.h /usr/include/netinet/tcpip.h
tcp_output.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
tcp_output.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
tcp_output.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h
tcp_subr.o: ../net/tcp_subr.c /usr/include/errno.h /usr/include/net/if.h
tcp_subr.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_subr.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_subr.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_icmp.h
tcp_subr.o: /usr/include/netinet/ip_var.h /usr/include/netinet/tcp.h
tcp_subr.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_subr.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_subr.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_subr.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_subr.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_subr.o: /usr/include/sys/systm.h
tcp_timer.o: ../net/tcp_timer.c /usr/include/errno.h /usr/include/net/if.h
tcp_timer.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_timer.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_timer.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_timer.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_fsm.h
tcp_timer.o: /usr/include/netinet/tcp_seq.h /usr/include/netinet/tcp_timer.h
tcp_timer.o: /usr/include/netinet/tcp_var.h /usr/include/netinet/tcpip.h
tcp_timer.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
tcp_timer.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
tcp_timer.o: /usr/include/sys/socketvar.h /usr/include/sys/systm.h
tcp_usrreq.o: ../net/tcp_usrreq.c /usr/include/errno.h /usr/include/net/if.h
tcp_usrreq.o: /usr/include/net/route.h /usr/include/netinet/in.h
tcp_usrreq.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
tcp_usrreq.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_var.h
tcp_usrreq.o: /usr/include/netinet/tcp.h /usr/include/netinet/tcp_debug.h
tcp_usrreq.o: /usr/include/netinet/tcp_fsm.h /usr/include/netinet/tcp_seq.h
tcp_usrreq.o: /usr/include/netinet/tcp_timer.h /usr/include/netinet/tcp_var.h
tcp_usrreq.o: /usr/include/netinet/tcpip.h /usr/include/sys/mbuf.h
tcp_usrreq.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
tcp_usrreq.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
tcp_usrreq.o: /usr/include/sys/systm.h
udp_usrreq.o: ../net/udp_usrreq.c /usr/include/errno.h /usr/include/net/if.h
udp_usrreq.o: /usr/include/net/route.h /usr/include/netinet/in.h
udp_usrreq.o: /usr/include/netinet/in_pcb.h /usr/include/netinet/in_systm.h
udp_usrreq.o: /usr/include/netinet/ip.h /usr/include/netinet/ip_icmp.h
udp_usrreq.o: /usr/include/netinet/ip_var.h /usr/include/netinet/udp.h
udp_usrreq.o: /usr/include/netinet/udp_var.h /usr/include/sys/dir.h
udp_usrreq.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
udp_usrreq.o: /usr/include/sys/protosw.h /usr/include/sys/socket.h
udp_usrreq.o: /usr/include/sys/socketvar.h /usr/include/sys/user.h
