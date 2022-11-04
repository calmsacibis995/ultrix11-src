
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 tty edit program.
 *
 * John Dustin	2/16/84
 *
 * This program allows modification of the /etc/ttys file
 * without the use of an editor.  It is recommended that 
 * this program be run single-user to avoid disabling any
 * active terminals.
 *
 */

#define JUMP	/* include setjmp/longjmp code to trap <CTRL/C> */

static char Sccsid[] =	"@(#)ted.c	3.0	4/22/86";

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LINELEN		132		/* length of an input line */
#define TRANSAC		20		/* longest transaction type */
#define ENTRYLEN	128		/* ttyfile entry length */
#define TTYXXLEN	16		/* "ttyxx" string (remember console) */
#define XXLEN		4		/* "nn" mode string */
#define BADDEV		9999		/* ?/? device number not in /dev */
#define STARTNUM    	0		/* start count for page one */		
#define SCREENFUL	12		/* lines to display per screen */

#define FAILED		-1
#define NOTFOUND	-2
#define BADCMD		-3
#define NOGO		-4

/*
 * Some of the following are defined twice
 * to allow for single characters getting
 * interpreted correctly.  Note the lack of
 * ambiguity since the command line is taken
 * in context.
 */

#define NO	0
#define	NOLOGIN	0
#define NAME	0
#define YES	1
#define	HELP	2
#define	CREATE	3
#define	REMOVE	4
#define	REMOTE	4
#define	WRITE	5
#define	PRINT	6
#define	LOCAL	7
#define	DISABLE	8
#define	DISCARD	8
#define	CTRLD	9
#define	NOREPLY	10			/* carriage return */
#define DIDPW	11			/* did print or write only */
#define HELPTED 12
#define H_CTRLC 13
#define H_CTRLD 14
#define MODIFY	15
#define MODE	15
#define SPEED	16

#define	EXIT	20

#define VALID		sizeof(valid)
#define DEVSIZ		(sizeof(dev) - 1)	/* else lose one char */

char	valid[50];				/* valid gettytab chars */
char	tmode[XXLEN];				/* "xx" mode characters */
char	newmode[XXLEN];				/* new "xx" mode characters */
char	line[LINELEN];				/* input line */
char	fline[LINELEN];				/* line read from ttyfile */
char	termname[TTYXXLEN];			/* name of tty read */
char	repterm[TTYXXLEN];			/* name of replacement tty */
char	ourtty[TTYXXLEN];			/* the user's tty */
char	progname[] =	"ted";			/* for perror */
char	dev[] =	"/dev/";			/* initial path to device */
char	testend[TTYXXLEN+DEVSIZ];		/* device pathname */
char	gettytab[] =	"/etc/gettytab";	/* getty table */
char	ttyfile[] =	"/etc/ttys";		/* the ttyfile */
char	tmp0file[] =	"/etc/tty.tmpXXXXXX";	/* temporary ttys file */
char	midfile[] =	"/etc/tty.midXXXXXX";	/* temporary link file */
char	lockfile[] =	"/tmp/TED.EDIT_LOCK";
char	added[] =	"added";
char	removed[] =	"removed";
char	changed[] =	"changed to ";
char	youarehere[] =	" [ * you are here ]";
char	dnp[] =		"device not present in /dev";
char	dot[] =		"";
char	dis[] = 	"disabled";
char	rem[] = 	"remote";
char	loc[] = 	"local";
char	locdis[] = 	"local (no logins)";
char	unk[] = 	"terminal mode unknown";
char	h_ctrlc[] =	"\nTo generate <CTRL/C>, you hold down the CTRL key and press C\n";
char	h_ctrld[] =	"\nTo generate <CTRL/D>, you hold down the CTRL key and press D\n";
char	badent[] =	"\n'%s' is not a valid command\n";
char	badtty[] =	"\n'%s' is not a valid terminal name\n";
char	notexist[] =	"\n%s does not exist.\n\nYou must enter a terminal name which currently exists in\nthe /etc/ttys file.";
char	exists[] =	"\n%s already exists.\n\nYou must enter a terminal name which is not currently in\nthe /etc/ttys file.";
char	isalready[] =	"\n%s is already -> ";
char	sayprint[] =	"\n\n[ enter 'print' to obtain a list of current terminals ]\n";
char	pretfm[] =	"\n\nPress <RETURN> for more: ";
char	h_pretfm[] =	"\nPress <RETURN> to see more terminals,\nPress <CTRL/D> to back up to the previous question.";
char	getround[] =	"\n[ To back up to the previous question press <CTRL/D> ]\n[ To return to the main menu press <CTRL/C> ]\n";
char	filenotfnd[] =	"\nFile must exist in /dev before you may access it.\n";
char	heading[] =	"\nMajor\tMinor\tName\t  Type\n---------------------------------------";
char	tty1help[] =	"\nTerminal names consist of the letters 'tty' followed by 2 unique\ncharacters.";
char	tty2help[] =	"  Valid terminal names are of the form: 'tty07', 'ttyd2',\n'ttypa'.  Enter only one terminal name at a time.";
int	length;			/* real length of the input line */
int	keepupper;		/* don't convert to lower case */
int	modify;			/* modifying a tty entry */
int	create;			/* create a new entry */
int	disable;		/* disabling a tty */
int	removing;		/* removing a tty entry from the ttyfile */
int	changing;		/* changing the name or speed of tty */
int	without;		/* enabling a local (no logins) */
int	quiting;		/* quiting the program */
int	sendhup;		/* remind about sending 'kill -1 1' at end */
int	nowriteallowed=0;	/* disallow multiple writes to pile up */
FILE	*tmpstrm;		/* for tmp0file */
FILE	*ttystrm;		/* for ttyfile */
FILE	*gtabstrm;		/* for gettytab file */
FILE	*lockstrm;		/* for lockfile */

/* 
 * These are kept alphabetical so that we don't get
 * two with the same significance.  They are taken
 * in context, hence the lack of ambiguity.
 */
struct	cmdtyp {
	char	*cmd_name;	/* name of command */
	int	cmd_id;		/* command ID */
} cmdtyp[] = {
	"?",		HELP,
	"bye",		EXIT,
	"create",	CREATE,
	"disabled",	DISABLE,
	"discard",	DISCARD,
	"exit",		EXIT,
	"help",		HELP,
	"help ted",	HELPTED,
	"local",	LOCAL,
	"mode",		MODE,
	"modify",	MODIFY,
	"name",		NAME,
	"no",		NO,
	"nologins",	NOLOGIN,
	"print",	PRINT,
	"quit",		EXIT,
	"remove",	REMOVE,
	"remote",	REMOTE,
	"speed",	SPEED,
	"write",	WRITE,
	"yes",		YES,
	"0",		DISABLE,
	"1",		REMOTE,
	"2",		LOCAL,
	"3",		NOLOGIN,
	"ctrl/c",	H_CTRLC,
	"^c",		H_CTRLC,
	"ctrl/d",	H_CTRLD,
	"^d",		H_CTRLD,
	0,		BADCMD,
};

