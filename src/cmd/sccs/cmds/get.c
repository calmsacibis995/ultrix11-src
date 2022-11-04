
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

# include	"../hdr/defines.h"
# include	"../hdr/had.h"

static char Sccsid[] = "@(#)get.c 3.0 4/22/86";
USXALLOC();

int	Debug = 0;
int	had_pfile;
struct packet gpkt;
struct sid sid = {
	0,-1,0,0,
};
unsigned	Ser;
int	num_files;
char	had[26];
char	Pfilename[FILESIZE];
char	*ilist, *elist, *lfile;
long	cutoff = 0X7FFFFFFFL;	/* max positive long */
int verbosity;
char	Gfile[16];
char	*Type;
int	Did_id;

main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	register char *p;
	char c;
	int testmore;
	extern int Fcnt;
	extern get();

	Fflags = FTLEXIT | FTLMSG | FTLCLN;
	for(i=1; i<argc; i++)
		if(argv[i][0] == '-' && (c=argv[i][1])) {
			p = &argv[i][2];
			testmore = 0;
			switch (c) {

			case 'a':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				Ser = patoi(p);
				break;
			case 'r':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				chksid(sid_ab(p,&sid),&sid);
				if ((sid.s_rel < MINR) ||
					(sid.s_rel > MAXR))
					fatal("r out of range (ge22)");
				break;
			case 'c':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				if (date_ab(p,&cutoff))
					fatal("bad date/time (cm5)");
				break;
			case 'l':
				lfile = p;
				break;
			case 'i':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				ilist = p;
				break;
			case 'x':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				elist = p;
				break;
			case 'b':
			case 'g':
			case 'e':
			case 'p':
			case 'k':
			case 'm':
			case 'n':
			case 's':
			case 't':
				testmore++;
				break;
			default:
				fatal("unknown key letter (cm1)");
			}

			if (testmore) {
				testmore = 0;
				if (*p) {
					sprintf(Error,
						"value after %c arg (cm8)",c);
					fatal(Error);
				}
			}
			if (had[c - 'a']++)
				fatal("key letter twice (cm2)");
			argv[i] = 0;
		}
		else num_files++;

	if(num_files == 0)
		fatal("missing file arg (cm3)");
	if (HADE && HADM)
		fatal("e not allowed with m (ge3)");
	if (HADE)
		HADK = 1;
	if (!HADS)
		verbosity = -1;
	if (HADE && ! logname())
		fatal("User ID not in password file (cm9)");
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for (i=1; i<argc; i++)
		if (p=argv[i])
			do_file(p,get);
	exit(Fcnt ? 1 : 0);
}


extern char *Sflags[];

