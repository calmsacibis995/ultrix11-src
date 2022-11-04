
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ctl.h	3.0	(ULTRIX-11)	4/22/86
 */

/* ctl.h describes the structure that talk and talkd pass back
   and forth
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef pdp11
#define	print_response		pr_resp
#define	print_request		pr_req
#define	my_machine_name		My_name
#define	his_machine_name	His_name
#define	my_machine_addr		My_addr
#define	his_machine_addr	His_addr

#endif pdp11
#define NAME_SIZE 9
#define TTY_SIZE 16
#define HOST_NAME_LENGTH 256

#define MAX_LIFE 60 /* maximum time an invitation is saved by the
			 talk daemons */
#define RING_WAIT 30  /* time to wait before refreshing invitation 
			 should be 10's of seconds less than MAX_LIFE */

    /* the values for type */

#define LEAVE_INVITE 0
#define LOOK_UP 1
#define DELETE 2
#define ANNOUNCE 3

    /* the values for answer */

#define SUCCESS 0
#define NOT_HERE 1
#define FAILED 2
#define MACHINE_UNKNOWN 3
#define PERMISSION_DENIED 4
#define UNKNOWN_REQUEST 5

typedef struct ctl_response CTL_RESPONSE;

struct ctl_response {
    char type;
    char answer;
#ifdef	pdp11
    short dummy1;
#endif
    int id_num;
#ifdef	pdp11
    short dummy2;
#endif	pdp11
    struct sockaddr_in addr;
};

typedef struct ctl_msg CTL_MSG;

struct ctl_msg {
    char type;
    char l_name[NAME_SIZE];
    char r_name[NAME_SIZE];
    int id_num;
#ifdef	pdp11
    short dummy1;
#endif	pdp11
    int pid;
#ifdef	pdp11
    short dummy2;
#endif	pdp11
    char r_tty[TTY_SIZE];
    struct sockaddr_in addr;
    struct sockaddr_in ctl_addr;
};
