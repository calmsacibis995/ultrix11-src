
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)opser.c	3.0	4/22/86";
#include <stdio.h>
#include <signal.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <a.out.h>
#include <pwd.h>
#include <sys/stat.h>
#include <setjmp.h>
#define SULOGFILE	"/usr/adm/sulog"

struct sgttyb tty;
struct tchars ctty;
unsigned ttype;
FILE *tfd;
char instr[80], oustr[80], backfile[80];


char **cmd;
char *cmds[] =
{	"users",
	"shutdown",
	"backup",
	"restart",
	"help",
	"?",
	"exit",
	"quit",
	"bye",
	"!sh",
	"fsck",
	"halt",
	0,
};
char	*ttynam;
char	*ttyname();
char	cnsle[]	= {"/dev/console"};
char	*tsns = "\nCommand invalid unless time-sharing stopped\n";
int	hltcode[4] = {0102, 0, 0106, 0};
char *proced, *bname;
int dirfile, found;
struct direct dir;
struct nlist nl[] = {
	{ "_lks" },
	{ "trap" },
	{ "" },
};
jmp_buf jmpbuf;

main()
{
	extern onintr();

	ioctl(1, TIOCGETP, &tty);
	ioctl(1, TIOCGETC, &ctty);
	tty.sg_erase = '\177';
	tty.sg_kill = '\025';
	ctty.t_intrc = '\003';
	ioctl(0, TIOCSETC, &ctty);
	ioctl(0, TIOCSETP, &tty);
	ttype = LPRTERA|LCTLECH|LDECCTQ;
	ioctl(1, TIOCLSET, &ttype);
	sigset(SIGINT, onintr);
	sighold(SIGINT);
	fprintf(stdout, "\nULTRIX-11 Operator Services\n");
	fprintf(stdout, "\nTo correct typing mistakes:\n");
	fprintf(stdout, "\n\t<DELETE> erases the last character,");
	fprintf(stdout, "\n\t<CTRL/U> erases the entire line.\n");
	fprintf(stdout, "\nFor help, type h then press <RETURN>\n");
	sigrelse(SIGINT);
	setjmp(jmpbuf);
	menu();
}
menu()
{
	char c;
	int pntr;
	int	ruid, rgid;
	struct	passwd *pwd,*getpwnam(),*getpwuid();
 	char *nptr;
 	char oname[14];	/* orig name */
 	char *password;
	char	*crypt();
	char	*getpass();
	char	*getenv();
	while(1)
	{
		fprintf(stdout, "\nopr> ");
		for(pntr = 0; ; pntr++){
		/*	if(read(0, &c, 1) == 0) {  use stdio routines  -jsd */
			if((c = getchar()) == EOF) {
				goodbye();
				pntr--;
				continue;
			}
			if(c == '\n')
				break;
			instr[pntr] = c;
		}
		instr[pntr] = c = '\0';
		for(pntr = 0, cmd = cmds; *cmd; *cmd++, pntr++)
		{
			if(strncmp(*cmd, &instr,
			   (strlen(*cmd)<strlen(&instr))
			      ?strlen(*cmd):strlen(&instr)) == 0)
				break;
		}
		switch(pntr)
		{
			case 0:
				users();
				break;
			case 1:
				shutdown();
				break;
			case 2:
				backup();
				break;
			case 3:
				restart();
				break;
			case 4:
			case 5:
				help();
				break;
			case 6:
			case 7:
			case 8:
				goodbye();
				break;
			case 9:

 				ruid = getuid();
 				rgid = getgid();
 				nptr=getpwuid(ruid);
 				strcpy(oname,nptr->pw_name);
 				nptr = "root";
 				if((pwd=getpwnam(nptr)) == NULL) {
 					printf("Unknown id: %s\n",nptr);
 				exit(1);
 				}
 				if(pwd->pw_passwd[0] == '\0' )
 					goto ok;
 				password = getpass("Password:");
 				if(strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0) {
 						sulog(oname, nptr, password); 
 					printf("Sorry, You must know the password.\n");
 					break;
 				}
ok:

				fprintf(stdout, "type <CTRL/D> to return to opser\n");
				system("sh");
				break;
			case 10:
				if(access("/etc/sdloglock", 0)
	  				&& access("/etc/loglock", 0))
				    fprintf(stdout, "%s", tsns);
				else
				    system("/bin/fsck -p -t /tmp/fsck.tmp1234");
				break;
			case 11:
				if(access("/etc/sdloglock", 0)
	  				&& access("/etc/loglock", 0))
				    fprintf(stdout, "%s", tsns);
				else
					haltsys();
				break;
			default:
				fprintf(stdout, "\n\7\7\7Invalid command\n");
				fprintf(stdout, "For help type h");
				break;
		}
	}
}