get(file)
{
	register char *p;
	register unsigned ser;
	extern char had_dir, had_standinp;
	struct stats stats;
	char	str[32];
	int	l_rel;
	int i,status;

	Fflags |= FTLMSG;
	if (setjmp(Fjmp))
		return;
	if (HADE) {
		had_pfile = 1;
		/*
		call `sinit' to check if file is an SCCS file
		but don't open the SCCS file.
		If it is, then create lock file.
		*/
		sinit(&gpkt,file,0);
		if (lockit(auxf(file,'z'),10,getpid()))
			fatal("cannot create lock file (cm4)");
	}
	/*
	Open SCCS file and initialize packet
	*/
	sinit(&gpkt,file,1);
	gpkt.p_ixuser = (HADI | HADX);
	gpkt.p_reqsid.s_rel = sid.s_rel;
	gpkt.p_reqsid.s_lev = sid.s_lev;
	gpkt.p_reqsid.s_br = sid.s_br;
	gpkt.p_reqsid.s_seq = sid.s_seq;
	gpkt.p_verbose = verbosity;
	gpkt.p_stdout = (HADP ? stderr : stdout);
	gpkt.p_cutoff = cutoff;
	gpkt.p_lfile = lfile;
	copy(auxf(gpkt.p_file,'g'),Gfile);

	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (dodelt(&gpkt,&stats,0,0) == 0)
		fmterr(&gpkt);
	finduser(&gpkt);
	doflags(&gpkt);
	if (!HADA)
		ser = getser(&gpkt);
	else {
		if ((ser = Ser) > maxser(&gpkt))
			fatal("serial number too large (ge19)");
		move(&gpkt.p_idel[ser].i_sid, &gpkt.p_gotsid, sizeof(sid));
		if (HADR && sid.s_rel != gpkt.p_gotsid.s_rel) {
			zero(&gpkt.p_reqsid, sizeof(gpkt.p_reqsid));
			gpkt.p_reqsid.s_rel = sid.s_rel;
		}
		else
			move(&gpkt.p_gotsid, &gpkt.p_reqsid, sizeof(sid));
	}
	doie(&gpkt,ilist,elist,0);
	setup(&gpkt,ser);
	if (!(Type = Sflags[TYPEFLAG - 'a']))
		Type = Null;
	if (!(HADP || HADG))
		if (exists(Gfile) && (S_IWRITE & Statbuf.st_mode)) {
			sprintf(Error,"writable `%s' exists (ge4)",Gfile);
			fatal(Error);
		}
	if (gpkt.p_verbose) {
		sid_ba(&gpkt.p_gotsid,str);
		fprintf(gpkt.p_stdout,"%s\n",str);
	}
	if (HADE) {
		if (HADC || gpkt.p_reqsid.s_rel == 0)
			move(&gpkt.p_gotsid, &gpkt.p_reqsid, sizeof(sid));
		newsid(&gpkt,Sflags[BRCHFLAG - 'a'] && HADB);
		permiss(&gpkt);
		if (exists(auxf(&gpkt.p_file,'p')))
			mk_qfile(&gpkt);
		else had_pfile = 0;
		wrtpfile(&gpkt,ilist,elist);
	}
	if (HADG) {
		fclose(gpkt.p_iop);
	}
	else {
		if (gpkt.p_stdout)
			fflush(gpkt.p_stdout);
		if ((i = fork()) < 0)
			fatal("cannot fork, try again");
		if (i == 0) {
			Fflags |= FTLEXIT;
			Fflags &= ~FTLJMP;
			setuid(getuid());
			if (HADL)
				gen_lfile(&gpkt);
			flushto(&gpkt,EUSERTXT,1);
			idsetup(&gpkt);
			gpkt.p_chkeof = 1;
			Did_id = 0;
			/*
			call `xcreate' which deletes the old `g-file' and
			creates a new one except if the `p' keyletter is set in
			which case any old `g-file' is not disturbed.
			The mod of the file depends on whether or not the `k'
			keyletter has been set.
			*/

			if (gpkt.p_gout == 0) {
				if (HADP)
					gpkt.p_gout = stdout;
				else
					gpkt.p_gout = xfcreat(Gfile,HADK ? 0644 : 0444);
			}
			while(readmod(&gpkt)) {
				prfx(&gpkt);
				p = idsubst(&gpkt,gpkt.p_line);
				fputs(p,gpkt.p_gout);
			}
			if (gpkt.p_gout)
				fflush(gpkt.p_gout);
			if (gpkt.p_gout && gpkt.p_gout != stdout)
				fclose(gpkt.p_gout);
			if (gpkt.p_verbose)
				fprintf(gpkt.p_stdout,"%u lines\n",gpkt.p_glnno);
			if (!Did_id && !HADK)
				if (Sflags[IDFLAG - 'a'])
					fatal("no id keywords (cm6)");
				else if (gpkt.p_verbose)
					printf("No id keywords (cm7)\n");
			exit(0);
		}
		else {
			wait(&status);
			if (status) {
				Fflags &= ~FTLMSG;
				fatal();
			}
		}
	}
	if (gpkt.p_iop)
		fclose(gpkt.p_iop);

	/*
	remove 'q'-file because it is no longer needed
	*/
	unlink(auxf(&gpkt.p_file,'q'));
	xfreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());

	/*
	 * Prompt for dot.why comments unless environment "SCCSWHY=off"
	 */
	if ((HADE) && (!had_dir)) {
	    if (((p=getenv("SCCSWHY")) == NULL) ||
	       (((p=getenv("SCCSWHY")) != NULL) && (strcmp(p,"off") != 0)))
		    dodotwhy();
	}
}
/*
 * If the file is being edited with "get -e" or "edit",
 * prompt for a line or two for inclusion in 'file.why'
 * so we know who currently has it checked out and why.
 * 	--John Dustin  11/12/84
 */
