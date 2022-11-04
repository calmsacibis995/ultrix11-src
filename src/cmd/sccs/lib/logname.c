
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

# include	"pwd.h"
# include	"sys/types.h"
# include	"macros.h"

static char Sccsid[] = "@(#)logname.c 3.0 4/22/86";

char	*logname()
{
	struct passwd *getpwuid();
	struct passwd *log_name;
	int uid;

	uid = getuid();
	log_name = getpwuid(uid);
	endpwent();
	if (! log_name)
		return((char *)log_name);
	else
		return(log_name->pw_name);
}
