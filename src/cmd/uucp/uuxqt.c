
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uuxqt.c	3.0	4/22/86";

/*
 *	uuxqt will execute commands set up by a uux command,
 *	usually from a remote machine - set by uucp.
 */

/*****************
 * Mods:
 *	decvax!larry - handle subdirectories properly.
 *		     - add concurrency on command type basis via the 
 *			-c option
 *		     - create lock files for each command to prevent
 *			other uuxqt from executing the same command.
 *		     - handle system V.? return address
 *****************/

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef NDIR
#include "ndir.h"
#else
#include "sys/dir.h"
#endif


#define APPCMD(d) {\
char *p;\
for (p = d; *p != '\0';) *cmdp++ = *p++;\
*cmdp++ = ' ';\
*cmdp = '\0';}


#define	NCMDS	50
struct command {
	char *cmd;
	int xeqlevel;
} Cmds[NCMDS];

int notiok = 1;
int nonzero = 0;
int getpid();
int getepid();
int Euid;   /* effective user id */

char PATH[MAXFULLNAME] = "PATH=/bin:/usr/bin";
/*  to remove restrictions from uuxqt
 *  define ALLOK 1
 *
 *  to add allowable commands, add to the file CMDFILE
 *  A line of form "PATH=..." changes the search path
 */