dodotwhy()
{
#include <signal.h>
	extern char had_dir;
	extern int errno;
	int cc, done, gotit, whyfd;
	char *user;
	char *termname;
	char *date;
	char whyfile[FILESIZE];
	char whytmp[FILESIZE];
	char whybuff[BUFSIZ];
	struct stat sb;

	/*
	 * Up to 10-character file names will work,
	 * otherwise, prompt for a why-file name.
	 */
	signal(SIGINT, SIG_IGN);
	if (strlen(Gfile) > 10) {
		printf("Cannot create '%s.why' (why-file name too long).\n",Gfile);
		printf("Press <CTRL/D> to cancel, or <RETURN> to use default name.\n");

		/*
		 * Loop forever, until something reasonable is
		 * entered or the user enters <CR> alone on a line,
		 * or a <CTRL/D> (end of file).
		 */
		for (;;) {
			printf("\nEnter an alternate why-file name");
			/*
				printf(" (ie: %.10s",Gfile);
				if (Gfile[9] != '.')
					printf(".");
				printf("why): ");
			 */
			/*
			 * Set up whytmp with the suggested default why-name
			 */
			sprintf(whytmp, "%.10s", Gfile);
			if (whytmp[9] != '.')
				strcat(whytmp, ".");
			strcat(whytmp, "why");
			printf(" < %s >: ", whytmp);
			if (fgets(whybuff, 132, stdin) != NULL) {
				if (strlen(whybuff) > 1) {
					/* get rid of '\n' in whybuff */
					whybuff[strlen(whybuff)-1] = '\0';
					if (strlen(whybuff) > 14) {
						printf("\nPlease use a shorter file name (14 characters or less).\n");
						continue;
					}
					if (stat(whybuff, &sb) < 0) {
						;	/* OK */
					} else {
						printf("\n'%s' file already exists, try something else.\n", whybuff);
						continue;
					}
				} else {
					/*
					 * entered a <CR> only, just
					 * use constructed why-file name.
					 */
					strcpy(whybuff, whytmp);
					if (stat(whybuff, &sb) < 0) {
						;	/* OK */
					} else {
						printf("\n'%s' file already exists, try something else.\n", whybuff);
						continue;
					}
				}
			} else {	/* CTRL/D entered */
				printf("\nNo why-file created.\n");
				return(1);
			}
			/*
			 * Use the name which the user entered.
			 * filename already has the ".why" extension
			 */
			strcpy(whyfile, whybuff);
			printf("\n");
			break;
		}  /* end for(;;) */
	} else {
		strcpy(whyfile,Gfile);
		strcat(whyfile,".why\0");
	}

	if ((whyfd = creat(whyfile,0644)) < 0) {
		printf("cannot create %s (errno=%d)\n",whyfile,errno);
		printf("No why-file created!\n");
		return(1);
	}
	if (stat(Gfile, &sb) < 0) {	/* should never fail... */
		printf("\ncannot stat %s (errno=%d)\n",Gfile, errno);
		return(1);
	}
	if ((chown(whyfile, sb.st_uid, sb.st_gid)) < 0) {
		printf("cannot chown %s to uid=%d, gid=%d (errno=%d)\n",whyfile,sb.st_uid,sb.st_gid,errno);
	}
	if ((chmod(whyfile, 0644)) < 0) {
		printf("cannot chmod %s to 0644 (errno=%d)\n",whyfile,errno);
	}
	/*
	 * whybuff is initially set up with one line
	 * of user information: name, tty, and date
	 */
	if ((user = getlogin()) == NULL)
		strcpy(whybuff,"???");
	else
		strcpy(whybuff, user);
	strcat(whybuff, "  ");
	if ((termname = ttyname(0)) == NULL)
		strcat(whybuff,"/dev/tty??");
	else
		strcat(whybuff, termname);
	strcat(whybuff, "  ");
	/*
	 * 'date' is the create time of the why-file
	 */
	stat(whyfile, &sb);
	date = '\0';
	date = ctime(&sb.st_ctime);
	strcat(whybuff, date);
	strcat(whybuff, "\n\0");
	cc = strlen(whybuff);
	if (write(whyfd, whybuff, cc) != cc) {
		printf("\ncannot write to %s (errno=%d)\n",whyfile, errno);
		close(whyfd);
		return(1);
	}
	printf("Comments for '%s' ? (CTRL/D when complete):\n", whyfile);
	cc=done=gotit=0;
	do {
		if (fgets(whybuff, 132, stdin) != NULL) {
			if (strlen(whybuff) > 1)
				gotit++;
		} else {		 /* CTRL/D */
			if (gotit)
				done++;
			else
				printf("\ncomments? ");
			continue;
		}
		if ((whybuff[0] == '.') && (whybuff[1] == '\n')) {
			if (gotit) {
				whybuff[0]='\0';
				done++;
			}
		}
		/*
		 * write a line at a time to the whyfile
		 */
		if ((cc = strlen(whybuff)) > 0) {
			if (write(whyfd,whybuff,cc) != cc) {
				printf("%s: write error (errno=%d)\n", whyfile,errno);
				done++;
			}
		}
	} while (! done);
	close(whyfd);
	signal(SIGINT, SIG_DFL);
}

