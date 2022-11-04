
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char *Sccsid = "@(#)ctl_transact.c	3.0	(ULTRIX-11)	4/22/86";

#include "talk_ctl.h"
#ifdef vax
#include <sys/time.h>
#endif

#define CTL_WAIT 2	/* the amount of time to wait for a 
			   response, in seconds */

    /*
     * SOCKDGRAM is unreliable, so we must repeat messages if we have
     * not recieved an acknowledgement within a reasonable amount
     * of time
     */

ctl_transact(target, msg, type, response)
struct in_addr target;
CTL_MSG msg;
int type;
CTL_RESPONSE *response;
{
    struct sockaddr junk;
    long read_mask;
    long ctl_mask;
    int nready;
    int cc;
    int junk_size;
#ifdef vax
    struct timeval wait;
#else pdp11
    int wait = CTL_WAIT;
#endif

#ifdef vax
    wait.tv_sec = CTL_WAIT;
    wait.tv_usec = 0;
#endif vax

    msg.type = type;

    daemon_addr.sin_addr = target;
    daemon_addr.sin_port = daemon_port;

    ctl_mask = 1L << ctl_sockt;

	/*
	 * keep sending the message until a response of the right
	 * type is obtained
	 */
    do {
	    /* keep sending the message until a response is obtained */

	do {
	    cc = sendto(ctl_sockt, (char *)&msg, sizeof(CTL_MSG), 0,
			&daemon_addr, sizeof(daemon_addr));

	    if (cc != sizeof(CTL_MSG)) {
		if (errno == EINTR) {
			/* we are returning from an interupt */
		    continue;
		} else {
		    p_error("Error on write to talk daemon");
		}
	    }

	    read_mask = ctl_mask;

	    while ((nready = select(32, &read_mask, 0, 0, &wait)) < 0) {
		if (errno == EINTR) {
			/* we are returning from an interupt */
		    continue;
		} else {
		    p_error("Error on waiting for response from daemon");
		}
	    }
	} while (nready == 0);

	    /* keep reading while there are queued messages 
	       (this is not necessary, it just saves extra
	       request/acknowledgements being sent)
	     */

	do {

	    junk_size = sizeof(junk);
	    cc = recvfrom(ctl_sockt, (char *) response,
			   sizeof(CTL_RESPONSE), 0, &junk, &junk_size );
	    if (cc < 0) {
		if (errno == EINTR) {
		    continue;
		}
		p_error("Error on read from talk daemon");
	    }

	    read_mask = ctl_mask;

		/* an immediate poll */

#ifdef vax
	    timerclear(&wait);
#else pdp11
	    wait = 0;
#endif
	    nready = select(32, &read_mask, 0, 0, &wait);

	} while ( nready > 0 && response->type != type);

    } while (response->type != type);
}