struct llist {			/* linked list of tty entrys */
	char tmode[XXLEN];	/* tty mode */
	char tname[TTYXXLEN];	/* tty name */
	char ttype[TRANSAC];	/* transaction type */
	int  maj;		/* major device num */
	int  min;		/* minor device num */
	char *comment;		/* comment, if any (gets malloc'd) */
	struct llist *next;	/* next entry in list */
};

struct llist *head;		/* head of ttys list */
struct llist *deadhead;		/* head of new changes list */

struct stat statbuf;

#ifdef JUMP
struct jmp_buf *jmpbuf;
struct jmp_buf *savbuf;
#endif

jmp_buf	printbuf;
jmp_buf	mainbuf;

int	(*savsig)();

#ifdef JUMP
interr()
{
	signal(SIGINT, interr);
	printf("\n");
	longjmp(jmpbuf, 1);
}
#endif

/*
 * various help messages 
 */

char	*h_general[] =
{
	"Ted allows you to modify the /etc/ttys file without the use",
	"of an editor.",
	"",
	"To print the contents of the /etc/ttys file, enter 'print'.",
	"To enable or disable existing terminals, enter 'modify'.",
	"To create new terminal entries in the /etc/ttys file, enter",
	"'create'.",
	"",
	"The < command list > prompt means ted is asking you to select",
	"one of the commands from the command list.  You can enter just",
	"the first letter of the command name to select the command.",
	"",
	"Ted keeps track of the changes that you have made to the",
	"/etc/ttys file.  Your new changes are not permanently saved",
	"in the /etc/ttys file until you use the 'write' command.  Ted",
	"will ask if you try to quit without saving the latest changes.",
	"",
	"Enter 'help' or '?' at any time for more help with the current",
	"command.",
	"",
	0
};
char	*h_opts1[] =
{
	"Summary of TED commands:",
	"",
	"(c) create - allows you to create a new terminal entry in the",
	"	     /etc/ttys file.",
	"(m) modify - allows you to modify the name, speed, or mode",
	"	     of any existing terminal entry.  You use the",
	"	     'modify' command to enable and disable terminals.",
	"(r) remove - removes an entry from the /etc/ttys file.",
	"(p) print  - prints the mode of all the terminals currently",
	"	     in the ttys file.  The major and minor device",
	"	     numbers are listed along with the name and mode",
	"	     of the terminal.  The 'print' command shows the",
	"	     current status of the /etc/ttys file.",
	"(w) write  - writes out your latest changes to the /etc/ttys",
	"	     file.  The changes made since the last 'write'",
	"	     are included in the /etc/ttys file permanently.",
	"(e) exit   - exits the program.  'quit' and 'bye' also exit.",
	"",
	"The 'print' or 'write' options may be entered at any time.",
	"",
	0
};
char	*h_opts2[] =
{
	"The first character of the /etc/ttys entry reflects the",
	"mode of the terminal.  For example, the first character",
	"is '2' for a local terminal line.",
	"",
	"Choose from the following terminal modes:\n",
	"(d) disabled  - '0', disables a terminal.  The terminal is",
	"                     ignored by the system.",
	"(r) remote    - '1', enables a remote (dial-up) terminal.",
	"(l) local     - '2', enables a local terminal.",
	"(n) nologins  - '3', no logins allowed on the line.",
	"                     Useful for outgoing TIP/UUCP lines.",
	"",
	"Hard wired terminals are usually 'local'.  Terminals",
	"which are not being used, or are not connected, are usually",
	"'disabled'.",
	"",
	0
};
char	*h_modify[] =
{
	"For any given entry, there are three characteristics that",
	"may be modified.  These are:",
	"",
	"(m) mode -  this selects the mode of the terminal.  The",
	"	    possible modes are disabled, remote, local, and",
	"	    nologins.  The first character of the /etc/ttys",
	"	    entry selects the terminal mode.",
	"(s) speed - this selects the sequence of line speeds which",
	"	    getty cycles through when first logging in.  The",
	"	    second character of the /etc/ttys entry selects",
	"	    the line speed.",
	"(n) name  - this is the actual name of the terminal.  The",
	"	    name can be changed to anything you wish, but",
	"	    the new name must already exist in the /dev",
	"	    directory.",
	"",
	0
};

char	*h_getch[] =
{
	"The second character of the /etc/ttys entry is used to index",
	"the /etc/gettytab table. The referenced gettytab entry",
	"determines what sequence of line speeds getty cycles through",
	"when logging in.  Each time the <BREAK> key is depressed,",
	"getty changes the line speed to the next value specified in",
	"the table.  Sometimes speed recognition is unnecessary, in",
	"which case the 'login: ' message is just reprinted.",
	"",
	"Refer to Getty(8) in the ULTRIX-11 Programmers Manual, Vol. 1",
	"for a complete description of this procedure.",
	"",
	"	 0   - Cycles through 300-1200-150-110 baud.",
	"	       Useful for default dialup lines.",
	"	 1   - Optimized for a 150-baud teletype model 37.",
	"	 2   - Intended for an on-line 9600-baud terminal.",
	"	 3   - Starts at 1200-baud, cycles to 300 and back.",
	"	 4   - Useful for on-line console DECwriter (LA36).",
	"    others   - See Getty(8) in the ULTRIX-11 Programmers Manual.",
	"",
	0
};

char	*h_decide[] =
{
	"Choose from one of the following:",
	"",
	"(y) yes     - include these changes in the /etc/ttys file.",
	"	      Newly enabled or disabled terminals are written",
	"	      to the /etc/ttys file permanently.",
	"(n) no      - do not update the /etc/ttys file yet, but",
	"	      reserve these changes internally for later.",
	"(d) discard - discard all of these changes and start over.",
	"",
	0
};
char	*h_sendhup[] =
{
	"After modifying the /etc/ttys file, it is necessary to notify",
	"init of the changes that have been made.  To do this, the",
	"hangup signal (-1) is sent to init.  The command 'kill -1 1'",
	"causes init to reread the /etc/ttys file, activating any",
	"terminals which have been altered since the last reboot.",
	"",
	"If this signal is not sent, the altered terminals will not be",
	"recognized until the next reboot, or until the next time init",
	"receives a hangup signal.",
	"",
	0
};