newsid(pkt,branch)
register struct packet *pkt;
int branch;
{
	int chkbr;

	chkbr = 0;
	/* if branch value is 0 set newsid level to 1 */
	if (pkt->p_reqsid.s_br == 0) {
		pkt->p_reqsid.s_lev += 1;
		/*
		if the sid requested has been deltaed or the branch
		flag was set or the requested SID exists in the p-file
		then create a branch delta off of the gotten SID
		*/
		if (sidtoser(&pkt->p_reqsid,pkt) ||
			pkt->p_maxr > pkt->p_reqsid.s_rel || branch ||
			in_pfile(&pkt->p_reqsid,pkt)) {
				pkt->p_reqsid.s_rel = pkt->p_gotsid.s_rel;
				pkt->p_reqsid.s_lev = pkt->p_gotsid.s_lev;
				pkt->p_reqsid.s_br = pkt->p_gotsid.s_br + 1;
				pkt->p_reqsid.s_seq = 1;
				chkbr++;
		}
	}
	/*
	if a three component SID was given as the -r argument value
	and the b flag is not set then up the gotten SID sequence
	number by 1
	*/
	else if (pkt->p_reqsid.s_seq == 0 && !branch)
		pkt->p_reqsid.s_seq = pkt->p_gotsid.s_seq + 1;
	else {
		/*
		if sequence number is non-zero then increment the
		requested SID sequence number by 1
		*/
		pkt->p_reqsid.s_seq += 1;
		if (branch || sidtoser(&pkt->p_reqsid,pkt) ||
			in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			pkt->p_reqsid.s_seq = 1;
			chkbr++;
		}
	}
	/*
	keep checking the requested SID until a good SID to be
	made is calculated or all possibilities have been tried
	*/
	while (chkbr) {
		--chkbr;
		while (in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
		while (sidtoser(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
	}
	if (sidtoser(&pkt->p_reqsid,pkt) || in_pfile(&pkt->p_reqsid,pkt))
		fatal("bad SID calculated in newsid()");
}


enter(pkt,ch,n,sidp)
struct packet *pkt;
char ch;
int n;
struct sid *sidp;
{
	char str[32];
	register struct apply *ap;

	sid_ba(sidp,str);
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,"%s\n",str);
	ap = &pkt->p_apply[n];
	switch(ap->a_code) {

	case EMPTY:
		if (ch == INCLUDE)
			condset(ap,APPLY,INCLUSER);
		else
			condset(ap,NOAPPLY,EXCLUSER);
		break;
	case APPLY:
		sid_ba(sidp,str);
		sprintf(Error,"%s already included (ge9)",str);
		fatal(Error);
		break;
	case NOAPPLY:
		sid_ba(sidp,str);
		sprintf(Error,"%s already excluded (ge10)",str);
		fatal(Error);
		break;
	default:
		fatal("internal error in get/enter() (ge11)");
		break;
	}
}


gen_lfile(pkt)
register struct packet *pkt;
{
	int n;
	int reason;
	char str[32];
	char line[BUFSIZ];
	struct deltab dt;
	FILE *in;
	FILE *out;

	in = xfopen(pkt->p_file,0);
	if (*pkt->p_lfile)
		out = stdout;
	else
		out = xfcreat(auxf(pkt->p_file,'l'),0444);
	fgets(line,sizeof(line),in);
	while (fgets(line,sizeof(line),in) != NULL && line[0] == CTLCHAR && line[1] == STATS) {
		fgets(line,sizeof(line),in);
		del_ab(line,&dt);
		if (dt.d_type == 'D') {
			reason = pkt->p_apply[dt.d_serial].a_reason;
			if (pkt->p_apply[dt.d_serial].a_code == APPLY) {
				putc(' ',out);
				putc(' ',out);
			}
			else {
				putc('*',out);
				if (reason & IGNR)
					putc(' ',out);
				else
					putc('*',out);
			}
			switch (reason & (INCL | EXCL | CUTOFF)) {
	
			case INCL:
				putc('I',out);
				break;
			case EXCL:
				putc('X',out);
				break;
			case CUTOFF:
				putc('C',out);
				break;
			default:
				putc(' ',out);
				break;
			}
			putc(' ',out);
			sid_ba(&dt.d_sid,str);
			fprintf(out,"%s\t",str);
			date_ba(&dt.d_datetime,str);
			fprintf(out,"%s %s\n",str,dt.d_pgmr);
		}
		while ((n = fgets(line,sizeof(line),in)) != NULL)
			if (line[0] != CTLCHAR)
				break;
			else {
				switch (line[1]) {

				case EDELTAB:
					break;
				default:
					continue;
				case MRNUM:
				case COMMENTS:
					if (dt.d_type == 'D')
						fprintf(out,"\t%s",&line[3]);
					continue;
				}
				break;
			}
		if (n == NULL || line[0] != CTLCHAR)
			break;
		putc('\n',out);
	}
	fclose(in);
	if (out != stdout)
		fclose(out);
}


char	Curdate[18];
char	*Curtime;
char	Gdate[9];
char	Chgdate[18];
char	*Chgtime;
char	Gchgdate[9];
char	Sid[32];
char	Mod[16];
char	Olddir[BUFSIZ];
char	Pname[BUFSIZ];
char	Dir[BUFSIZ];
char	*Qsect;

idsetup(pkt)
register struct packet *pkt;
{
	extern long Timenow;
	register int n;
	register char *p;

	date_ba(&Timenow,Curdate);
	Curtime = &Curdate[9];
	Curdate[8] = 0;
	makgdate(Curdate,Gdate);
	for (n = maxser(pkt); n; n--)
		if (pkt->p_apply[n].a_code == APPLY)
			break;
	if (n)
		date_ba(&pkt->p_idel[n].i_datetime,Chgdate);
	Chgtime = &Chgdate[9];
	Chgdate[8] = 0;
	makgdate(Chgdate,Gchgdate);
	sid_ba(&pkt->p_gotsid,Sid);
	if (p = Sflags[MODFLAG - 'a'])
		copy(p,Mod);
	else
		copy(Gfile,Mod);
	if (!(Qsect = Sflags[QSECTFLAG - 'a']))
		Qsect = Null;
}


makgdate(old,new)
register char *old, *new;
{
	if ((*new = old[3]) != '0')
		new++;
	*new++ = old[4];
	*new++ = '/';
	if ((*new = old[6]) != '0')
		new++;
	*new++ = old[7];
	*new++ = '/';
	*new++ = old[0];
	*new++ = old[1];
	*new = 0;
}


static char Zkeywd[5] = "@(#)";

idsubst(pkt,line)
register struct packet *pkt;
char line[];
{
	static char tline[BUFSIZ];
	static char str[32];
	register char *lp, *tp;
	extern char *Type;
	extern char *Sflags[];

	if (HADK || !any('%',line))
		return(line);

	tp = tline;
	for(lp=line; *lp != 0; lp++) {
		if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%') {
			switch(*++lp) {

			case 'M':
				tp = trans(tp,Mod);
				break;
			case 'Q':
				tp = trans(tp,Qsect);
				break;
			case 'R':
				sprintf(str,"%u",pkt->p_gotsid.s_rel);
				tp = trans(tp,str);
				break;
			case 'L':
				sprintf(str,"%u",pkt->p_gotsid.s_lev);
				tp = trans(tp,str);
				break;
			case 'B':
				sprintf(str,"%u",pkt->p_gotsid.s_br);
				tp = trans(tp,str);
				break;
			case 'S':
				sprintf(str,"%u",pkt->p_gotsid.s_seq);
				tp = trans(tp,str);
				break;
			case 'D':
				tp = trans(tp,Curdate);
				break;
			case 'H':
				tp = trans(tp,Gdate);
				break;
			case 'T':
				tp = trans(tp,Curtime);
				break;
			case 'E':
				tp = trans(tp,Chgdate);
				break;
			case 'G':
				tp = trans(tp,Gchgdate);
				break;
			case 'U':
				tp = trans(tp,Chgtime);
				break;
			case 'Z':
				tp = trans(tp,Zkeywd);
				break;
			case 'Y':
				tp = trans(tp,Type);
				break;
			case 'W':
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Mod);
				*tp++ = '\t';
			case 'I':
				tp = trans(tp,Sid);
				break;
			case 'P':
				copy(pkt->p_file,Dir);
				dname(Dir);
				if(curdir(Olddir) != 0)
					fatal("curdir failed (ge20)");
				if(chdir(Dir) != 0)
					fatal("cannot change directory (ge21)");
				if(curdir(Pname) != 0)
					fatal("curdir failed (ge20)");
				if(chdir(Olddir) != 0)
					fatal("cannot change directory (ge21)");
				tp = trans(tp,Pname);
				*tp++ = '/';
				tp = trans(tp,(sname(pkt->p_file)));
				break;
			case 'F':
				tp = trans(tp,pkt->p_file);
				break;
			case 'C':
				sprintf(str,"%u",pkt->p_glnno);
				tp = trans(tp,str);
				break;
			case 'A':
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Type);
				*tp++ = ' ';
				tp = trans(tp,Mod);
				*tp++ = ' ';
				tp = trans(tp,Sid);
				tp = trans(tp,Zkeywd);
				break;
			default:
				*tp++ = '%';
				*tp++ = *lp;
				continue;
			}
			lp++;
		}
		else
			*tp++ = *lp;
	}

	*tp = 0;
	return(tline);
}


