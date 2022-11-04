
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uustat.c	3.0	4/22/86";
 
/*************
 *	uustat --- A command that provides uucp status.
 */

/**************
 * Mods:
 *	decvax!larry - handle subdirectories properly
 *		     - handles data in binary format
 *	    3/8/84   - use STST. files for system status
 **************/

#include "uucp.h"
#ifdef UUSTAT
#include <time.h>
#include <sys/types.h>
#include "uust.h"
#include <errno.h>
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif
#define	rid	0		/* user id for super user */
#define L_DUMMY		"/usr/lib/uucp/dummy"
#define S_DUMMY		"/usr/spool/uucp/dummy"
 
/***	system status text	***/
char *us_stext[] = {
	"CONVERSATION SUCCEEDED",
	"BAD SYSTEM",
	"WRONG TIME TO CALL",
	"SYSTEM LOCKED",
	"NO DEVICE AVAILABLE",
	"DIAL FAILED",
	"LOGIN FAILED",
	"HANDSHAKE FAILED",
	"STARTUP FAILED",
	"CONVERSATION IN PROGRESS",
	"CONVERSATION FAILED",
	"CALL SUCCEEDED",
	"CONNECT FAILED"
	};
/***	request status text	***/
char *us_rtext[] = {
	"STATUS UNKNOWN: SYSTEM ERROR",
	"COPY FAIL",
	"LOCAL ACCESS TO FILE DENIED",
	"REMOTE ACCESS TO FILE DENIED",
	"A BAD UUCP COMMAND GENERATED",
	"REMOTE CAN'T CREATE TEMP FILE",
	"CAN'T COPY TO REMOTE DIRECTORY",
	"CAN'T COPY TO LOCAL DIRECTORY - FILE LEFT IN PUBDIR/USER/FILE",
	"LOCAL CAN'T CREATE TEMP FILE",
	"CAN'T EXECUTE UUCP",
	"COPY (PARTIALLY) SUCCEEDED",
	"COPY FINISHED, JOB DELETED",
	"JOB IS QUEUED"
	};
 