main()
{
	int	i;

	signal(SIGTERM, SIG_IGN); 	/* Ignore signal 15 so we don't get */
					/* killed by someone else trying to */
					/* run ted. Ignore ^C's at start */
	signal(SIGINT, SIG_IGN);

	printf("\nULTRIX-11 System tty edit program\n");
	if (getuid() != 0) {
		fprintf(stderr, "\nted: must be super-user!\n\n");
		exit(1);
	}
	if (lockup() == FAILED) {			/* secure channels */
		fprintf(stderr, "Could not lock %s file.\n",ttyfile);
		exit(1);
	}
	if (whome() == FAILED) {		/* find user's terminal */
		fprintf(stderr, "Could not find user's terminal!\n");
		exit(1);
	}
	if (readttys() == FAILED) {		/* get ttys into list */
		fprintf(stderr, "Could not get information from %s file!\n", ttyfile);
		exit(1);
	}

	printf("\nFor instructions type 'help ted', then press <RETURN>\n");
#ifdef JUMP
	setjmp(mainbuf);
	jmpbuf = &mainbuf;
	signal(SIGINT, interr);
#endif

	newmode[2]=NULL;
prompt:
	clrregs();
	printf("\nCommand < help create modify remove print write exit >: ");

	switch (getcmd()) {
	case HELPTED:
		for(i=0;h_general[i];i++)
			printf("\n%s",h_general[i]);
		break;
	case HELP:
		for(i=0;h_opts1[i];i++)
			printf("\n%s",h_opts1[i]);
		break;
	case CREATE:
		crmenu();
		break;
	case MODIFY:
		modify++;
		do {
			printf("\nEnter the name of a terminal you wish to modify:");
			printf("\n\nModify < tty## >: ");
		} while (doable() >= 0);
		break;
	case REMOVE:
		removing++;
		do {
			printf("\nEnter the name of a terminal you wish to remove:");
			printf("\n\nRemove < tty## >: ");
		} while (doable() >= 0);
		break;
	case CTRLD:
	case EXIT:
		reallyquit();			/* ask if sure or not */
		break;
	case NOREPLY:
	case DIDPW:
		break;
	case BADCMD:
	default:
		printf(badent,line);
		break;
	}
		goto prompt;
}

/*
 * This creates new entrys.
 */
crmenu()
{
	int	i;

	for (;;) {
		clrregs();
		printf("\nCreate which type of terminal?\n");
		printf("\nCreate < disabled remote local nologins help >: ");

		switch(getcmd()) {
		case CTRLD:
			return;
		case DISABLE:
			do {
				strcpy(newmode,"00");
				clrregs();
				create++;
				disable++;
				printf("\nEnter the name of a disabled terminal you wish to create:");
				printf("\n\nCreate disabled terminal < tty## >: ");
			} while (doable() >= 0);
			break;
		case REMOTE:
			do {
				newmode[0]='1';
				clrregs();
				create++;
				printf("\nEnter the name of a remote terminal to create:");
				printf("\n\nCreate remote terminal < tty## > ");
			} while (doable() >= 0);
			break;
		case LOCAL:
			do {
				newmode[0]='2';
				clrregs();
				create++;
				printf("\nEnter the name of a local terminal to create:");
				printf("\n\nCreate local terminal < tty## > ");
			} while (doable() >= 0);
			break;
		case NOLOGIN:
			do {
				strcpy(newmode,"30");
				clrregs();
				create++;
				without++;
				printf("\nEnter the name of a local (no logins) terminal to create:");
				printf("\n\nCreate local (no logins) terminal  < tty## >: ");
			} while (doable() >= 0);
			break;
		case HELP:
			for(i=0;h_opts2[i];i++)
				printf("\n%s",h_opts2[i]);
			break;
		case NOREPLY:
			printf(getround);
		case DIDPW:
			break;
		case BADCMD:
		default:
			printf(badent,line);
			break;
		}
	}
}

doable()
{
	int	cmd;

	cmd = getcmd();
	if (cmd == CTRLD)
		return(-1);			/* quit the doable loop */
	else if (cmd == NOREPLY) {
		printf(getround);
		return(0);
	}
	else if (cmd == DIDPW)
		return(0);
	else if (checkok(line) == BADCMD)		/* error or something */
		return(0);
	strcpy(termname,line);
	if (findtty(termname) == NOTFOUND) {
		if (create) {
			if (disable || without)
				cent();
			else {
				do {
					printf("\nCreating entry: %s\n", termname);
					printf("\n\tEnter a single gettytab character:\n");
				} while(cent() == DIDPW);
			}
		}
		else {
			printf(notexist,termname);
			printf(sayprint);
		}
	}
	else {
		if (create) {
			printf("\nError:  %s%s already exists.",tmode,termname);
			printf(sayprint);
		}
		else if (removing) {
			if (deadlog(tmode,termname,removed) == FAILED)
				return(0);
			if (rement(termname) == YES) {
				printf("\n*** removed: ");
				pline(tmode,termname);
				printf("\n");
			}
		}
		else if (modify) {
			modifyit(termname);
		}
		else
			printf("\nImpossible flag set!\n");
	}
	return(0);
}

/*
 * Modify an entry - termname (tt)
 */
modifyit(tt)
char	*tt;
{
	int	i;

	for (;;) {
		printf("\nWhat would you like to modify about %s?",tt);
		printf("\n\nModify %s < mode speed name help >: ",tt);

		switch(getcmd()) {
		case CTRLD:
			return;
		case NAME:
			newname();
			break;
		case SPEED:
			newspeed();
			break;
		case MODE:
			newmod();
			break;
		case HELP:
			for(i=0;h_modify[i];i++)
				printf("\n%s",h_modify[i]);
			break;
		case NOREPLY:
			printf(getround);
		case DIDPW:
			break;
		case BADCMD:
		default:
			printf(badent,line);
			break;
		}
	}
}

/*
 * find a tty entry if it exists
 */
findtty(lf)
char	*lf;
{
	register struct llist *ptr;

	for(ptr = head; ptr != NULL; ptr = ptr->next) {
		if (strcmp(lf, ptr->tname) == 0) {		/* match! */
			strcpy(tmode,ptr->tmode);		/* save mode */
			return(1);
		}
	}
	if (ptr == NULL)
		return(NOTFOUND);	
}

/*
 * Read a line, decode the command, return command id
 */