trans(tp,str)
register char *tp, *str;
{
	Did_id = 1;
	while(*tp++ = *str++)
		;
	return(tp-1);
}


prfx(pkt)
register struct packet *pkt;
{
	char str[32];

	if (HADN)
		fprintf(pkt->p_gout,"%s\t",Mod);
	if (HADM) {
		sid_ba(&pkt->p_inssid,str);
		fprintf(pkt->p_gout,"%s\t",str);
	}
}


clean_up(n)
{
	/*
	clean_up is only called from fatal() upon bad termination.
	*/
	if (gpkt.p_iop)
		fclose(gpkt.p_iop);
	if (gpkt.p_gout) {
		fflush(gpkt.p_gout);
		if (gpkt.p_gout != stdout) {
			fclose(gpkt.p_gout);
			unlink(Gfile);
		}
	}
	if (HADE) {
		if (! had_pfile) {
			unlink(auxf(&gpkt.p_file,'p'));
		}
		else if (exists(auxf(&gpkt.p_file,'q'))) {
			copy(auxf(&gpkt.p_file,'p'),Pfilename);
			rename(auxf(&gpkt.p_file,'q'),Pfilename);
		     }
	}
	xfreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());
}


static	char	warn[] = "WARNING: being edited: `%s' (ge18)\n";

