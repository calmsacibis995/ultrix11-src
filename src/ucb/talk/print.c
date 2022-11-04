
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char *Sccsid = "@(#)print.c	3.0	(ULTRIX-11)	4/22/86";

/* debug print routines */

#include <stdio.h>
#include "ctl.h"
char *rt[] = {
	"LEAVE_INVITE",
	"LOOK_UP",
	"DELETE",
	"ANNOUNCE"
};
char *ans[] = {
	"SUCCESS",
	"NOT_HERE",
	"FAILED",
	"MACHINE_UNKNOWN",
	"PERMISSION_DENIED",
	"UNKNOWN_REQUEST"
};

print_request(request)
CTL_MSG *request;
{
    
    if (request->type < 4 && request->type >= 0)
	    printf("type is %s", rt[request->type]);
    else
	    printf("type is %d", request->type);
    printf(", l_user %s, r_user %s, r_tty %s\n",
	    request->l_name, request->r_name, request->r_tty);
    printf("        id = %d\n", request->id_num);
    fflush(stdout);
}

print_response(response)
CTL_RESPONSE *response;
{
    if (response->type < 4 && response->type >= 0)
	    printf("type is %s ,", rt[response->type]);
    else
	    printf("type is %d ,", response->type);
    if (response->answer <6 && response->answer >= 0)
	    printf("answer is %s, ", ans[response->answer]);
    else
	    printf("answer is %d, ", response->answer);
    printf("id = %d\n\n", response->id_num);
    fflush(stdout);
}