getcmd()
{
	int	cc, cmd;
	register char *q,*s;

	if (fgets(line, LINELEN, stdin) == NULL) {
		printf("\n");
		return(CTRLD);
	}
	if (line[0] == '\n')
		return(NOREPLY);

	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = NULL;

	for(q=line; isspace(*q); q++)		/* strip leading blanks */
		;
	if (keepupper) {			/* preserving uppercase */
		length = strlen(q);
		strcpy(line, q);
		keepupper = 0;
	}
	else {
		for(s=line; *s = *q; q++) {	/* put to lowercase */
			if (isascii(*s) && isupper(*s))
				*s = tolower(*s);
			s++;
		}
		length = strlen(line);
	}
	cmd = parcmd();		/* return cmd_id */
	switch(cmd) {
		case H_CTRLC:
			printf(h_ctrlc);
			cmd = DIDPW;
			break;
		case H_CTRLD:
			printf(h_ctrld);
			cmd = DIDPW;
			break;
		default:
			break;
	}
	return(cmd);
}

/*
 * parse commands, returning cmd_id.
 */
parcmd()
{
	int	i;

	for (i=0; cmdtyp[i].cmd_name; i++) {
		if (strncmp(cmdtyp[i].cmd_name, line, length) == 0) {
			switch (cmdtyp[i].cmd_id) {
			case PRINT:
				printall();			/* print ttys */
				return(DIDPW);
				break;
			case WRITE:				/* do write */
				if (nowriteallowed) /* avoid multiple writes */
					return(DIDPW);
				if (deadhead == NULL)
					printf("\nNo new changes have been entered.\n");
				else {
					printf("\nThe following changes have been entered:\n");
					report();
				}
				return(DIDPW);			/* both cases */
			default:
				return(cmdtyp[i].cmd_id);	/* command id */
			}
		}
	}

	return(BADCMD);				/* reached end - no match */
}

/*
 * change the mode of a terminal.
 */
chmode(tt)
char	*tt;
{
	if (newmode[0] == tmode[0]) {
		printf(isalready,tt);
		pline(tmode,tt);
		printf("\n");
	}
	else {
		savsig = signal(SIGINT,SIG_IGN);
		strcpy(repterm,tt);
		if (repterm[0] != NULL)
			rep();
		else
			cre();
		signal(SIGINT,savsig);
		strcpy(tmode,newmode);
	}
}

/*
 * create an entry
 */
cent()
{
	int	cmd, retcode, i;

	/*
	 * Don't ask for gettytab char if withoutflag (No logins) (30)
	 * or disabledflag (00).
	 */

	if (without || disable) {
		savsig = signal(SIGINT,SIG_IGN);
		if (repterm[0] != NULL)
			rep();
		else
			cre();
		signal(SIGINT,savsig);
		return;
	}

	/*
	 * create entry with appropriate character
	 */
	else {
		for (;;) {	/* while NOREPLY, do 'till the cows come home */
			printf("\n\t< ? >: ");
			keepupper++;
			cmd = getcmd();
			keepupper=0;
			if (cmd == CTRLD) {
				printf("\nRequest Canceled.\n");
				return;
			} else if (cmd == DIDPW) {
				return(DIDPW);
			} else if (cmd == NOREPLY) {
				printf(getround);
				printf("\n\tEnter a single ");
				printf("gettytab character:\n");
			} else
				break;
		}

		/*
		 * Decode possible valid commands first 
		 */

		if ((retcode = whichchar(line)) == YES) {/* it is a valid char*/
			savsig = signal(SIGINT,SIG_IGN);
			if (repterm[0] != NULL) {
				if (newmode[1] == tmode[1]) {
					printf(isalready, termname);
					pline(tmode, termname);
					printf("\n");
				} else
					rep();
			} else
				cre();
			/* restore signal here before returning */
			signal(SIGINT,savsig);
			return;
		}

		/*
		 * decode cmd after checking gettytab character
		 * since single letters like 'n' or 'h' could be 
		 * valid gettytab entries.
		 */

		else if (cmd == HELP) {
			for(i=0;h_getch[i];i++)
				printf("\n%s",h_getch[i]);
			return(DIDPW);
		}
		else if (retcode == BADCMD) {
			printf("\nEntry must be a single character.\n",line);
			return(DIDPW);
		}
		else if (retcode == NOTFOUND) {
			printf("\n'%s' is not a valid gettytab character.\n",line);
			return(DIDPW);
		}
		else	/* (FAILED) - no /etc/gettytab or cannot open */
			return(FAILED);
	}
}

/*
 * replace a terminal, actually REPLACE entry, don't just remove
 * the old one, and create a new one like before. (else the order changes)
 */
rep()
{
	register struct llist *ptr, *prev;

	ptr = head;
	for (;;) {
		/* look for the old entry */
		if ((strcmp(ptr->tname, repterm)) == 0) {
			break;	
		}
		/* didn't find it yet, get the next one */
		prev = ptr;
		ptr = ptr->next;
		if (ptr == NULL) {
			printf("\nFatal: %s not found!\n",repterm);
			printf("Attempt to replace %s failed.\n",repterm);
			return(FAILED);
		}
	}
/*** The variables:
	old term: (tmode, repterm)
	new term: (newmode, termname)
***/
	/* replace the changed field(s), leaving the comment as is */

	/* if current mode in entry structure is same as 'old' mode,
	   AND if the new mode is different than existing one... */
	if ((strcmp(ptr->tmode, tmode) == 0)
	 && (strcmp(ptr->tmode, newmode) != 0)) {
		strcpy(ptr->tmode, newmode);
	}

	/* check same thing for name */
	if ((strcmp(ptr->tname, repterm) == 0)
		&& (strcmp(ptr->tname, termname) != 0)) {
		strcpy(ptr->tname, termname);
	}

	/* record entry in logfile */
	if (deadlog(tmode,repterm,changed) == FAILED)
		return;
	if (deadlog(newmode,termname,dot) == FAILED)
		return;

	printf("\n*** changed entry to: ");
	pline(newmode,termname);
	printf("\n");

/******************* The OLD way: (remove it and create a new one, since they used to be sorted in major/minor device order...)

	if (rement(repterm) == YES) {
		if (cement(newmode,termname, (char *) NULL) < 0)
			return;
		if (deadlog(tmode,repterm,changed) == FAILED)
			return;
		if (deadlog(newmode,termname,dot) == FAILED)
			return;
		printf("\n*** changed entry to: ");
		pline(newmode,termname);
		printf("\n");
	}
 *******************/
}

/*
 * create terminal
 */
cre()
{
	if (cement(newmode,termname, (char *) NULL) == FAILED)
		return;
	if (deadlog(newmode,termname,added) == FAILED)
		return;
	printf("\n*** new entry created: ");
	pline(newmode,termname);
	printf("\n");
}

/*
 * change existing tty name
 */