wrtpfile(pkt,inc,exc)
register struct packet *pkt;
char *inc, *exc;
{
	char line[64], str1[32], str2[32];
	char *user;
	FILE *in, *out;
	struct pfile pf;
	register char *p;
	int fd;
	int i;
	extern long Timenow;

	user = logname();
	if (exists(p = auxf(pkt->p_file,'p'))) {
		fd = xopen(p,2);
		in = fdfopen(fd,0);
		while (fgets(line,sizeof(line),in) != NULL) {
			p = line;
			p[length(p) - 1] = 0;
			pf_ab(p,&pf,0);
			if (!(Sflags[JOINTFLAG - 'a'])) {
				if ((pf.pf_gsid.s_rel == pkt->p_gotsid.s_rel &&
     				   pf.pf_gsid.s_lev == pkt->p_gotsid.s_lev &&
				   pf.pf_gsid.s_br == pkt->p_gotsid.s_br &&
				   pf.pf_gsid.s_seq == pkt->p_gotsid.s_seq) ||
				   (pf.pf_nsid.s_rel == pkt->p_reqsid.s_rel &&
				   pf.pf_nsid.s_lev == pkt->p_reqsid.s_lev &&
				   pf.pf_nsid.s_br == pkt->p_reqsid.s_br &&
				   pf.pf_nsid.s_seq == pkt->p_reqsid.s_seq)) {
					fclose(in);
					sprintf(Error,
					     "being edited: `%s' (ge17)",line);
					fatal(Error);
				}
				if (!equal(pf.pf_user,user))
					fprintf(stderr,warn,line);
			}
			else fprintf(stderr,warn,line);
		}
		out = fdfopen(dup(fd),1);
		fclose(in);
	}
	else
		out = xfcreat(p,0644);
	fseek(out,0L,2);
	sid_ba(&pkt->p_gotsid,str1);
	sid_ba(&pkt->p_reqsid,str2);
	date_ba(&Timenow,line);
	fprintf(out,"%s %s %s %s",str1,str2,user,line);
	if (inc)
		fprintf(out," -i%s",inc);
	if (exc)
		fprintf(out," -x%s",exc);
	fprintf(out,"\n");
	fclose(out);
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,"new delta %s\n",str2);
}