main(argc, argv)
char *argv[];
{
	char xcmd[MAXFULLNAME];
	int argnok;
	char xfile[MAXFULLNAME], user[MAXFULLNAME], buf[BUFSIZ];
	char lbuf[MAXFULLNAME];
	char cfile[MAXFULLNAME], dfile[MAXFULLNAME];
	char file[MAXFULLNAME];
	char fin[MAXFULLNAME], sysout[NAMESIZE], fout[MAXFULLNAME];
	char ferr[MAXFULLNAME];
	char xsys[NAMESIZE];
	FILE *xfp, *dfp, *fp;
	char path[MAXFULLNAME];
	char cmdtype[MAXFULLNAME];
	char lock[MAXFULLNAME];
	char xlock[MAXFULLNAME];
	char retaddr[MAXFULLNAME];
	int ret_input;
	struct stat stbuf;
	char cmd[BUFSIZ];
	/* set size of prm to something large -- cmcl2!salkind */
	char *cmdp, prm[1000], *ptr;
	char *getprm(), *lastpart();
	int uid, ret, badfiles, i;
	int stcico = 0;
	char retstat[30];
	int orig_uid = getuid();
	char *argp[2];
	int narg = 0;

	strcpy(Progname, "uuxqt");
	uucpname(Myname);

	/* Try to run as uucp -- rti!trt */
	setgid(getegid());
	setuid(geteuid());
	Euid = geteuid();

	umask(WFMASK);
	Ofn = 1;
	Ifn = 0;
	cmdtype[0]='\0';
	while (argc>1 && argv[1][0] == '-') {
		switch(argv[1][1]){
		case 'x':
			chkdebug(orig_uid);
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		case 'c':
			sprintf(cmdtype, "%s", &argv[1][2]);
			break;
		default:
			fprintf(stderr, "unknown flag %s\n", argv[1]);
				break;
		}
		--argc;  argv++;
	}

	DEBUG(4, "\n\n** %s **\n", "START");
	uid = getuid();
	guinfo(uid, User, path);
	DEBUG(4, "User - %s\n", User);
	sprintf(lock,"%s%s",X_LOCK,cmdtype);
	DEBUG(5, "the lock is :%s:\n",lock);
	if (ulockf(lock, (time_t)  X_LOCKTIME) != 0)
		exit(0);

	fp = fopen(CMDFILE, "r");
	if (fp == NULL) {
		/* Fall-back if CMDFILE missing. Sept 1982, rti!trt */
		logent("CAN'T OPEN", CMDFILE);
		Cmds[0].cmd = "rmail";
		Cmds[0].xeqlevel = 1;
		Cmds[1].cmd = "rnews";
		Cmds[1].xeqlevel = 1;
		Cmds[2].cmd = NULL;
		goto doprocess;
	}
	DEBUG(5, "%s opened\n", CMDFILE);
	for (i=0; i<NCMDS-1 && cfgets(cmd, sizeof(cmd), fp) != NULL; i++) {
		/* get command */
		narg = getargs(cmd, argp);
		if (strncmp(argp[0], "PATH=", 5) == 0) {
			/* change path in L.cmd file */
			strcpy(PATH, argp[0]);
			i--;
			continue;
		}
		Cmds[i].cmd = malloc((unsigned)(strlen(argp[0])+1));
		strcpy(Cmds[i].cmd, argp[0]);
		DEBUG(5, "cmd = %s\n", Cmds[i].cmd);
		if (narg < 2  || argp[1][0] != 'X') {
			DEBUG(5, "default xeq level provided %d\n", 9);
			Cmds[i].xeqlevel = 9;
		}		
		else  
			Cmds[i].xeqlevel = atoi(&argp[1][1]);
		DEBUG(4, "xeq level for command is %d\n", Cmds[i].xeqlevel);
	}
	Cmds[i].cmd = 0;
	fclose(fp);

	/* make temporary file for standard error output */
	sprintf(ferr,"%s/LTMP.ERR.%d", SPOOL, getpid);

	xlock[0] = '\0';
nextsys:  /* switch to another per system spool directory */
	while(gnsys(xsys, XQTPRE)) {
	  DEBUG(5, "nextsys: %s\n", xsys);
	  mkspname(xsys);
	  DEBUG(5, "spooldir: %s\n", Spool);
	  subchdir(Spool);
	  strcpy(Wrkdir, Spool);

doprocess:  /* process work in the spool directory */
	  DEBUG(4, "process %s\n", "");
	  while (gtxfile(xfile) > 0) {
		if (xlock[0] != '\0') { /* remove lock for previous X.file */
			rmlock(xlock);
			xlock[0] = '\0';
		}
		ultouch();	/* rti!trt */
		DEBUG(4, "xfile - %s\n", xfile);

		xfp = fopen(subfile(xfile), "r");
		ASSERT(xfp != NULL, "CAN'T OPEN", xfile, 0);

		/*  initialize to default  */
		strcpy(user, User);
		strcpy(fin, "/dev/null");
		strcpy(fout, "/dev/null");
		sprintf(sysout, "%.7s", Myname);
		retaddr[0] = '\0';
		badfiles = 0;	/* this was missing -- rti!trt */
		notiok = 1;
		nonzero = 0;
		ret_input = 0;
		while (fgets(buf, BUFSIZ, xfp) != NULL) {
			switch (buf[0]) {
			case X_USER:
				sscanf(&buf[1], "%s%s", user, Rmtname);
				break;
			case X_STDIN:
				sscanf(&buf[1], "%s", fin);
				i = expfile(fin);
				/* rti!trt: do not check permissions of
				 * vanilla spool file */
				if (i != 0
				 && (chkpth("", "", fin) || anyread(fin) != 0))
					badfiles = 1;
				break;
			case X_STDOUT:
				sscanf(&buf[1], "%s%s", fout, sysout);
				sysout[7] = '\0';
				/* rti!trt: do not check permissions of
				 * vanilla spool file.  DO check permissions
				 * of writing on a non-vanilla file */
				i = 1;
				if (fout[0] != '~' || prefix(sysout, Myname))
					i = expfile(fout);
				if (i != 0
				 && (chkpth("", "", fout)
					|| chkperm(fout, (char *)1)))
					badfiles = 1;
				break;
			case X_CMD:
				strcpy(cmd, &buf[2]);
				if (*(cmd + strlen(cmd) - 1) == '\n')
					*(cmd + strlen(cmd) - 1) = '\0';
				break;
			case X_NONOTI:
				notiok = 0;
				break;
			case X_NONZERO:
				nonzero = 1;
				break;
			case X_INPUT:     /* not used yet */
				ret_input = 1;
				break;
			case X_RETURNADDR:
				sscanf(&buf[1], "%s", retaddr);
				break;
			default:
				break;
			}
		}

		fclose(xfp);
		DEBUG(4, "fin - %s, ", fin);
		DEBUG(4, "fout - %s, ", fout);
		DEBUG(4, "sysout - %s, ", sysout);
		DEBUG(4, "user - %s\n", user);
		DEBUG(4, "cmd - %s\n", cmd);

		/*  command execution  */
		if (strcmp(fout, "/dev/null") == SAME)
			strcpy(dfile,"/dev/null");
		else
			gename(DATAPRE, sysout, 'O', dfile);

		/* expand file names where necessary */
		expfile(dfile);
		strcpy(buf, PATH);
		strcat(buf, ";export PATH;");
		cmdp = buf + strlen(buf);
		ptr = cmd;
		xcmd[0] = '\0';
		argnok = 0;
		while ((ptr = getprm(ptr, prm)) != NULL) {

			if (prm[0] == ';' || prm[0] == '^'
			  || prm[0] == '&'  || prm[0] == '|') {
				xcmd[0] = '\0';
				APPCMD(prm);
				continue;
			}
			if ((argnok = argok(xcmd, prm)) != 0) 
				/*  command not valid  */
				break;

			/* if LCK.XQTxcmd exists then another
			 * uuxqt daemon is running specifically
			 * for the command specified by xcmd
			 */

			/* a null cmdtype implies a general uuxqt */
			if (cmdtype[0] == '\0') {
				sprintf(lock,"%s%s",X_LOCK,xcmd);
				if (stat(lock,&stbuf) != -1) {
					DEBUG(3,"uuxqt running for command: %s\n", xcmd);
					goto doprocess;
				}
			}
			else   /* only xqt commands of type cmdtype */
				if (strncmp(cmdtype,xcmd,strlen(cmdtype))) {
					DEBUG(3,"xcmd=%s not the desired type\n",xcmd);
					goto doprocess;
				}
			

			if (prm[0] == '~')
				expfile(prm);
			APPCMD(prm);
		}

		/* prevent concurrent uuxqts from working on the same
		 * X.file.  This can only happen when a command specific
		 * uuxqt starts up after a general uuxqt.
		 * This locking mechanism should permit concurrent general
		 * uuxqts.
		 */
		sprintf(xlock,"%s/LCK.%s",SPOOL,xfile);
		if (ulockf(xlock, (time_t)  X_LOCKTIME) != 0) {
			/* dont remove another daemons lock file */
			xlock[0] = '\0';
			DEBUG(8,"this xfile already processed: %s\n",
				xfile);
			goto doprocess;
		}
			
		if (argnok || badfiles) {
			sprintf(lbuf, "%s XQT DENIED", user);
			logent(cmd, lbuf);
			DEBUG(4, "bad command %s\n", prm);
			notify(user, Rmtname, cmd, "DENIED", ferr, retaddr);
			goto rmfiles;
		}
		sprintf(lbuf, "%s XQT", user);
		logent(buf, lbuf);
		DEBUG(4, "cmd %s\n", buf);

		mvxfiles(xfile);
		subchdir(XQTDIR);
		
		ret = shio(buf, fin, dfile, (char *)NULL, ferr);
/* watcgl.11, dmmartindale, signal and exit values were reversed */
		sprintf(retstat, "signal %d, exit %d", ret & 0377,
		  (ret>>8) & 0377);
		DEBUG(5,"retstat is %s: \n", retstat);
		/* don't return exit status for mail commands */
		if (strcmp(xcmd, "rmail") == SAME)
			notiok = 0;
		/* only return nonzero (fail) exit status for news commands */
		if (strcmp(xcmd, "rnews") == SAME)
			nonzero = 1;

		/* notify user of exit status if:
		 * 1.  ok to notify user  && ok to return any exit status
		 * 2.  ok to notify user  && exit status was non zero
		 * 3.  mail command failed - return mail to sender
 		 */

		if (notiok && (!nonzero || (nonzero && ret != 0)))
			notify(user, Rmtname, cmd, retstat, ferr, retaddr);
		else if (ret != 0 && strcmp(xcmd, "rmail") == SAME) {
			/* mail failed - return letter to sender  */
			retosndr(user, Rmtname, fin, ferr, retaddr);
			sprintf(buf, "ret (%o) from %s!%s", ret, Rmtname, user);
			logent("MAIL FAIL", buf);
		}
		DEBUG(4, "exit cmd - %d\n", ret);
		subchdir(Spool);
		rmxfiles(xfile);
		if (ret != 0) {
			/*  exit status not zero */
			dfp = fopen(subfile(dfile), "a");
			ASSERT(dfp != NULL, "CAN'T OPEN", dfile, 0);
			fprintf(dfp, "exit status %d", ret);
			fclose(dfp);
			sprintf(buf, "cmd: %s; ret: %s", xcmd, retstat);
			logent("CMD FAILED", buf);
		}
		if (strcmp(fout, "/dev/null") != SAME) {
			if (prefix(sysout, Myname)) {
				xmv(dfile, fout);
				chmod(fout, BASEMODE);
			}
			else {
				gename(CMDPRE, sysout, 'O', cfile);
				fp = fopen(subfile(cfile), "w");
				ASSERT(fp != NULL, "OPEN", cfile, 0);
				fprintf(fp, "S %s %s %s - %s 0666\n",
				dfile, fout, user, lastpart(dfile));
				fclose(fp);
			}
		}
	rmfiles:
		xfp = fopen(subfile(xfile), "r");

		ASSERT_NOFAIL(xfp != NULL, "CAN NOT OPEN", xfile, 0);
		/* on the chance that another uuxqt was working
		 * on this file and removed it, keep going
		 * - with the addition of per command uuxqts
		 * it is possible for overlap when another daemon
		 * starts up.
		 */
		if (xfp == NULL)  
			continue;
		while (fgets(buf, BUFSIZ, xfp) != NULL) {
			if (buf[0] != X_RQDFILE)
				continue;
			sscanf(&buf[1], "%s", file);
			unlink(subfile(file));
		}
		unlink(subfile(xfile));
		fclose(xfp);
		unlink(subfile(ferr));
	  } /* end doprocess loop */
	} /* end nextsys loop */

	if (stcico)
		xuucico("");
	cleanup(0);
}