short vflag = 0;
short mflag;
int errno;
 
 
main(argc, argv)
char **argv;
int argc;
{
	extern char *optarg;
	extern int optind;
	struct us_rsf u;
	struct us_ssf ss;
	short cflag,jflag,sflag,kflag,uflag,oflag,yflag,iflag;
	int c;
	short ca, oa, ya;
	char sa[NAME7], ua[NAME7], ma[NAME7];
	char s[128], buf[BUFSIZ], ka[5], ja[5];
	FILE *fp, *fq, *us_open();
	int orig_uid = getuid();
	FILE *sfd;
	DIR *ufd;
	struct direct *dirp;
	uucpname(s);  /* init subdir stuff */
 
	cflag=jflag=sflag=kflag=uflag=oflag=yflag=mflag=iflag=vflag=0;
	while ((c=getopt(argc, argv, "x:c:j:s:k:u:o:y:m:v")) != EOF ) 
		switch(c) {
		case 'x':
			chkdebug(orig_uid);
			Debug = atoi(optarg);
			break;
		case 'c':
			if (mflag || iflag || kflag || jflag) goto error;
			cflag++;
			ca = atoi(optarg);
			break;
		case 'j':
			if (mflag || iflag || kflag || cflag) goto error;
			jflag++;
			strncpy(ja, optarg, 4);
			ja[4] = '\0';
			break;
		case 'm':
			if (jflag || iflag || kflag || cflag) goto error;
			mflag++;
			strncpy(ma, optarg, NAME7);
			ma[NAME7-1] = '\0';
			break;
		case 'k':
			if (jflag || mflag || iflag || cflag) goto error;
			kflag++;
			strncpy(ka, optarg, 4);
			ka[4] = '\0';
			break;
		case 'u':
			if (jflag || mflag || kflag || cflag) goto error;
			iflag++;
			uflag++;
			strncpy(ua, optarg, NAME7);
			ua[NAME7-1] = '\0';
			break;
		case 'o':
			if (jflag || mflag || kflag || cflag) goto error;
			iflag++;
			oflag++;
			oa = atoi(optarg);
			break;
		case 'y':
			if (jflag || mflag || kflag || cflag) goto error;
			iflag++;
			yflag++;
			ya = atoi(optarg);
			break;
		case 's':
			if (jflag || mflag || kflag || cflag) goto error;
			iflag++;
			sflag++;
			strncpy(sa, optarg, NAME7);
			sa[NAME7-1] = '\0';
			break;
		case 'v':
			vflag++;
			break;
		case '?':
			error:
			 fprintf(stderr, "Usage: uustat [-j* -v]");
			 fprintf(stderr, " [-m*] [-k*] [-c*] [-v]\n");
			 fprintf(stderr, "\t\t[-u* -s* -o* -y* -v]\n");
			 exit(2);
		}
 
	subchdir(Spool);
	guinfo(getuid(), User, s);	/* User: the current user name */
	if (cflag) {	/* remove entries in R_stat older than ca hours */
			/* used only by "uucp" or "root" */
 
		if ((strcmp(User,"uucp")!=SAME)&&(getuid()!=rid)) {
			fprintf(stderr,"Only uucp or root is allowed ");
			fprintf(stderr,"to use '-c' option\n");
			exit(1);
		}
		DEBUG(5, "enter clean mode, ca: %d\n", ca);
		fp=us_open(R_stat, "r", L_DUMMY, 1, 1);
		if (fp==NULL) exit(FAIL);
		sprintf(s, "%s/%s.%.7d",Spool,"rstat",getpid());
		DEBUG(5, "temp file: %s\n", s);
		fq=us_open(s, "w+", S_DUMMY, 1, 1);
		if (fq==NULL) exit(FAIL);
		while(fread(&u, sizeof(u), 1, fp) != NULL){
			if (older(u.stime, ca))	break;
			else	fwrite(&u, sizeof(u), 1 , fq);
		}
		fclose(fp);
		fclose(fq);
		if (xmv(s, R_stat) == FAIL)
			fprintf(stderr, "mv fails in uustat: %s\n", "-c");
		unlink(S_DUMMY);
		exit(0);
	}
 
	if (mflag) {		/* print machine status */
		ufd = opendir("/usr/spool/uucp/STST.","r");
	    	ASSERT(ufd!=NULL,"can not open SPOOL/STST. ","", errno);
		chdir("/usr/spool/uucp/STST.");
		if (strcmp(ma,"all") == SAME) { 
			while( (dirp=readdir(ufd))!=NULL) {
		   	   if( strncmp("STST.",dirp->d_name,5) == 0 )  
				if( (sfd=fopen(dirp->d_name,"r")) != NULL ) {
					sout(sfd, dirp->d_name+5);
					fclose(sfd);
				}
			}
			closedir(ufd);
			exit(0);
		}
		while( (dirp=readdir(ufd))!=NULL) 
		   if( strncmp(dirp->d_name+5, ma, strlen(ma)) == 0 )  
			if( (sfd=fopen(dirp->d_name,"r")) != NULL ) {
				sout(sfd, dirp->d_name+5);
				fclose(sfd);
				closedir(ufd);
				exit(0);
			}
		fprintf(stderr, "system %s or its status unknown\n", ma);
		exit(1);
	}
 
	/*
	 * Kill the job 'ka' and remove the Command file
	 * from spool directory. However, the D. files
	 * are not removed.
	 * Remove the entry from R_stat file and compact
	 * the hole. It is assumed that '-k' option
	 * will only be used occasionally.
	 */
 
	if (kflag) {
		DIR *pdirf;	/* for spool directory */
		char *name, file[100];
		int n;
		int jobfound = 0;
 
		DEBUG(5, "enter kill loop, ka: %s\n", ka);
		fp=us_open(R_stat, "r", L_DUMMY, 1, 1);
		if (fp==NULL) exit(FAIL);
		sprintf(s, "%s/%s.%.7d",Spool,"rstat",getpid());
		DEBUG(5, "temp file: %s\n", s);
		fq=us_open(s, "w+", S_DUMMY, 1, 1);
		if (fp==NULL) exit(FAIL);
		while(fread(&u, sizeof(u), 1, fp) != NULL){
			u.jobn[4] = '\0';
			DEBUG(7,"u.jobn to kill = %s\n", u.jobn);
			if ((strncmp(u.jobn,ka,4)==SAME) &&
			((strcmp(u.user,User)==SAME) || (getuid()==rid))) {
				DEBUG(5, "Job %s is deleted\n", ka);
				jobfound++;
			/* delete the command file from spool dir. */
				mkspname(u.rmt);
				subchdir(Spool);
				if ((pdirf=opendir(subdir(Spool,CMDPRE),"r"))==NULL) {
					perror(subdir(Spool,CMDPRE));
					unlink(S_DUMMY);
					exit(FAIL);
				}
				/* get next file name from directory 'pdirf' */
				while (gnamef(pdirf,file)) {
					if (file[0] != CMDPRE) continue;
					name = file + strlen(file) - 4;
					if (strncmp(name, ka, 4) == SAME) {
						DEBUG(4, "file unlinked: %s\n",
							subfile(file));
						unlink(subfile(file));
						closedir(pdirf);
						goto k1cont;
					}
				}
				closedir(pdirf);
			}
			else	fwrite(&u, sizeof(u), 1 , fq);
		}
	k1cont:	/* copy the rest of R_stat file */
		if (jobfound) 
			while(fread(&u, sizeof(u), 1, fp) != NULL)
				fwrite(&u, sizeof(u), 1 , fq);
		else 
			printf("Job not found: %s\n", ka);
		fclose(fp);
		fclose(fq);
		chdir(Spool);
		if (xmv(s, R_stat) == FAIL)
			fprintf(stderr, "mv fails in uustat: %s\n", "-k");
		unlink(S_DUMMY);
		exit(0);
	}
 
	fp=us_open(R_stat, "r", L_DUMMY, 1, 1);
	if (fp == NULL) exit(FAIL);
	while(fread(&u, sizeof(u), 1, fp) != NULL){
		DEBUG(5, "user: %s ", u.user);
		DEBUG(5, " User: %s\n", User);
		if (jflag) {	/* print request status of job# 'ja' */
			if ((strcmp(ja,"all")==SAME)
				|| (strcmp(ja,u.jobn)==SAME)) jout(&u);
		}

		else if (iflag) {
			if ((!uflag || (strcmp(u.user,ua) == SAME )) &&
			    (!sflag || (strcmp(u.rmt, sa) == SAME )) &&
			    (!oflag || older(u.qtime, oa)) &&
			    (!yflag || !older(u.qtime, ya)) )	jout(&u);
		}
 
		/* no option is given to "uustat" command,
		   print the status of all jobs isuued by
		   the current user	*/
		else if (strcmp(u.user, User) == SAME)
			jout(&u);
	}
}
 