getser(pkt)
register struct packet *pkt;
{
	register struct idel *rdp;
	int n, ser, def;
	char *p;
	extern char *Sflags[];

	def = 0;
	if (pkt->p_reqsid.s_rel == 0) {
		if (p = Sflags[DEFTFLAG - 'a'])
			chksid(sid_ab(p, &pkt->p_reqsid), &pkt->p_reqsid);
		else {
			pkt->p_reqsid.s_rel = MAX;
			def = 1;
		}
	}
	ser = 0;
	if (pkt->p_reqsid.s_lev == -1) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if ((rdp->i_sid.s_br == 0 || HADT) &&
				pkt->p_reqsid.s_rel >= rdp->i_sid.s_rel &&
				rdp->i_sid.s_rel > pkt->p_gotsid.s_rel) {
					ser = n;
					pkt->p_gotsid.s_rel = rdp->i_sid.s_rel;
			}
		}
	}
	/*
	 * If had '-t' keyletter and R.L SID type, find
	 * the youngest SID
	*/
	else if ((pkt->p_reqsid.s_br == 0) && HADT) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
			    rdp->i_sid.s_lev == pkt->p_reqsid.s_lev )
				break;
		}
		ser = n;
	}
	else if (pkt->p_reqsid.s_br && pkt->p_reqsid.s_seq == 0) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
				rdp->i_sid.s_lev == pkt->p_reqsid.s_lev &&
				rdp->i_sid.s_br == pkt->p_reqsid.s_br)
					break;
		}
		ser = n;
	}
	else {
		ser = sidtoser(&pkt->p_reqsid,pkt);
	}
	if (ser == 0)
		fatal("nonexistent sid (ge5)");
	rdp = &pkt->p_idel[ser];
	move(&rdp->i_sid, &pkt->p_gotsid, sizeof(pkt->p_gotsid));
	if (def || (pkt->p_reqsid.s_lev == 0 && pkt->p_reqsid.s_rel == pkt->p_gotsid.s_rel))
		move(&pkt->p_gotsid, &pkt->p_reqsid, sizeof(pkt->p_gotsid));
	return(ser);
}


/* Null routine to satisfy external reference from dodelt() */

escdodelt()
{
}


in_pfile(sp,pkt)
struct	sid	*sp;
struct	packet	*pkt;
{
	struct	pfile	pf;
	char	line[BUFSIZ];
	char	*p;
	FILE	*in;

	if (Sflags[JOINTFLAG - 'a']) {
		if (exists(auxf(pkt->p_file,'p'))) {
			in = xfopen(auxf(pkt->p_file,'p'),0);
			while ((p = fgets(line,sizeof(line),in)) != NULL) {
				p[length(p) - 1] = 0;
				pf_ab(p,&pf,0);
				if (pf.pf_nsid.s_rel == sp->s_rel &&
					pf.pf_nsid.s_lev == sp->s_lev &&
					pf.pf_nsid.s_br == sp->s_br &&
					pf.pf_nsid.s_seq == sp->s_seq) {
						fclose(in);
						return(1);
				}
			}
			fclose(in);
		}
		else return(0);
	}
	else return(0);
}


mk_qfile(pkt)
register struct	packet *pkt;
{
	FILE	*in, *qout;
	char	line[BUFSIZ];

	in = xfopen(auxf(pkt->p_file,'p'),0);
	qout = xfcreat(auxf(pkt->p_file,'q'),0644);

	while ((fgets(line,sizeof(line),in) != NULL))
		fputs(line,qout);
	fclose(in);
	fclose(qout);
}

chksid(p,sp)
char *p;
register struct sid *sp;
{
	if (*p ||
		(sp->s_rel == 0 && sp->s_lev))
			fatal("invalid sid (co8)");
}

char *satoi(p,ip)
register char *p;
register int *ip;
{
	register int sum;

	if(*p){
		sum = 0;
		while (numeric(*p))
			sum = sum * 10 + (*p++ - '0');
		*ip = sum;
	}
	return(p);
}