newname()
{
	int	cmd;

	for(;;) {
		printf("\nChanging the name of %s:",termname);
		printf("\n\nNew name < tty## >: ");

		switch(getcmd()) {
		case CTRLD:
			return;
		case NOREPLY:
			printf(getround);
			continue;
		case DIDPW:
			continue;
		default:
			if (checkok(line) == BADCMD)	/* error or 'help' */
				continue;
			else {
				if (findtty(line) == NOTFOUND) {
					if (t_stat(line) == NOTFOUND) {
						continue;
					}
					else
						break;
				}
				else {		/* can't use existing name */
					printf(exists, line);
					printf(sayprint);
					continue;
				}
			}
		}				/* end switch */
	break;
	}
	/* get here if valid name was entered */

	strcpy(repterm,termname);	/* termname is the present one */
	strcpy(termname,line);
	strcpy(newmode,tmode);
	savsig = signal(SIGINT,SIG_IGN);
	if (repterm[0] != NULL)
		rep();
	else
		cre();
	strcpy(tmode,newmode);
	signal(SIGINT,savsig);
	return;
}

/*
 * change existing tty speed
 */
newspeed()
{
	if ((tmode[0] == '0') || (tmode[0] == '3')) {
		printf("\nThe terminal is disabled.  You may change the speed");
		printf("\nof the terminal only after changing the mode to");
		printf("\nsomething other than 'disabled'.  Local (no logins)");
		printf("\nmeans the same as 'disabled' in this case.\n");
	}
	else {
		strcpy(newmode,tmode);
		strcpy(repterm,termname);
		do {
			printf("\nChanging entry: %s%s", tmode,termname);
			printf("\n\n\tEnter new gettytab character?\n");
		} while (cent() == DIDPW);
		strcpy(tmode,newmode);		/* copy new mode back to tmode*/
	}
}

/*
 * change existing tty mode
 */
newmod()
{
	int	i;

	for(;;) {
		strcpy(newmode,tmode);
		printf("\nEnter new terminal mode for %s?",termname);
		printf("\n\nNew mode < disabled remote local nologins help >: ");
		switch(getcmd()) {
		case CTRLD:
			return;
		case LOCAL:
			newmode[0]='2';
			chmode(termname);
			return;
		case DISABLE:
			newmode[0]='0';
			newmode[1]='0';
			chmode(termname);
			return;
		case REMOTE:
			newmode[0]='1';
			chmode(termname);
			return;
		case NOLOGIN:
			newmode[0]='3';
			newmode[1]='0';
			chmode(termname);
			return;
		case HELP:
			for(i=0;h_opts2[i];i++)
				printf("\n%s",h_opts2[i]);
			continue;
		case NOREPLY:
			printf(getround);
		case DIDPW:
			continue;
		case BADCMD:
		default:
			printf(badent,line);
			continue;
		}

	}
}

/*
 * read ttyfile into linked list.
 */
readttys()
{
	int	savflg, entlen;
	char tm1[XXLEN];	/* mode */
	char tm2[TTYXXLEN];	/* ttyxx part */
	char localbuf[LINELEN];	/* local buffer for comment, if any */
	char dummy[32];		/* dummy buffer, plenty big enough */
	char *p;		/* pointer */

	savflg = create;
	create = 0;
	if ((ttystrm = fopen(ttyfile, "r")) == NULL) {
		printf("\n");
		perror(ttyfile);
		return(FAILED);	
	}
	if (fgets(fline, LINELEN, ttystrm) == NULL) {
		fprintf(stderr, "\n%s file is null!\n", ttyfile);
		return(FAILED);		  /* should at least find console! */
	}
	do {
	/* sscanf stops on white-space, thus tm2 will not contain a comment. */
		sscanf(fline,"%2s%s",tm1,tm2);
		if (tm1[0] == '#') {	  /* entire line is a comment */
			tm1[0] = tm2[0] = '\0'; /* null the entry pointers */
			fline[strlen(fline)-1] = '\0'; /* get rid of '\n' */
			strcpy(localbuf, fline);
		} else {
		/*
		 * Check if something like space or tab follows the entry,
		 * in which case it is a comment.
		 */
		if (strncmp(tm2, "console", 7) == 0)
		    entlen=7;	/* length of "console" */
		else
		    entlen=5;	/* length of "ttyxx" */

		if (strlen(fline) > entlen+3) {
	/* next won't work if there is white space comments */
	/*	    sscanf(fline, "%*s", entlen+2, dummy);	 */
		    p = fline;
		    p = p + entlen + 2;	/* add 2 for leading 00 */
		    if (strlen(p) < 1)
			localbuf[0] = '\0';
		    else {
			strcpy(localbuf, p);
			localbuf[strlen(localbuf)-1] = '\0'; /* no <CR> */
		    }
		} else
		    localbuf[0] = '\0';

		}  /* end the else #comment */

		if (cement(tm1,tm2,localbuf) == FAILED)
			return(FAILED);		/* malloc error */

	} while (fgets(fline, LINELEN, ttystrm) != NULL);

	if (fclose(ttystrm) == EOF)
		printf("\nwarning: could not close %s.\n",ttyfile);
	create = savflg;
	return(0);
}

/*
 * append transaction to deadlist. 
 * return YES normal, return FAILED if malloc error.
 */
deadlog(tm,tn,ttype)
char *tm,*tn,*ttype;
{	
	register struct llist *ptr, *prev;

	if (deadhead != NULL)
	for (prev=deadhead;prev->next!=NULL;prev=prev->next);
	if ((ptr = malloc(sizeof(struct llist))) == NULL) {
		printf("\n\7Memory allocation error (1027: struct llist).\nCannot create another entry.\n");
		return(FAILED);
	}
	strcpy(ptr->tmode, tm);
	strcpy(ptr->tname, tn);
	strcpy(ptr->ttype, ttype);
	if (deadhead == NULL) {
		deadhead = ptr;
		deadhead->next = NULL;
	}
	else {
		ptr->next = prev->next;
		prev->next = ptr;
	}
	return(YES);
}

/*
 * clear dead list
 * if n=0, just clear the list;
 * if n=1, restore ttyslist by re-reading /etc/ttys;
 * also want to re-get the tmode for the current terminal.
 */
