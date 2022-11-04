
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uux.c	3.0	4/22/86";

/*********************
 * UUX - Unix to Unix eXecute program 
 *********************/


/* Grade option "-g<g>" added. See cbosgd.2611 (Mark Horton) */
/* no-copy option "-c" added. Suggested by Steve Bellovin */
/* "-l" is synonym for "-c". */
/* "X" files use local system name, avoids conflict. Steve Bellovin */
/*	uux.c	1.1	81/12/31	*/
	/*  uux 3.7  1/11/80  14:16:55  */

/*****************
 * Mods:
 *	decvax!larry - handle subdirectories properly
 *		     - third party execute case,  bring data file
 *			to local system first, then send it to 
 *			execution site.
 *****************/

#include "uucp.h"


#define NOSYSPART 0
#define HASSYSPART 1

#define APPCMD(d) {\
char *p;\
for (p = d; *p != '\0';) *cmdp++ = *p++;\
*cmdp++ = ' ';\
*cmdp = '\0';}

#define GENSEND(f, a, b, c, d, e) {\
fprintf(f, "S %s %s %s -%s %s 0666\n", a, b, c, d, e);\
}
#define GENRCV(f, a, b, c) {\
fprintf(f, "R %s %s %s - \n", a, b, c);\
}


/*
 *	
 */

