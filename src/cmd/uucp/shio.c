
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)shio.c	3.0	4/22/86";


/*******
 *	shio(cmd, fi, fo, user, fe)	execute shell of command with
 *	char *cmd, *fi, *fo, *fe;	fi, fo, fe as standard in/out/err
 *	char *user;		user name
 *
 *	return codes:
 *		0  - ok
 *		non zero -  failed  -  status from child
 */

/*******
 * Mods:
 * 	decvax!larry - touch LCK.Xfiles periodically 
 *		     - only allow MAXTOUCH touches before 
 *			aborting the command.
 *******/

#include "uucp.h"
#include <signal.h>

touch_alarm();
#define MAXTOUCH  9 /* the max number of times a LCK file should be touched */
int pid;

char *tcmd;
int touch_count;

shio(cmd, fi, fo, user, fe)
char *cmd, *fi, *fo, *user, *fe;
{
	int status, f;
	int uid, ret;
	char path[MAXFULLNAME];

	if (fi == NULL)
		fi = "/dev/null";
	if (fo == NULL)
		fo = "/dev/null";
	if (fe == NULL)
		fe = "/dev/null";
	tcmd = cmd;  /* so touch_alarm() knows the name of command */

	DEBUG(3, "shio - %s\n", cmd);
	if ((pid = fork()) == 0) {
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGKILL, SIG_IGN);
		signal(SIGALRM, SIG_IGN);
		signal(SIGPIPE, SIG_IGN);
		close(Ifn);
		close(Ofn);
		close(0);
		if (user == NULL
		|| (gninfo(user, &uid, path) != 0)
		|| setuid(uid))
			setuid(getuid());
		f = open(subfile(fi), 0);
		if (f != 0) {
			ASSERT_NOFAIL(f == 0, "CANT NOT OPEN INPUT - shio", 
				subfile(fi), 0);
			exit(f);
		}
		close(1);
		f = creat(subfile(fo), 0666);
		if (f != 1) {
			ASSERT_NOFAIL(f == 1, "CANT NOT OPEN OUPUT - shio", 
				subfile(fo), 0);
			exit(f);
		}
/* sendmail does not like this ? 
		close(2);
		f = creat(subfile(fe), 0666);
		if (f != 2) {
			ASSERT_NOFAIL(f == 2, "CANT NOT OPEN ERROR - shio", 
				subfile(fe), 0);
			exit(f);
		}
*/
		execl(SHELL, "sh", "-c", cmd, 0);
		exit(100);
	}
	signal(SIGALRM, touch_alarm);
	alarm(X_LOCKTIME-500); /* to refresh lock file */
	while ((ret = wait(&status)) != pid && ret != -1);
	touch_count = 0;
	DEBUG(3, "status %d\n", status);
	alarm(0);
	return(status);
}


touch_alarm()
 {
	if (++touch_count > MAXTOUCH) {
		logent(tcmd, "command terminated - exceeded time limit");
		touch_count = 0;
		kill(pid, SIGTERM);
		return;
	}
	ultouch();
	logent("touch lock file", "cmd xqt'ing > 55 minutes");
	alarm(X_LOCKTIME-500);
	return;
 }	