deadclr(n)
int n;
{
	register struct llist *ptr, *prev;

	if (deadhead == NULL)
		return;
	ptr = prev = deadhead;
	deadhead = NULL;
	do {
		prev = ptr;
		ptr = ptr->next;
		free(prev);
	} while (ptr != NULL);
	if (n == 1) {			   		/* put things back! */
		if (head == NULL)
			printf("\ninternal warning: head is null!\n");
		ptr = prev = head;
		head = NULL;
		do {					/* free the ttys list */
			prev = ptr;
			ptr = ptr->next;
			free(prev);
		} while (ptr != NULL);
		if (readttys() < 0) {			/* reread ttyfile */
			printf("Fatal: couldn't read %s file!\n",ttyfile);
		}
		/* restore tmode in case we were in the middle of something. */
		findtty(termname);	/* it has to be there! */
					/* tmode is filled in globally */
	}
}

/*
 * say which changes have been made so far.
 */
saywhich()
{
	register struct llist *ptr;

	printf("\n\t");
	for (ptr = deadhead; ptr != NULL; ptr = ptr->next) {
		printf("%s%s %s",ptr->tmode, ptr->tname, ptr->ttype);
		if ((strcmp(ptr->ttype, added) == 0) ||
		    (strcmp(ptr->ttype, removed) == 0) ||
		    (strcmp(ptr->ttype, dot) == 0))
			printf("\n\t");
	}
}

/*
 * Tell about existing terminals becoming disabled, or names changed...
 */
fersure()
{
	register struct llist *ptr;

	printf("\n\t");
	for (ptr = deadhead; ptr != NULL; ptr = ptr->next) {
		if (strcmp(ptr->ttype, added) == 0)
			printf("added:   ");
	 	else if (strcmp(ptr->ttype, removed) == 0)
			printf("removed: ");
		else if (strcmp(ptr->ttype, changed) == 0)
			printf("changed: ");
		else if (strcmp(ptr->ttype, dot) == 0)
			printf("     to: ");
		else
			printf("	");	/* should never happen */

		pline(ptr->tmode, ptr->tname);
		printf("\n\t");
	}
	deadclr(0);				/* clear deadhead list */
}

/*
 * write list of ttys to temp file and update the ttyfile
 * return 1 successful
 */
update()
{
	register struct llist *ptr;

	if ((tmpstrm = fopen(tmp0file, "w")) == NULL) {
		printf("\nError: cannot open %s\n", tmp0file);
		printf("\nChanges not saved!!!\n\n");
		return;	
	}
	printf("\nUpdating...\n");
	for (ptr = head; ptr != NULL; ptr = ptr->next) {
		if ((ptr->tmode != NULL) && (ptr->tname != NULL)) {
			fputs(ptr->tmode, tmpstrm);
			fputs(ptr->tname, tmpstrm);
		}
		if (ptr->comment != NULL)
			fputs(ptr->comment, tmpstrm);
		fputs("\n", tmpstrm);
	}
	if (fclose(tmpstrm) == EOF)
		printf("\nwarning: couldn't close %s\n",tmp0file);
	/*
	 * If a link/unlink fails, leave ttyfile as is,
	 * and announce a new file: /etc/tty.newXXXXXX
	 */
	if (link(ttyfile,midfile) < 0) {
		printf("\n");
		perror(ttyfile);
		cleanup(1);
		unlink(midfile);
		exit(0);
	}
	if (unlink(ttyfile) < 0) {
		printf("\n");
		perror(ttyfile);
		cleanup(1);
		unlink(midfile);
		exit(0);
	}
	if (link(tmp0file, ttyfile) < 0) {
		printf("\n");
		perror(tmp0file);
		cleanup(1);
		link(midfile,ttyfile);		/* restore ttys file */
		unlink(midfile);
		exit(0);
	}
	if (unlink(tmp0file) < 0) {
		printf("\n");
		perror(tmp0file);
		printf("\n\7warning: couldn't unlink %s\n",tmp0file);	
		return(1);
	}
	if (unlink(midfile) < 0) {
		printf("\n");
		perror(midfile);
		printf("\n\7warning: couldn't unlink %s\n",midfile);	
		return(1);
	}
	printf("\nNew changes saved in %s\n",ttyfile);
	return(1);
}

/*
 * print tty and mode
 */
pline(tmod,ttynum)
char *tmod, *ttynum;
{
	int	i;

	    printf("%s%s - ",tmod,ttynum);
	    sscanf(tmod,"%1d",&i);
	    switch(i) {
	    case 0:
		printf(dis);
		break;
	    case 1:
		printf(rem);
		break;
	    case 2:
		printf(loc);
		break;
	    case 3:
		printf(locdis);
		break;
	    default:
		printf(unk);
		break;
	    }
}

/*
 * removes an entry from the ttys file.
 */
rement(n)
char *n;
{
	register struct llist *ptr, *prev;

	ptr = head;
	for (;;) {
		if ((strcmp(ptr->tname, n)) == 0) {
			break;	
		}
		prev = ptr;
		ptr = ptr->next;
		if (ptr == NULL) {
			printf("\nFatal: %s not found!\n",n);
			printf("Attempt to remove %s failed.\n",n);
			return(FAILED);
		}
	}
	prev->next = ptr->next;		/* skip that one */
	if (strlen(ptr->comment) > 1)
		free(ptr->comment);	/* free the comment, if any */
	free(ptr);
	return(YES);
}
/*
 * test for device present in /dev
 * return NOTFOUND if not in /dev. (p points to ttyname)
 */
t_stat(p)
char *p;
{
	strcpy(testend,dev);
	strcat(testend,p);
	if (stat(testend, &statbuf) < 0) {
		printf("\n");
		perror(testend);
		printf(filenotfnd);
		return(NOTFOUND);
	}
	else
		return;
}

/*
 * put entry into ttys list in major/minor device order.
 * 	p is first 2 chars of ttys file, *p is NULL if only a comment.
 *	q is tty name (console, or ttyxx, *q is NULL if only a comment)
 *	s is ptr to comment, and *s is NULL if no comment.
 * return FAILED on malloc error.
 * return NOTFOUND if tty not in /dev
 */