cleanup(code)
int code;
{
	logcls();
	rmlock(CNULL);
	exit(code);
}


/*******
 *	gtxfile(file)	get a file to execute
 *	char *file;
 *
 *	return codes:  0 - no file  |  1 - file to execute
 * Mod to recheck for X-able files. Sept 1982, rti!trt.
 * Should use stuff like bldflst to keep files in sequence
 * Suggested by utzoo.2458 (utzoo!henry)
 */


gtxfile(file)
char *file;
{
	static int reopened;
	static DIR *dirp;
	char pre[3];

retry:
	if (dirp == NULL) {
		dirp = opendir(subdir(Spool, XQTPRE));
		ASSERT(dirp != NULL, "GTXFILE CAN'T OPEN", Spool, 0);
	}

	pre[0] = XQTPRE;
	pre[1] = '.';
	pre[2] = '\0';
	while (gnamef(dirp, file) != 0) {
		DEBUG(4, "file - %s\n", file);
		if (!prefix(pre, file))
			continue;
#ifndef	UUDIR
		/* Skip spurious subdirectories */
		if (strcmp(pre, file) == SAME)
			continue;
#endif
		if (gotfiles(file))
			/*  return file to execute */
			return(1);
	}

	closedir(dirp);
	dirp = NULL;
	if (!reopened) {
		reopened++;
		goto retry;
	}
	return(0);
}