main(argc, argv)
char *argv[];
{
	char cfile[MAXFULLNAME]; /* send commands for files from here */
	char dfile[NAMESIZE];/* used for all data files from here */
	char rxfile[NAMESIZE];	/* to be sent to xqt file (X. ...) */
	char tfile[NAMESIZE];	/* temporary file name */
	char tcfile[NAMESIZE];	/* temporary file name */
	char t2file[NAMESIZE];	/* temporary file name */
	char rfile[MAXFULLNAME]; /* hold full path name of data file */
	int cflag = 0;		/*  commands in C. file flag  */
	int rflag = 0;		/*  C. files for receiving flag  */
	int Copy = 1;		/* Copy spool files */
	char buf[BUFSIZ];
	char inargs[BUFSIZ];
	int pipein = 0;
	int startjob = 1;
	char Grade = 'A';
	char path[MAXFULLNAME];
	char cmd[BUFSIZ];
	char *ap, *cmdp;
	char prm[BUFSIZ];
	char syspart[8], rest[MAXFULLNAME];
	char xsys[8], local[8];
	FILE *fprx, *fpc, *fpd, *fp, *xfp;
	extern char *getprm(), *lastpart();
	extern char *spoolname();
	extern FILE *ufopen();
	int uid, ret;
	char redir = '\0';
	int nonoti = 0;
	int nonzero = 0;
	int orig_uid = getuid();

	strcpy(Progname, "uux");
	uucpname(Myname);
	umask(WFMASK);
	Ofn = 1;
	Ifn = 0;
	while (argc>1 && argv[1][0] == '-') {
		switch(argv[1][1]){
		case 'p':
		case '\0':
			pipein = 1;
			break;
		case 'r':
			startjob = 0;
			break;
		case 'c':
		case 'l':
			Copy = 0;
			break;
		case 'g':
			Grade = argv[1][2];
			break;
		case 'x':
			chkdebug(orig_uid);
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		case 'n':
			nonoti = 1;
			break;
		case 'z':
			nonzero = 1;
			break;
		default:
			fprintf(stderr, "unknown flag %s\n", argv[1]);
				break;
		}
		--argc;  argv++;
	}

	DEBUG(4, "\n\n** %s **\n", "START");

	inargs[0] = '\0';
	for (argv++; argc > 1; argc--) {
		DEBUG(4, "arg - %s:", *argv);
		strcat(inargs, " ");
		strcat(inargs, *argv++);
	}
	DEBUG(4, "arg - %s\n", inargs);
	ret = gwd(Wrkdir);
	if (ret != 0) {
		fprintf(stderr, "can't get working directory; will try to continue\n");
		strcpy(Wrkdir, "/UNKNOWN");
	}
	uid = getuid();
	guinfo(uid, User, path);

	sprintf(local, "%.7s", Myname);

	/* find remote system name */
	ap = inargs;
	xsys[0] = '\0';
	while ((ap = getprm(ap, prm)) != NULL) {
		if (prm[0] == '>' || prm[0] == '<') {
			ap = getprm(ap, prm);
			continue;
		}


		split(prm, xsys, rest);
		break;
	}
	if (xsys[0] == '\0')
		strcpy(xsys, local);
	sprintf(Rmtname, "%.7s", xsys);
	DEBUG(4, "xsys %s\n", xsys);
	if (versys(xsys) != 0) {
		/*  bad system name  */
		fprintf(stderr, "bad system name: %s\n", xsys);
		cleanup(101);
	}

	mkspname(xsys);
	subchdir(Spool);


	cmdp = cmd;
	*cmdp = '\0';
	gename(DATAPRE, local, 'X', rxfile);
	fprx = ufopen(rxfile, "w");
	ASSERT(fprx != NULL, "CAN'T OPEN", rxfile, 0);
	gename(DATAPRE, local, 'T', tcfile);
	fpc = ufopen(tcfile, "w");
	ASSERT(fpc != NULL, "CAN'T OPEN", tcfile, 0);
	fprintf(fprx, "%c %s %s\n", X_USER, User, local);
	if (nonoti)
		fprintf(fprx,"%c\n", X_NONOTI);
	if (nonzero)
		fprintf(fprx,"%c\n", X_NONZERO);



	if (pipein) {
		gename(DATAPRE, local, 'b', dfile);
		fpd = ufopen(dfile, "w");
		ASSERT(fpd != NULL, "CAN'T OPEN", dfile, 0);
		while (!feof(stdin)) {
			ret = fread(buf, 1, BUFSIZ, stdin);
			fwrite(buf, 1, ret, fpd);
		}
		fclose(fpd);
		if (strcmp(local, xsys) != SAME) {
			GENSEND(fpc, dfile, dfile, User, "", dfile);
			cflag++;
		}
		fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
		fprintf(fprx, "%c %s\n", X_STDIN, dfile);
	}
	/* parse command */
	ap = inargs;
	while ((ap = getprm(ap, prm)) != NULL) {
		DEBUG(4, "prm - %s\n", prm);
		if (prm[0] == '>' || prm[0] == '<') {
			redir = prm[0];
			continue;
		}

		if (prm[0] == ';') {
			APPCMD(prm);
			continue;
		}

		if (prm[0] == '|' || prm[0] == '^') {
			if (cmdp != cmd)
				APPCMD(prm);
			continue;
		}

		/* process command or file or option */
		ret = split(prm, syspart, rest);
		DEBUG(4, "s - %s, ", syspart);
		DEBUG(4, "r - %s, ", rest);
		DEBUG(4, "ret - %d\n", ret);
		if (syspart[0] == '\0')
			strcpy(syspart, local);

		if (cmdp == cmd && redir == '\0') {
			/* command */
			APPCMD(rest);
			continue;
		}

		/* process file or option */
		DEBUG(4, "file s- %s, ", syspart);
		DEBUG(4, "local - %s\n", local);
		/* process file */
		if (redir == '>') {
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(2);
			fprintf(fprx, "%c %s %s\n", X_STDOUT, rest,
			 syspart);
			redir = '\0';
			continue;
		}

		if (ret == NOSYSPART && redir == '\0') {
			/* option */
			APPCMD(rest);
			continue;
		}

		if (strcmp(xsys, local) == SAME
		 && strcmp(xsys, syspart) == SAME) {

	/* all work done here */

			if (ckexpf(rest))
				cleanup(2);
			if (redir == '<')
				fprintf(fprx, "%c %s\n", X_STDIN, rest);
			else
				APPCMD(rest);
			redir = '\0';
			continue;
		}

		if (strcmp(syspart, local) == SAME) {
			/*  generate send file */

	/* data file is local , execute command at remote site */

			if (ckexpf(rest))
				cleanup(2);
			gename(DATAPRE, local, 'a', dfile);
			DEBUG(4, "rest %s\n", rest);
			if ((chkpth(User, "", rest) || anyread(rest)) != 0) {
				fprintf(stderr, "permission denied %s\n", rest);
				cleanup(1);
			}
			if (Copy) {
				if (xcp(rest, dfile) != 0) {
					fprintf(stderr, "can't copy %s to %s\n", rest, dfile);
					cleanup(1);
				}
				GENSEND(fpc, rest, dfile, User, "", dfile);
			}
			else {
				GENSEND(fpc, rest, dfile, User, "c", "D.0");
			}
			cflag++;
			if (redir == '<') {
				fprintf(fprx, "%c %s\n", X_STDIN, dfile);
				fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
			}
			else {
				APPCMD(lastpart(rest));
				fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, lastpart(rest));
			}
			redir = '\0';
			continue;
		}

		if (strcmp(local, xsys) == SAME) {

	/* file is at remote site, command is executed locally */

			/*  generate local receive  */
			gename(CMDPRE, syspart, 'R', tfile);
			strcpy(dfile, tfile);
			dfile[0] = DATAPRE;
			fp = ufopen(tfile, "w");
			ASSERT(fp != NULL, "CAN'T OPEN", tfile, 0);
			if (ckexpf(rest))
				cleanup(2);
			GENRCV(fp, rest, dfile, User);
			fclose(fp);
			rflag++;
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(2);
			if (redir == '<') {
				fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
				fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			}
			else {
				fprintf(fprx, "%c %s %s\n", X_RQDFILE, dfile,
				  lastpart(rest));
				APPCMD(lastpart(rest));
			}

			redir = '\0';
			continue;
		}

		if (strcmp(syspart, xsys) != SAME) {
			/* generate remote receives */

	/* command and data files are located on different remote systems */

			/*
			 * tfile contains uucico receive command line.
			 * the desired data file will be received first.
			 * A uucp command will be set up in a local X.file
			 * When the desired data file arrives this uucp
		 	 * command will ship it over to the execution
			 * site.
			 */

			/* syspart contains the site where the data  
			 * file is located.
			 * tfile contains the name of C.syspart file that
			 * will be used to receive the remote data file. 
			 * dfile contains the name that the received data
			 * file will have when it arrives.
			 * rfile is the full path of the latter file.
			 * t2file contains the name of the file at the
		  	 * the execution site.
			 */
			gename(CMDPRE, syspart, 'r', tfile);
			sprintf(cfile,"%s/C./%s",spoolname(syspart),tfile);
			fpd = ufopen(cfile, "w");
			ASSERT(fpd != NULL, "CAN'T OPEN", cfile, 0);
			gename(DATAPRE, local, 't', t2file);
			/*
			 * return D.file to subdirectory that belongs
		         * to syspart.
			 */
			gename(DATAPRE, syspart, 't', dfile);
			GENRCV(fpd, rest, dfile, User);
			fclose(fpd);
			sprintf(rfile,"%s/D./%s",spoolname(syspart),dfile);
			cflag++;

			/* generate an X.file to "uucp" dfile over to xsys */

			gename(XQTPRE, xsys, 'X', tfile);
			xfp = ufopen(tfile, "w");
			ASSERT(xfp != NULL, "CAN NOT OPEN", tfile, 0);
			fprintf(xfp, "%c %s %s\n", X_USER, User, local);
			fprintf(xfp, "%c %s %s\n", X_RQDFILE, rfile,
				  dfile);
			fprintf(xfp, "%c uucp -W  %s %s!%s\n", X_CMD, rfile,
				   xsys, t2file);
			fclose(xfp);

			if (redir == '<') {
				fprintf(fprx, "%c %s\n", X_RQDFILE, t2file);
				fprintf(fprx, "%c %s\n", X_STDIN, t2file);
			}
			else {
				fprintf(fprx, "%c %s %s\n", X_RQDFILE, t2file,
				  lastpart(rest));
				APPCMD(lastpart(rest));
			}
			redir = '\0';
			continue;
		}

		/* file on remote system */
		if (rest[0] != '~')
			if (ckexpf(rest))
				cleanup(2);
		if (redir == '<')
			fprintf(fprx, "%c %s\n", X_STDIN, rest);
		else
			APPCMD(rest);
		redir = '\0';
		continue;

	}

	fprintf(fprx, "%c %s\n", X_CMD, cmd);
	logent(cmd, "XQT QUE'D");
	fclose(fprx);

	strcpy(tfile, rxfile);
	tfile[0] = XQTPRE;
	if (strcmp(xsys, local) == SAME) {
		link(subfile(rxfile), subfile(tfile));
		unlink(subfile(rxfile));
		if (startjob)
			if (rflag)
				xuucico(xsys);
			else
				xuuxqt();
	}
	else {
		GENSEND(fpc, rxfile, tfile, User, "", rxfile);
		cflag++;
	}

	fclose(fpc);
	if (cflag) {
		gename(CMDPRE, xsys, Grade, cfile);
		link(subfile(tcfile), subfile(cfile));
		unlink(subfile(tcfile));
		if (startjob)
			xuucico(xsys);
		cleanup(0);
	}
	else
		unlink(subfile(tcfile));
}

#define FTABSIZE 30
char Fname[FTABSIZE][NAMESIZE];
int Fnamect = 0;

/***
 *	cleanup - cleanup and unlink if error
 *
 *	return - none - do exit()
 */

cleanup(code)
int code;
{
	int i;

	logcls();
	rmlock(CNULL);
	if (code) {
		for (i = 0; i < Fnamect; i++)
			unlink(subfile(Fname[i]));
		fprintf(stderr, "uux failed. code %d\n", code);
	}
	DEBUG(1, "exit code %d\n", code);
	exit(code);
}

/***
 *	ufopen - open file and record name
 *
 *	return file pointer.
 */

FILE *ufopen(file, mode)
char *file, *mode;
{
	if (Fnamect < FTABSIZE)
		strcpy(Fname[Fnamect++], file);
	else
		logent("Fname", "TABLE OVERFLOW");
	return(fopen(subfile(file), mode));
}