cement(p,q,s)
char *p,*q,*s;
{	
	register struct llist *ptr, *prev;
	int	maj, min;
	int	badflg;				/* /dev/tty?? not found */

	ptr = prev = head;
	badflg = 0;

	strcpy(testend,dev);			/* "/dev/" */
	strcat(testend,q);			/* add "ttyxx" */
	if (stat(testend, &statbuf) < 0) {	/* then file not in /dev ! */
		fprintf(stderr, "\nwarning: ");
		perror(testend);
		fflush(stdout);
		maj = min = BADDEV;
		badflg++;
	} else {
		maj = major(statbuf.st_rdev);
		min = minor(statbuf.st_rdev);
	}

	/* find the end of the list */
	if (head != NULL) {
		do {
			prev = ptr;
			ptr = ptr->next;
		} while (ptr != NULL); 	/* find the end of the list */
	}

	/*
	 * only malloc a structure if at least p or q exist, or
	 * a comment (*s) is present
	 */
	 /* NOTE: p and q always point somewhere, if only to a null. */
	if ((*p && *q) || (s && *s)) {
	    if ((ptr = malloc(sizeof(struct llist))) == NULL) {
		printf("\n\7Memory allocation error (1315: struct llist).\nCannot create another entry.\n");
		return(FAILED);
	    }
	}

	if (badflg) {
		maj = BADDEV;
		min = BADDEV;
	}

	if ((*p != '\0') && (*q != '\0')) {
		ptr->maj = maj;
		ptr->min = min;
	} else {
	/*
	 * Use 0 here, even though it is legal.
	 * (we never look at it since it is a comment (#))
	 */
		ptr->maj = 0;
		ptr->min = 0;
	}

	if (*p != '\0')
		strcpy(ptr->tmode, p);
	else
		ptr->tmode[0] = '\0';

	if (*q != '\0')
		strcpy(ptr->tname, q);
	else
		ptr->tname[0] = '\0';

	if (s && *s) {
		if ((ptr->comment = malloc(strlen(s)+1)) == NULL) {
			printf("\n\7Memory allocation error (1346: strlen(comment) = %d).\nCannot create another entry.\n",strlen(s));
				return(FAILED);
			}
		strcpy(ptr->comment, s);
	}
	else {
		if ((ptr->comment = malloc(sizeof(char))) == NULL) {
			printf("\n\7Memory allocation error (1355: sizeof(char) = %d).\nCannot create another entry.\n", sizeof(char));
			return(FAILED);
		}
		ptr->comment[0] = '\0';
	}

	if (head == NULL) {
		head = ptr;
		head->next = NULL;
	} else {
		ptr->next = prev->next;
		prev->next = ptr;
	}
	return(1);
}

/*
 * get user's ttyname
 */
whome()
{
	char	*ttynum;

	if ((ttynum = ttyname(0)) == NULL) {
		printf("\n");
		perror(ttynum);
		printf("ted: cannot find your tty.\n\n");
		return(FAILED);	
	}
	strcpy(ourtty,&ttynum[DEVSIZ]);		/* save 'ttyxx' part */
	/*
	printf("You are on %s.\n",ourtty);
	*/
	return(0);

}

/*
 * print maj/min, tty, mode
 */
printall()
{
	int	cc;
	char line[LINELEN];
	register struct llist *ptr;
	int	pcnt;
	int	beatit=0;

#ifdef JUMP
	if (setjmp(printbuf)) {
		jmpbuf = savbuf;
		printf("\n");
		return;
	}
	savbuf = jmpbuf;
	jmpbuf = &printbuf;
#endif JUMP

	pcnt = STARTNUM;
	printf("\n%s",heading);
	for (ptr = head; ptr != NULL; ptr = ptr->next) {
		if (pcnt < SCREENFUL) {
			putone(ptr);
			pcnt++;
		}
		else for (;;) {				/* screenful */
			printf(pretfm);
			if (fgets(line, LINELEN, stdin) == NULL) {
				beatit++;
				break;
			}
			if (line[0] == '\n') {
				printf(heading);
				putone(ptr);
				pcnt = STARTNUM;
				break;
			}
			/* be able to say 'q' for quit */
			else if ((line[0] == 'q') || (line[0] == 'Q')) {
				beatit++;
				break;
			}
			else {
				printf(h_pretfm);
				continue;
			}
		}
		if (beatit)		/* break on CTRL/D */
			break;
	}
	printf("\n");
#ifdef JUMP
	jmpbuf = savbuf;
#endif
}

/*
 * print a line of maj/min	tty	mode
 * maj/min become ?/? if device not present in /dev
 */
putone(ptr)
struct llist *ptr;
{
/*	printf("\n"); */
	if (ptr->maj == BADDEV)
		printf("\n?\t?\t%s%s - %s",ptr->tmode, ptr->tname,dnp);
	else {
		if ((ptr->tmode[0] != '\0') && (ptr->tname[0] != '\0')) {
			printf("\n%d\t%d", ptr->maj, ptr->min);
			printf("\t");
			pline(ptr->tmode,ptr->tname);
		}
	}
	if ((strcmp(ptr->tname, ourtty)) == 0)
		printf(youarehere);
}

/*
 * Lock things up
 * Return FAILED if can't do it
 */
lockup()
{
	int	pid;				/* old pid, if any */

	if (chmod(ttyfile,0400) < 0) {
		printf("\n");
		perror(ttyfile);
		return(FAILED);
	}
	if ((lockstrm = fopen(lockfile, "r")) == NULL) {
		if(takeover() == FAILED)
			return(FAILED);
	}
	else {					/* lock file already exits */
		if (fgets(fline,LINELEN,lockstrm) == NULL) {
			unlink(lockfile);
			printf("\nCouldn't lock %s file.  Please try again.\n\n", ttyfile);
			return(FAILED);
		} else {
			sscanf(fline,"%d",&pid);
			if (kill(pid, SIGTERM) == -1) {		/*alreadydead*/
				if (takeover() < 0)
					return(FAILED);
			} else {
				printf("\nted:  %s is running.\n",progname);
				printf("\nOnly one user may run the %s program at once.\n",progname);
				return(FAILED);
			}
		}
	}
	mktemp(tmp0file);			/* create temp ttys file */
	mktemp(midfile);			/* create a temp link file */
	return(0);
}

/*
 * No lockfile around or process is dead so move in.
 */
takeover()
{
	int	pid;				/* current pid */
	char	spid[8];			/* string form of pid */

	if ((lockstrm = fopen(lockfile, "w")) == NULL) {
		printf("\n");
		perror(lockfile);
		printf("\nNo write permission for %s.\n",lockfile);
		return(FAILED);
	} else {				/* insert our pid */
		pid = getpid();
		sprintf(spid,"%d",pid);
		fputs(spid, lockstrm);
		if (fclose(lockstrm) == EOF)
			printf("\nwarning: couldn't close %s.\n",lockfile);
		return(0);
	}
}

/*
 * close files, put things back and update as necessary
 * unlock lock file, chmod the ttyfile, etc.
 */
cleanup(n)
int	n;
{
	signal(SIGINT, SIG_IGN);
	if (n ==1) {				/* ungraceful exit */
		printf("\7\7\7");
		printf("\nA copy of your latest changes are in %s",tmp0file);
		printf("\n\nTo save your changes, use: \n\n\t\"cp %s %s\"",tmp0file,ttyfile);
		printf("\n\nEXIT (link error)\n");
		if (chmod(ttyfile,0664) < 0) {
			printf("\n");
			perror(ttyfile);
		}
		return;
	}
	if (deadhead != NULL) {
		printf("\n\n\7The following changes are not saved:\n");
		report();
	}
	if (chmod(ttyfile,0664) < 0) {
		printf("\n");
		perror(ttyfile);
	}
	if (unlink(lockfile) < 0) {
		printf("\n");
		perror(lockfile);
	}
	return;
}