/***
 *	gotfiles(file)		check for needed files
 *	char *file;
 *
 *	return codes:  0 - not ready  |  1 - all files ready
 */

gotfiles(file)
char *file;
{
	struct stat stbuf;
	FILE *fp;
	char buf[BUFSIZ], rqfile[MAXFULLNAME];

	fp = fopen(subfile(file), "r");
	if (fp == NULL)
		return(0);

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		DEBUG(4, "%s\n", buf);
		if (buf[0] != X_RQDFILE)
			continue;
		sscanf(&buf[1], "%s", rqfile);
		expfile(rqfile);
		if (stat(subfile(rqfile), &stbuf) == -1) {
			fclose(fp);
			return(0);
		}
	}

	fclose(fp);
	return(1);
}


/***
 *	rmxfiles(xfile)		remove execute files to x-directory
 *	char *xfile;
 *
 *	return codes - none
 */

rmxfiles(xfile)
char *xfile;
{
	FILE *fp;
	char buf[BUFSIZ], file[NAMESIZE], tfile[NAMESIZE];
	char tfull[MAXFULLNAME];

	if((fp = fopen(subfile(xfile), "r")) == NULL)
		return;

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", file, tfile) < 2)
			continue;
		sprintf(tfull, "%s/%s", XQTDIR, tfile);
		unlink(subfile(tfull));
	}
	fclose(fp);
	return;
}