sout(sfd, name)		/* print a record of us_ssf in L_stat file */
FILE *sfd;
char *name;
{
	int type, count; 
	time_t lasttime, retrytime, successtime;
	char stst[132];
	char *rname;
	struct tm *tp, *localtime();
 
	fscanf(sfd,"%d %d %ld %ld %ld", &type, &count, 
			&lasttime, &retrytime, &successtime);
	if( fgets(stst,132,sfd) != NULL ) {
		/* remove remote name from STST file */
		rname = stst + strlen(stst);
		while( *rname != ' ' )
			rname--; 
		*rname = '\0';
	}
	tp = localtime(&lasttime);
	printf("%.7s\t%02d/%02d-%02d:%02d",name,tp->tm_mon+1,
		tp->tm_mday, tp->tm_hour, tp->tm_min);
	if (mflag && successtime){ 
		tp = localtime(&successtime);
		printf("\t%02d/%02d-%02d:%02d",tp->tm_mon+1,
			tp->tm_mday, tp->tm_hour, tp->tm_min);
	}
	else printf("\t\t");
	printf("\t%s\n",stst);
	return(0);
}
 
jout(u)		/* print one line of job status in "u" */
struct us_rsf *u;
{
	register i, j;
	struct tm *tp, *localtime();
 
	tp = localtime(&u->qtime);
	printf("%.4s  %.7s  %.7s",u->jobn,u->user,u->rmt);
	printf("  %02d/%02d-%02d:%02d", tp->tm_mon+1, tp->tm_mday,
		tp->tm_hour, tp->tm_min);
	tp = localtime(&u->stime);	/* status time */
	printf("  %02d/%02d-%02d:%02d", tp->tm_mon+1, tp->tm_mday,
		tp->tm_hour, tp->tm_min);
	if (vflag)
		if (u->ustat == (USR_COK | USR_COMP))
			printf("  %s",us_rtext[11]);
		else
			for (j=1, i=u->ustat; i>0; j++, i=i>>1) {
				if (i&01) printf("  %s",us_rtext[j]);
			}
	else	printf("  %o", u->ustat);
	printf("\n");
	return(0);
}
 
 
/* older(rt,t) rt: request time;	t: hours.
   return 1 if rt older than current time by t hours or more. */
older(rt,t)
short t;
time_t rt;
{
	time_t ct;		/* current time */
	time(&ct);
	return ((ct-rt) > (t*3600L));
}
 
#else
main(argc, argv)
char **argv;
{
	fprintf(stderr,"Uustat is not implemented on this system\n");
}
#endif

cleanup(dummy)
int dummy;
{
/* so make will work for now */
}