help()
{

	fprintf(stdout, "\n() - may use first letter in place of full name\n");
	fprintf(stdout, "Valid commands are:\n\n");
	fprintf(stdout, "!sh\t\t- shell escape (execute ULTRIX-11 commands)\n");
	fprintf(stdout, "\t\t  (Type <CTRL/D> to return from shell)\n");
	fprintf(stdout, "(u)sers\t\t- show logged in users\n");
	fprintf(stdout, "(s)hutdown\t- stop time-sharing\n");
	fprintf(stdout, "(f)sck\t\t- file system checks\n");
	fprintf(stdout, "(r)estart\t- restart time-sharing\n");
	fprintf(stdout, "(h)elp\t\t- print this help message\n");
	fprintf(stdout, "backup cfn\t- file system backup\n");
	fprintf(stdout, "\t\t  (cfn = command file name)\n");
	fprintf(stdout, "halt\t\t- halt processor\n");
	fprintf(stdout, "^D (<CTRL/D>)\t- exit from opser");
	fprintf(stdout, "\n");
}

users()
{
	fprintf(stdout, "The following users are logged in :\n\n");
	system("who");
}

shutdown()
{
	if(access("/etc/sdloglock", 0) == 0
	    || access("/etc/loglock", 0) == 0){
		fprintf(stdout, "\nTime-sharing already stopped\n");
		return;
	}
	system("/opr/shutdown");
	sync();
}

backup()
{

	for(proced = instr; *proced != ' '; proced++){
		if(*proced == '\0'){
		    fprintf(stdout, "\nBackup requires command file name\n");
		    fprintf(stdout, "e.g.: backup daily or backup monthly\n");
		    return;
		}
	}
	proced++;
	strcpy(&backfile, "/opr/");
	strcat(&backfile, proced);
	strcat(&backfile, ".bak");
	if(access(&backfile, 0)){
		fprintf(stdout, "%s not found\n", proced);
		if((dirfile = open(".", 0)) <= 0){
			fprintf(stdout, "Cannot open '.'\n");
			return;
		}
		fprintf(stdout, "Existing backup command file are :\n");
		found = 0;
		while((read(dirfile, &dir, sizeof(struct direct)))
		   == sizeof(struct direct)){
			if(dir.d_ino == 0)
				continue;
			if(bname = index(dir.d_name, '.')){
				if(strcmp(bname, ".bak"))
					continue;
				*bname = '\0';
				fprintf(stdout, "%s\n", dir.d_name);
				found++;
			}
		}
		if(found == 0)
			fprintf(stdout, "No valid backup command files!\n");
		close(dirfile);
		return;
	}
	strcpy(&oustr, "sh ");
	strcat(&oustr, &backfile);
	system(&oustr);
}

restart()
{
	if(access("/etc/sdloglock", 0)
	  && access("/etc/loglock", 0))
		fprintf(stdout, "%s", tsns);
	else
		system("sh /opr/restart");
}

onintr()
{
	longjmp(jmpbuf,0);
}

goodbye()
{
	if(access("/etc/sdloglock", 0) == 0
	    || access("/etc/loglock", 0) == 0){
		fprintf(stdout, "\nTime-sharing stopped\n");
		return;
	}
	fprintf(stdout, "\nOpser terminating\n");
	exit(0);
}

haltsys()	/* Big gun here. Halts Cpu */
{
	int mem, tcnt, clkadr;
	char *coref;

	ttynam = ttyname(0);
	if (strcmp(cnsle, ttynam))
	{
		printf("\nHalt can only be run from the console device\n");
		return;
	}
	nlist("/unix", nl);
	if (nl[0].n_type==0) {
		fprintf(stderr, "No namelist\n");
		return;
	}
	coref = "/dev/mem";
	if ((mem = open(coref, 2)) < 0) {
		fprintf(stderr, "No mem\n");
		return;
	}
	hltcode[0] = (nl[1].n_value + 022);
	hltcode[2] = (nl[1].n_value + 022);
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, &clkadr, sizeof(clkadr));
	lseek(mem, ((long)clkadr)&~0377000000000L, 0);
	printf("Ready to halt system ? ");
	gets(instr);
	if(instr[0] != 'y'){
		close(mem);
		return;
	}
	printf("Halting System in 1 second\n");
	sync();
	sleep(1);
	sleep(1);
	tcnt = 0;
	write(mem, &tcnt, sizeof(tcnt));	/* turn off clock. i hope */
	lseek(mem, (long)0100, 0);
	write(mem, hltcode, sizeof(hltcode));
	lseek(mem, ((long)clkadr)&~0377000000000L, 0);
	tcnt = 0100;
	write(mem, &tcnt, sizeof(tcnt));	/* turn on clock. i hope */
	for(tcnt = 0; tcnt > 0; tcnt++);
	close(mem);
}

sulog(whofrom, whoto, password)
	register char *whofrom, *whoto, *password;
 {
 	register FILE	*logf;
 	int	i;
 	long	now;
 	char	*ttyn, *ttyname();
 	struct	stat statb;
 	
 	if (stat(SULOGFILE, &statb) < 0)
 		return;
 	if ((logf = fopen (SULOGFILE, "a")) == NULL)
 		return;
 	
 	for (i = 0; i < 3; i++)
 		if ((ttyn = ttyname(i)) != NULL)
 			break;
 	time (&now);
 	fprintf (logf, "%24.24s  %-8.8s  %-8.8s-> %-8.8s  ",
 			ctime(&now), ttyn+5, whofrom, whoto);
 	if (password == (char *) 0)
 		fprintf(logf, "OK\n");
 	else
 		fprintf(logf, "FAILED\n");
 	fclose (logf);
}