/***
 *	mvxfiles(xfile)		move execute files to x-directory
 *	char *xfile;
 *
 *	return codes - none
 */

mvxfiles(xfile)
char *xfile;
{
	FILE *fp;
	char buf[BUFSIZ], ffile[MAXFULLNAME], tfile[NAMESIZE];
	char tfull[MAXFULLNAME];
	int ret;

	if((fp = fopen(subfile(xfile), "r")) == NULL)
		return;

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", ffile, tfile) < 2)
			continue;
		expfile(ffile);
		sprintf(tfull, "%s/%s", XQTDIR, tfile);
		unlink(subfile(tfull));
		ret = link(subfile(ffile), subfile(tfull));
		ASSERT(ret == 0, "LINK ERROR", "", ret);
/*
		unlink(subfile(ffile));
*/
	}
	fclose(fp);
	return;
}


/***
 *	argok(xc, cmd)		check for valid command/argumanet
 *			*NOTE - side effect is to set xc to the
 *				command to be executed.
 *	char *xc, *cmd;
 *
 *	return 0 - ok | 1 nok
 */

argok(xc, cmd)
char *xc, *cmd;
{
	struct command *ptr;

#ifndef ALLOK
	/* don't allow sh command strings `....` */
	/* don't allow redirection of standard in or out  */
	/* don't allow other funny stuff */
	/* but there are probably total holes here */
	/* post-script.  ittvax!swatt has a uuxqt that solves this. */
	/* This version of uuxqt will shortly disappear */
	if (index(cmd, '`') != NULL
	  || index(cmd, '>') != NULL
	  || index(cmd, ';') != NULL
	  || index(cmd, '^') != NULL
	  || index(cmd, '&') != NULL
	  || index(cmd, '|') != NULL
	  || index(cmd, '<') != NULL)
		return(1);
#endif

	if (xc[0] != '\0')
		return(0);

#ifndef ALLOK
	ptr = Cmds;
	while(ptr->cmd != NULL) {
		if (strcmp(cmd, ptr->cmd) == SAME)
			break;
		ptr++;
	}
	if (ptr->cmd == NULL) {
		DEBUG(4, "command not found: %s\n", cmd);
		return(1);
	}
/*
 *	If the USERFILE execution level for the remote system is less 
 *	than the execution level specified in L.cmds then do not
 *	allow remote execution to proceed.
 */
	if (getxeqlevel(Rmtname) < ptr->xeqlevel) {
		DEBUG(4, "Remote site (%s) can not execute command\n", Rmtname);
		logent("execute level too low", Rmtname);
		return(1);
	}
#endif
	strcpy(xc, cmd);
	return(0);
}


/***
 *	notify	send mail to user giving execution results
 *	return code - none
 *	This program assumes new mail command - send remote mail
 */

notify(user, rmt, cmd, str, ferr, raddr)
char *user, *rmt, *cmd, *str, *ferr, *raddr;
{
	char text[MAXFULLNAME];
	char ruser[MAXFULLNAME];

	sprintf(text, "uuxqt cmd (%.50s) status (%s)", cmd, str);
	if (prefix(rmt, Myname))
		strcpy(ruser, user);
	else
		sprintf(ruser, "%s!%s", rmt, user);
	/* use return address if supplied */
	if (raddr[0] != '\0')
		strcpy(ruser, raddr);
	mailst(ruser, text, "", ferr);
	return;
}

/***
 *	retosndr - return mail to sender
 *
 *	return code - none
 */

retosndr(user, rmt, file, ferr, raddr)
char *user, *rmt, *file, *ferr, *raddr;
{
	struct stat stbuf;
	char ruser[100];
	int ret;

	if (strcmp(rmt, Myname) == SAME)
		strcpy(ruser, user);
	else
		sprintf(ruser, "%s!%s", rmt, user);

	/* use return address if supplied */
	if (raddr[0] != '\0')
		strcpy(ruser, raddr);
	ret = stat(subfile(file), &stbuf);
	if ((anyread(file) == 0) || (ret==0 && stbuf.st_uid == Euid)) 
		mailst(ruser, "Mail failed.  Letter returned to sender.\n", 
				file, ferr);
	else
		mailst(ruser, "Mail failed.  uuxqt can not read the letter \n", 
				"", ferr);
	return;
}