/*
 * ask about saving new changes 
 */
report()
{
	int	i;

	saywhich();
	nowriteallowed = 1;	/* disallow writes to pile up */
	for (;;) {
		printf("\nDo you wish to save these new changes?");
		printf("\n\nSave < yes no discard help >: ");

		switch(getcmd()) {
		case YES:
			if (update()) {
				sendhup++;
				fersure();
			}
			break;
		case NO:
			if (quiting)
				printf("\nNo changes saved.\n");
			else
				printf("\nOK.\n");
			break;
		case DISCARD:
			if (!quiting) {
				printf("\nChanges discarded.\n");
				printf("\nRereading %s\n",ttyfile);
				deadclr(1);
			}
			else
				printf("\nNo changes saved.\n");
			break;
		case HELP:
			for (i=0;h_decide[i];i++)
				printf("\n%s",h_decide[i]);
			saywhich();
			continue;
		case CTRLD:
			if (quiting) {			/* exit */
				printf("\7\7\nNew changes not saved !\n");
				break;
			}
		case NOREPLY:			/* else fall through */
			if (quiting)
				printf("\7");
			printf("\nPlease answer the question.\n");
		case DIDPW:
			continue;
		case BADCMD:
		default:
			printf(badent,line);
			continue;
		}	/* end switch */
		nowriteallowed=0;	/* reset flag */
		return;

	}	/* for loop */
}

clrregs()
{
	int	i;
	disable=without=removing=modify=create=changing=keepupper=quiting=0;
	for (i=0; i < TTYXXLEN; repterm[i++]=NULL);	/* zero out repterm */
}

/*
 * Checkok checks for a reasonable tty name, ie. 'tty##'.
 * return BADCMD 	if invalid entry
 * return   1		if it looks good
 */
checkok(p)
char *p;
{
	register char *q,*s;

	for(q=p; isspace(*q); q++)	/* remove leading blanks and tabs */
		;

	for(s=p; *s = *q; q++) {		/* put to lowercase */
		if (isascii(*s) && isupper(*s))
			*s = tolower(*s);
	s++;
	}

	length = strlen(p);
	strcpy(line, p);
	switch (parcmd()) {
		case HELP:
			printf(tty1help);
			printf(tty2help);
			printf(sayprint);
			return(BADCMD);
		case NOREPLY:
			return(NOREPLY);
		case DIDPW:
			return(BADCMD);
		default:			/* other normally valid ones */
			printf(badtty,line);
			return(BADCMD);
		case BADCMD:			/* but a bad command
					   	   might be a terminal name */
	/*
	 * We allow entrys like tty00, ttyd0, ttyp0
	 * Desire last two digits unique in ttyfile
	 * Hex digits are a possibility (ttyaf)
	 */

	if (strcmp(ourtty,p) == 0) {
		printf("\nYou can't change your own terminal. (%s)\n",ourtty);
		return(BADCMD);
	}
	if (length == 5) {
	 	for (q=p; q < &p[5]; q++)		/* alphanumeric test */
			if (!isalnum(*q)) {
				printf(badtty,line);
				return(BADCMD);
			}
		if (strncmp(p,"tty",3) == 0) {
			if ((create) && (t_stat(p) == NOTFOUND))
				return(BADCMD);
			return(1);
		}
	}
	else if (length > 5) {
		if (strcmp(p, "console") == 0) {
			printf("\nSorry, the console cannot be altered.\n");	
			return(BADCMD);
		}	
	}

	printf(badtty,line);		/* otherwise, bad entry */
	return(BADCMD);

	}	/* end switch */
}

/*
 * check for valid gettytab character
 * return BADCMD if too long, NOTFOUND if invalid char
 */
whichchar(q)
char *q;
{
	int	i;
	char	wline[LINELEN];

	/*
	 * Read valid getty chars from /etc/gettytab first time only.
	 * Just look for first character on a line followed by '|' .
	 */
	if (valid[0] == NULL) {	
		if ((gtabstrm = fopen(gettytab, "r")) == NULL) {
			printf("\n");
			perror(gettytab);
			return(FAILED);	
		}
		if (fgets(wline, LINELEN, gtabstrm) == NULL)
			return(FAILED);			/* file empty */
		i = 0;
		do {
			if (isascii(wline[0]) && (wline[1] == '|')) {
				valid[i++] = wline[0];
			}
		} while (fgets(wline, LINELEN, gtabstrm) != NULL);

		if (fclose(ttystrm) == EOF)
			printf("\nwarning: could not close %s.\n",gettytab);
	}

	if (q[1] == NULL) {			/* allow only 1 char entry */
		for (i=0; i<=VALID; i++) {
			if (valid[i] == q[0]) {
				newmode[1] = q[0];
				return(YES);
			}
			else if (i >= VALID) {
				return(NOTFOUND);
			}
		}
		return(NOTFOUND);		/* insurance */
	}
	else {
		return(BADCMD);
	}
}

/*
 * ask about quitting or not
 */
reallyquit()
{
	int	i, cmd;

	for(;;) {
		printf("\nDo you really want to quit? \n\n< yes no >: ");
		cmd = getcmd();
		if (cmd == HELP) {
			printf("\nIf you wish to quit, enter 'yes'.");
			printf("  Press <RETURN> or enter 'no' to continue.\n");
			continue;
		} else if ((cmd == YES) || (cmd == CTRLD)) {
			printf("\nExit");
			quiting++;
			cleanup(0);
			if (sendhup) {
				printf("\n");
				for (;;) {
					printf("\nDo you wish to notify init of the new changes to /etc/ttys?");
					printf("\n\nNotify init < yes no help >: ");
					cmd = getcmd();
					if (cmd == HELP) {
						for (i=0;h_sendhup[i];i++)
							printf("\n%s",h_sendhup[i]);
						continue;
					}
					else if (cmd == YES) {
						printf("\nkill -1 1");	
						kill(1, SIGHUP);
						break;
					}
					else if (cmd == NOREPLY)
						continue;
					else {
						printf("\7\7\n*** No hangup signal sent to init!");
						break;	/* includes CTRL/D */
					}
				}
			}
			printf("\n\n");
			exit(0);
		}
		else			/* if not YES or CTRL/D, assume NO */
			return;
	}

}

