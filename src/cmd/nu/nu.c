
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)nu.c	3.0	4/21/86";
/*	/etc/nu	->  New User Installation Program
 *
 *	Program to help system administrators manage login accounts
 *
 *    Originally written by:
 *	Brian Reid, Erik Hedberg, Jeff Mogul, Fred Yankowski
 *	-- Stanford University
 *
 *	This program was originally written by Fred Yankowski as an
 *	MS project in 1980.  Several years of experience at using
 *	it showed us lots of things that we would like it to do differently;
 *	Erik Hedberg added many changes.  In summer 1984 Brian Reid
 *	tore it all apart, adapted it to 4.2BSD, modified it to use
 *	the C library as much as possible (this was Fred's first attempt
 *	at a C program), added error checking and command-line parsing,
 *	rewrote code that didn't amuse him, and changed the structure
 *	of it so that it read in a configuration file instead of having
 *	having compile-time options. Jeff Mogul added the -d option.
 *	The man page is entirely by Brian Reid.
 *	
 *   Changes for ULTRIX-11:
 *	-  Add menu feature, instead of using command-line options
 *	-  Remove request queue scheme (save memory so program will fit)
 *	-  Add on-line help for all prompts
 *	-  Add modify mode selection after main menu
 *	-  Add SYSTEM V Environment question
 *	-  Add check to disallow system directories as $HOME
 *	-  Disallow removal of root account, and any system accounts
 *	-  Add 5 second delay before removing directory (echo command first)
 *
 *	John Dustin  8/12/85
 */
#ifdef DBM
#define ALIASES
#endif DBM

#define CONFIGFILE "/etc/nu.cf"		/* configuration info from here */

#ifdef ALIASES
#define	ALIASFILE  "/usr/lib/aliases"	/* sendmail alias file */
#endif ALIASES

#define DEFLINKFILE	"/etc/ptmp"	/* linkfile to use if not defined */
#define MAXSYMBOLS	100		/* limit of configuration symbols */
#define MAXGROUPS	100		/* limit of GroupHome symbols */

#define	NOREPLY		1
#define EXIT		2
#define	HELP		3
#define	ADD		4
#define	DELETE		5
#define	KILL		6
#define	MODIFY		7
#define	NO		8
#define	YES		9
#define	QUIT		10
#define	BADCMD		11
#define	CTRLD		12

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <grp.h>
#include <signal.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_LINE	250		/* line length buffer */
#define BUF_WORD	40		/* word length buffer */
#define TRUE		1		/* return(...) codes */
#define FALSE		0
#define	ERROR		1		/* exit(...) codes */
#define	OK		0

 /* DoCommand codes ... */
#define	FATAL		1		/* Errors during System calls exit */
#define	NONFATAL	0		/* Errors just print message */
#define SAFE		1		/* Safe to execute during debugging */
#define	UNSAFE		0		/* Must not execute while debugging */

#define NOT		!
#define MAXUID		32000		/* sentinel, no uid should be larger */
#define MAXMODS		0		/* max. modifications in session */
	/* MAXMODS=0 above is insignificant (each mod is a session) */

char *ctime(), *getlogin(), *crypt(), *StrV();
long time();
int IntV();
struct passwd *getpwent(), *getpwuid(), *getpwnam();
char *getpass();
struct group *getgrgid(), *getgrnam();
int	retval;	/* return value from Verified() routine */

struct passwd *pwd;	/* ptr to an entry of the passwd file */

/* This is a copy of the passwd structure, values are copied into this
    structure because getpw--- returns a pointer to a STATIC area that is
    overwritten on each call */
struct cpasswd {
    char    cpw_name[BUF_WORD];
    char    cpw_passwd[BUF_WORD];
    char    cpw_asciipw[BUF_WORD];	/* Extra field: Ascii password */
    int     cpw_uid;
    int     cpw_gid;
    char    cpw_group[BUF_WORD];	/* Extra field: Ascii group name */
    char    cpw_person[(BUF_WORD)*2];	/* person field could exceed BUF_WORD */
    char    cpw_dir[BUF_WORD];
    char    cpw_shell[BUF_WORD];
    char    cpw_env[BUF_WORD];		/* Extra field: SYSTEM V environment or not */
};
/* This structure is used to hold the configuration statements from nu.cf
   Note that no error checking of any kind is done! */

struct Symb {
    char   *SymbName;			/* Variable name goes here */
    char   *Svalue;			/* String argument goes here OR */
    int    ivalue;			/* Integer argument goes here */
};

struct Symb Symbols[MAXSYMBOLS];
long nsymbols = 0;

char	LINKFILE[BUF_WORD];		/* linkfile name, so don't have to
					  use StrV() */
/* Help messages.
 * 
 * The H_? defines are for answering help when
 * inside the YesNo routine.  This is so we don't
 * have to loop in the main program on the YesNo
 * part, just have YesNo reprompt.
 */

#define	H_0	0	/* basic message */
/*** 
char	H_0mess[]=
"\nIf you are not sure what to do, it is probably best to \njust use the default";
***/

#define	H_1	1	/* truncate login name to 8 letters? */
char	H_1mess[]=
"\nAll login names must be unique on this system, within the \nfirst 8 characters.  Is it acceptable to truncate the login name?\n";

#define	H_2	2	/* file exists, but not a dir, want to clobber it? */
char	H_2mess[]=
"\nIf you answer yes, this file will be replaced with a \ndirectory by the same name, which will become this user's \nnew login directory.\n";

#define	H_3	3	/* dir exists, want to clobber it? */
char	H_3mess[]=
"\nIf you answer yes, the contents of this directory will be \nremoved and the directory replaced with a new version for this \nuser.  This directory will then become that user's new login \ndirectory.";

#define	H_4	4	/* use dir, but not touch it's contents? */
char	H_4mess[]=
"\nIf you answer yes, the directory will not be touched.  No files \nwill be added or removed from the directory, however, the directory \nwill still be used as this user's new login directory.\n";

#define	H_5	5	/* that login shell file does not exist ok? */
char	H_5mess[]=
"\nThe login shell which you specified is not currently on \nthe system.  The user whose account you are setting up will \nnot be able to login since his login shell is missing.  If\nyou are sure that this is what you want, enter 'yes'.\n";

#define	H_6	6	/* are these values ok? */
/*** Original message:
char	H_6mess[]=
"\nEnter yes if the values shown above are correct, \notherwise you should enter no and start modifying this \nentry from the beginning again.";
***/
char	H_6mess[]=
"\nEnter yes if the values shown above are correct, \notherwise you should enter no to re-modify the entry.";

#define	H_7	7	/* wipe out this account entirely? */
char	H_7mess[]=
"\nAnswer yes if you wish to totally remove this user's \naccount from the system.  This user's files and home directory \nwill be removed using a command like \"rm -rf *\" in the user's \nhome directory.  This user's passwd entry in the /etc/passwd file \nwill be deleted.\n";

#define	H_8	8	/* do you want to delete this entry using NOLOGINS? */
char	H_8mess[]=
"\nAnswer yes if you wish to remove this user's home directory \nand files and dissallow any further logins.  The user's password\nwill be changed to \"NOLOGINS\" in the /etc/passwd file to insure \nhe is kept off the system and the entry retained for accounting\npurposes.\n";

#define	H_9	9	/* Set-up user for SYSTEM V Environment? */
char	H_9mess[]=
"\nA user's programming environment is changed using an environment \nvariable called \"PROG_ENV\".  When this variable is \"SYSTEM_FIVE\", \na SYSTEM V compatibility library is linked at compile time for each \nprogram which the user compiles.  The environment variable allows each \nuser to turn this SYSTEM V Compatibility feature on or off.  The default \nvalue for the PROG_ENV environment variable is \"ULTRIX\".";

int Helpcode = H_0;	/* contains (always) current help message code number */

/*
 * These message strings are for help messages
 * which occur inside the main program, and thus
 * don't require accompanying H_? codes.
 */
char	h_lognames[]=
"\nLogin names consist of up to 8 alphanumeric characters. \nEach login name must be unique to this system.  Login names \nare usually a user's initials, but some users like to use \ntheir last name or a nickname.  Enter <\CTRL/D> if you wish \nto skip this section and return to the main menu.\n";

char	h_passwd[]=
"\nPasswords consist of up to 8 alphanumeric characters, and are used \nto insure that only the real owner of this account is allowed to login \nunder that name.  The user can change the password at a later time with \nthe passwd(1) command.  If just you press <RETURN>, then this account \nwill be installed without a password.\n";

char	h_logdir[]=
"\nA login directory, also known as a home directory, is the \ndirectory where a user's personal files are kept.  By default, \nit is the directory where a user is placed when he first logs \non to the system.  Each user's login directory ends with the \nsame name as their login name, and is usually grouped under the \nsame directory tree with other users in his group.  Example login \ndirectories might include: /usr/users/glen, /usr/staff/rjm, or \n/usr/guest/pkb.\n";

char	h_actual[]=
"\nEnter the user's real name.  This will be used later by programs \nlike finger(1) to determine who this user is in real life.  See \nfinger(1) in The ULTRIX-11 Programmer's Manual for more information.\n";

char	h_shell[]=
"\nThe login shell is the program which is executed after each \nsuccessful login by each user.  This program can be \"/bin/csh\" to \nuse the C Shell, \"/bin/sh\" to use the Bourne Shell, or \"/bin/sh5\" \nto use the SYSTEM V Bourne Shell.  For more information see csh(1), \nsh(1), and sh5(1) in the ULTRIX-11 Programmer's Manual.\n";

char	h_mainmenu[]=
"\nChoose from the following: \n	add     -  add new user accounts to the system. \n	delete  -  remove a user from the system. \n	kill    -  wipe out a user's account entirely. \n	modify  -  modify an already existing /etc/passwd entry.\n";

char	h_previous[]=
"\nType '?' for help, or <CTRL/D> to backup to the previous question.\n";

/* Enter a uid number */
char	h_uid[]=
"\nUser id's are used to keep track of each user on the system, \nhence each uid must be unique.  Uid's must be greater than 0 and \nshould be greater than 10.  Most general user id's start at 100 \nand increase by 1 for each new user that is added to the system.";

char	h_gid[]=
"\nGroup id's are used to keep track of a group of users who, if \nworking on the same project, require access permission on a group \nof files.  Group id's are used when checking group file permissions, \nin the case where the owner file permissions have denied access.\nSee chmod(1) in The ULTRIX-11 Programmer's Manual for more information.\n";

char	h_delacct[]=
"\nThis section removes user accounts, but retains the deleted \nuser's passwd entry in the /etc/passwd file for accounting \npurposes.  The user's password is changed to \"NOLOGINS\", \nwhile his home directory and all of his files are removed. \n";

char	h_seealso[]=
"\n\nRead nu(8) in the ULTRIX-11 Programmer's Manual for more information.\n\n";

#ifdef ALIASES
/* Definitions for using dbm (to access alias database) */
typedef struct {
    char   *dptr;
    int    dsize;
}datum;

int dbminit();
datum fetch(), firstkey(), nextkey();
#define	dbm_buf_size 1024		/* dbm(3) guarantees this limit */

#endif ALIASES				/* End of dbm declarations */

char This_host[32];		/* Holds result of gethostname() */
struct topnode {
    int gid;			/* login group number */
    char *topnodename;		/* what comes after /usr */
};

int numtopnodes = 0;
struct topnode topnode[MAXGROUPS];/* to be filled in from nu.cf */

int incritsect;			/* Set to true when the program is in
				   the section involved with updating
				   the files and creating new files
				   and, hence, should not be
				   interrupted by ^C. */

int wasinterrupted;		/* Set to true if interrupt occurred
				   during a call to 'System' */

char cmd[BUF_LINE];		/* Buffer to construct commands in */
char editor[BUF_WORD];		/* person running this program */
FILE *logf;			/* File to log actions in */
struct stat statbuf;		/* for stat and lstat calls */

struct	cmdtyp {
	char	*cmd_name;	/* name of command */
	int	cmd_id;		/* command ID */
} cmdtyp[] = {
	"?",		HELP,
	"add",		ADD,
	"bye",		QUIT,
	"delete",	DELETE,
	"exit",		EXIT,
	"help",		HELP,
	"kill",		KILL,
	"modify",	MODIFY,
	"no",		NO,
	"quit",		QUIT,
	"yes",		YES,
	0,		BADCMD,
};

/* YesNo
 * Get a yes or no answer.  Returns TRUE=(yes), FALSE=(no).
 */
YesNo(def_ans)
char def_ans;
{
    int ans, done;

    done=FALSE;
    ans=0;
    while (NOT done) {
    ans=getcmd();

	switch(ans) {
	    case NOREPLY:
		done=TRUE;
		if (def_ans == 'y')
		    ans=TRUE;
		else
		if (def_ans == 'n')
		    ans=FALSE;
		break;
	    case YES:
		done=TRUE;
		ans=TRUE;
		break;
	    case NO:
	    case QUIT:
	    case EXIT:
		done=TRUE;
		ans=FALSE;
		break;
	    case HELP:		/* check if specific help is available */
		switch(Helpcode) {
		    case H_1:
			puts(H_1mess);
			break;
		    case H_2:
			puts(H_2mess);
			break;
		    case H_3:
			puts(H_3mess);
			break;
		    case H_4:
			puts(H_4mess);
			break;
		    case H_5:
			puts(H_5mess);
			break;
		    case H_6:
			puts(H_6mess);
			break;
		    case H_7:
			puts(H_7mess);
			break;
		    case H_8:
			puts(H_8mess);
			break;
		    case H_9:
			puts(H_9mess);
			break;
		    case H_0:	/* default help code (ie. no help at all) */
		    default:
			break;
		}
		break;	/* break case HELP: */
	    case BADCMD:
	    default:
	/*	printf("\nAnswer yes or no.");	*/
	/*	printf(" [%c]  ? ", def_ans);	*/
		break;
	}	/* end switch */

	if (NOT done) {
	    printf("\nAnswer yes or no");
	    printf(" [%c]  ? ", def_ans);
	}
    }
    Helpcode=H_0;	/* reset help code to default */
    return(ans);
}

/* UseDefault
 *	returns TRUE if NULL entered (meaning use default) or
 *	FALSE after reading text to use in place of default
 */
UseDefault(str, def)
char *str, *def;
{
    char temp[BUF_WORD];

    printf(" [%s] ", def);
    gets(temp);
    if (temp[0] == NULL) {
	if (str != def)
	    strcpy(str, def);
	return(TRUE);
    }
    else {
	strcpy(str, temp);
	return(FALSE);
    }
}

/* GetUserID
 *	returns the userid for the new user.  The routine scans the
 *	password file to find the highest userid so far assigned.
 *	The new userid is then set to be the one more than this value.
 *	This calculated default can be overridden, but the password
 *	file is searched to insure that the chosen userid will be
 *	unique.
 */
GetUserID()
{
    int maxuid, newuid, i;
    char def_ID[BUF_WORD], buf[BUF_WORD];

 /* scan the passwd file for the highest userid */
    setpwent();		/* rewind passwd file ptr */
    maxuid = 0;
    while ((pwd = getpwent ()) != NULL) {
	if (pwd->pw_uid > maxuid)
	    maxuid = pwd->pw_uid;
    }
    newuid = maxuid + 1;
    sprintf(def_ID, "%d", newuid);

    for (;;) {
	printf("\nUser id number?  (small integer) ");
	UseDefault(buf, def_ID);
	if ((strcmp(buf, "?") == 0 || (strcmp (buf, "help") == 0))) {
	    puts(h_uid);
	    continue;
	}
	for (i=0; buf[i] != NULL; i++) {
	    if ( NOT isdigit(buf[i])) {
		printf("\nSorry, user id must be an integer. (digits only)\n");
		break;
	    }
	}
	if (buf[i] != NULL)
	    continue;	/* means we hit "break;" above, due to a non-digit; */
	newuid = atoi(buf);
	if (newuid <= 0) {
	    printf("\nUserid must be > 0, and should be > 10.\n");
	    continue;
	}
	setpwent();	/* rewind password file search */
	if ((pwd = getpwuid(newuid)) == NULL)
	    return(newuid);
	else {
	    printf("\nUser id %d is already assigned to %s (%s)\n",
		    pwd->pw_uid, pwd->pw_name, pwd->pw_gecos);
	}
    }
}

/* GetGroup
 *	writes the group name in grpname and returns the numeric gid.
 *	A groupid can be entered as a number or the name for a
 *	group (as defined in /etc/group).  The getgr-- functions are
 *	used to peruse the group file.  Legal symbolic names are
 *	mapped into the corresponding groupid.  Input numeric
 *	groupid's are mapped into the corresponding symbolic name
 *	(if such exists) for verification.
 */
GetGroupID(group)
char *group;
{
    struct group *agrp;
    int gid;
    char buf[BUF_WORD], def[BUF_WORD];
    char hbuf[BUF_LINE], *hptr;
    int gcount;

 /* Get group name for IntV("DefGroup") */
    setgrent();
    if (agrp = getgrgid(IntV("DefGroup")))
	strcpy(def, agrp->gr_name);
    else
	strcpy(def, "unknown");

    for (;;) {
	printf("\nWhich user group? (name or number; 'l' for list) ");
	UseDefault(buf, def);
	if ((strcmp(buf, "?") == 0 || (strcmp (buf, "help") == 0))) {
	    puts(h_gid);
	    continue;
	}
	if ((strcmp(buf, "l") == 0 || (strcmp(buf, "list") == 0))) {
	    setgrent();
	    hptr = hbuf;
	    gcount = 0;
	    sprintf(hptr, "\nAvailable groups are:");
	    while ((agrp = getgrent()) != 0) {
		if ((gcount++) == 3) {
		    strcat(hptr, "\n\t\t     ");
		    gcount = 0;
		}
		strcat(hptr, "\t");
		strcat(hptr, agrp->gr_name);
	    }
	    endgrent();
	    puts(hptr);
	    continue;
	}

	if (isdigit(buf[0])) {
    /* presumably, a numeric groupid has been entered */
	    gid = atoi(buf);
	    setgrent();
	    if (agrp = getgrgid(gid))
		strcpy(buf, agrp->gr_name);
	    else
		strcpy(buf, "unknown");
	    printf("\nSelected groupid is %d (%s), OK?  [y] ", gid, buf);
	    if (YesNo('y')) {
		strcpy(group, buf);
		return(gid);
	    }
	}
	else {
    /* a symbolic group name has been entered */
	    setgrent();
	    if (agrp = getgrnam(buf)) {
		strcpy(group, agrp->gr_name);
		return(agrp->gr_gid);
	    }
	    else
		printf("\nSorry, '%s' is not a registered group name.\nFor a list of groups, type 'l' or 'list'.\n", buf);
	}
    }
}

/* MapLowerCase
 *     maps the given string into lower-case.
 */
MapLowerCase(b)
char *b;
{
    while(*b) {
	if (isascii(*b) && isupper(*b))
	    *b = tolower(*b);
	b++;
    }
}

HasBadChars(b)
char *b;
{
    while(*b) {
	if ((NOT isalpha (*b)) && (NOT isdigit (*b)) && (*b != '_'))
	    return(TRUE);
	b++;
    }
    return(FALSE);
}

/* GetLogName
 * 	Prompts for a login name and checks to make sure it is legal.
 *	The login name is returned, null-terminated, in "buf".
 */
GetLogName(buf)
char *buf;
{
    int done = FALSE;
#ifdef ALIASES
    int i, j;
    char *aptr, aname[dbm_buf_size];
    datum aliaskey, aliasname;
#endif ALIASES

    while (NOT done) {
	printf("\nLogin name?  ");
	if (gets(buf) == NULL) {	/* <CTRL/D> */
	    printf("\n");
	    return(CTRLD);
	}
	if (buf[0] == '')		/* entered CR only */
		continue;
	if (buf[0] == '?') {		/* help */
	    puts(h_lognames);
	    continue;
	}
	MapLowerCase(buf);
	if (HasBadChars(buf)) {
	    printf("\nSorry, the login name can contain only alphanumerics or '_'\n");
	    continue;
	}

	if (strlen(buf) > IntV("MaxNameLength")) {
	    printf("\nSorry, login names must not contain more than ");
	    printf("%d characters.\n", IntV("MaxNameLength"));
	    buf[IntV("MaxNameLength")] = 0;
	    printf("\nShould it be truncated to '%s'? ", buf);
	    Helpcode=H_1;
	    if (NOT YesNo('y')) {
		printf("\n");
		continue;		/* start over again */
	    }
	}

 /* check to see that the login is unique */
	setpwent();
	if (pwd = getpwnam(buf)) {
	    printf("\nSorry, the login '%s' is already in use (id %d, %s).\n",
		    buf, pwd->pw_uid, pwd->pw_gecos);
	    continue;
	}

#ifdef ALIASES
 /* check to make sure that the login does not conflict with an alias. This
    whole section of code is ridiculous overkill, but I was in the mood
    (BKR). */
	aliaskey.dptr = buf;
	aliaskey.dsize = strlen(buf) + 1; /* char count includes the null */
	aliasname = fetch(aliaskey);
	if (aliasname.dptr != NULL) {
	    printf("\nSorry, the name '%s' is already in use as a mail alias\n(aliased to '", buf);
	    if (aliasname.dsize > dbm_buf_size-2)
		aliasname.dsize = dbm_buf_size-2;

	    aptr = aliasname.dptr;
	    for (i = 0; (aptr[i] & 0177) == ' ' || (aptr[i] & 0177) == '\t'; i++);

	    if ((aptr[i] & 0177) == '"' && (aptr[aliasname.dsize-2] == '"')) {
		i++;			/* Unquote quoted names */
		aptr[aliasname.dsize-2] = 0;
		aliasname.dsize--;
	    }

	    for (j = 0; i < aliasname.dsize; i++, j++) {
		aname[j] = aptr[i] & 0177;
		if (i < 60 - strlen(buf))
		    putchar(aname[j]);
	    }
	    putchar('\'');
	    aname[j] = 0;
	    if (aliasname.dsize >= 60 - strlen (buf))
		printf("...");
	    puts(")");
printf("\nThis conflict must be resolved before you can create an\n");
printf("account named '%s'.\n", buf);

    /* 
     Try to figure out what the alias entry is. We do this because the person
     running nu is often an administrative assistant and not a programmer.
     */
	    if (aname[0] == '|') {	/* It's a program */
printf("\nThe current alias is a program, automatically run whenever\n");
printf("mail is sent to '%s'.  It is almost certainly a bad idea to\n",buf);
printf("delete this alias, so you should pick a different login name.\n");
	    }

	    else
		if (aname[0] == '/') {	/* A log file */
printf("\nThe current alias is a log file.  All mail sent to '%s'\n",buf);
printf("is automatically added to the end of '%s'.  You can\n",aname);
printf("probably negotiate with its owner to change from '%s'\n",buf);
printf("to a new name, but it is probably easier to just select a\n");
printf("different login name\n");
		}

		else {			/* alias or list */
		    if (index(aname, ',')) {/* mailing list */
printf("\nThe current alias is a mailing list, enabling people to send\n");
printf("mail to '%s@%s' and have it reach a group of people.\n", buf, This_host);
printf("To know whether it is safe to delete or rename that list,\n");
printf("you need to learn who uses it (the users are not necessarily\n");
printf("on this machine).  In this case, it is probably best to just\n");
printf("select a different login name.\n");
		    }

		    else {		/* alias entry */
			if (index(aname, '@') || index(aname, '!')) {
							/* network alias */
printf("\nThe current definition is a network mail alias for a\n");
printf("user who does not have a login on %s.  When somebody\n", This_host);
printf("sends mail to '%s@%s', it is forwarded to '%s'\n", buf, This_host,aname);
printf("However, if the %s login '%s' that you are currently\n", This_host, buf);
printf("trying to create is in fact for '%s', you probably\n",aname);
printf("want to leave this forwarding entry in place and create\n");
printf("the account anyhow.\n");
printf("\nDo you want to go ahead and create the\n");
printf("account '%s', knowing that its mail will\n",buf);
printf("be forwarded to '%s'?  [y] ",aname);
			    if (YesNo('y')) {
				done = TRUE;
				break;
			    }
			}
			else {		/* local alias */
printf("\nThe current definition is a local nickname or spelling\n");
printf("correction for '%s'.  It is probably ok to delete\n",aname);
printf("the nickname, but there might be people who are accustomed\n");
printf("to sending mail to '%s' instead of to '%s'.\n",buf, aname);
printf("It is probably best to just select a different login name.\n");
			}
		    }
		}
	    puts("\n");		/* puts will add a second newline */
	}
	else
#endif ALIASES
	    done = TRUE;
    }
    return(OK);
}

/* GetPassword
 *	read password (null is allowed)
 */
char *
GetPassword(ascpw, cryptpw)
char *ascpw, *cryptpw;
{
    char saltc[2], c;
    long salt;
    register int i;
    char pw1[40], pw2[40];

    do {
	strcpy(pw1, getpass("\nEnter password: "));
	if ((strlen(pw1) < 2) && (pw1[0] == '?')) {
	    puts(h_passwd);
	    continue;
	}
	strcpy(pw2, getpass("Retype password, please: "));
	if (strcmp(pw1, pw2) == 0)
	    break;
	printf("\nThey don't match. Please try again.\n");
    } while(TRUE);

    strcpy(ascpw, pw1);
    if (strlen(ascpw)) {
	time(&salt);
	salt += getpid ();
	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
	for (i=0; i < 2; i++) {
	    c = saltc[i] + '.';
	    if (c > '9')
		c += 7;
	    if (c > 'Z')
		c += 6;
	    saltc[i] = c;
	}
	strcpy(cryptpw, (crypt(ascpw, saltc)));
    }
    else
	strcpy(cryptpw, "");
}

/* GetRealName
 *	read the new user's actual name.
 */
GetRealName(buf)
char *buf;
{
    int done;
    done = FALSE;
    while (NOT done) {
	printf("\nEnter actual user name: ");
	if (gets(buf) == NULL) {
	    printf("\n");
	    return(CTRLD);	/* user typed ^D, back up */
	}
	if (buf[0] == '?') {
	    puts(h_actual);
	    continue;
	}
	if (index(buf, ':'))
	    printf("\nSorry, the name must not contain ':'\n");
	else
	    done = TRUE;
    }
    return(OK);
}

/* GetLogDir
 *	Writes the new user's login directory in the cpw_dir field.
 *	return OK if allright, else unknown.  Nothing checks return
 *	value from here anyway.
 */
int DontClobberDir;	/* keep this global, so everyone can see it */
int
GetLogDir(np)
struct cpasswd *np;
{
    register int i,done;
    char defdir[BUF_WORD];		/* default login directory */
    char templdir[BUF_WORD];		/* temp. login directory - for checks */
    char myolddir[BUF_WORD];	/* saved version of dir, coming in */
    char *q, *p;

    /*
     * lastDCD: previous value of DontClobberDir, gets
     * restored if new dir is same as old dir */
    int lastDCD;

    /*
     * Save old information:
     */
    lastDCD=DontClobberDir;
    strcpy(myolddir, np->cpw_dir);
    strcpy(defdir, StrV("DefHome"));
    strcat(defdir, "/");

    for (i=0; topnode[i].gid; i++) {
	if (np->cpw_gid == topnode[i].gid) {
	    strcpy(defdir, topnode[i].topnodename);
	    strcat(defdir, "/");
	    break;
	}
    }
    strcat(defdir, np->cpw_name);

    done = FALSE;
    while (NOT done) {
	printf("\nLogin directory? ");
	UseDefault(np->cpw_dir, defdir);
	if (np->cpw_dir[0] == '?') {	/* help */
	    puts(h_logdir);
	    continue;
	}
	if (index(np->cpw_dir, ':')) {
	    printf("\nSorry, the name must not contain ':'\n");
	    continue;
	}
	DontClobberDir = 0;

	if (baddir(np->cpw_dir) < 0)
		continue;

	if (stat(np->cpw_dir, &statbuf) == 0) {
	    /*
	     * stat succeeded, file exists; check if regular or directory
	     */
	    if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
		printf("\nThe file '%s' already exists, but is not a directory.\n", np->cpw_dir);
		printf("\nDo you want to clobber it?  [n] ");
		Helpcode=H_2;
		if (YesNo('n')) {	 /* entered yes */
		    if (unlink(np->cpw_dir) < 0) {
			printf("\nCouldn't clobber '%s'.  Please remove the file if it is \nnot important, or else enter a different login directory.\n", np->cpw_dir);
		    } else {
			done=TRUE;		/* stop next time */
		    }
		}
		continue;
	    } else {
		printf("\nThe directory '%s' already exists.\n", np->cpw_dir);
		printf("\nDo you want to clobber it?  [n] ");
		Helpcode=H_3;
		if (YesNo('n')) {	/* yes */
		    done=TRUE;		/* stop */
		    continue;
		}
		printf("\nDo you want to use directory '%s', but not\ntouch its contents?  [y] ", np->cpw_dir);
		Helpcode=H_4;
		if (NOT YesNo('y'))	/* no, go again */
		    continue;
		/*
		 * reach this point if we are using existing directory,
		 * 	(leaving existing files intact)
		 */
		DontClobberDir = 1;
		done=TRUE;		/* stop */
	    }
	} else {
	    DontClobberDir = 0;		 /* default */
	    /*
	     * Home directory does not exist, so we need to
	     * check and see if directory looks reasonable,
	     * ie, starts with '/'.  Also check that initial
	     * path exists.  If login dir is /usr/guest/jsd
	     * make sure /usr/guest is a directory.
	     */
	    strcpy(templdir, np->cpw_dir);	/* get login dir into temp loc*/
	    if (templdir[0] != '/') {
		printf("\nSorry, the login directory must begin with '/'.\n");
		continue;	/* go again */
	    /* else name does begin with a '/'  */
	    } else {
		p = index(templdir, '/');
		q = rindex(templdir, '/');
		if (q == p) {
		    done = TRUE;
		    continue;
	    	} else {	/* found morethan one '/', check initial path */
 		    *q++ = '\0';		/* get rid of last '/' */
		}
		/* make sure it is a directory */
		if (stat(templdir, &statbuf) == 0) {
		/* leading directory path does exist... */
		    if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
printf("\nSorry, leading directory path '%s' exists, but it\n", templdir);
printf("is not a directory.  Please remove this file and make it\n");
printf("a directory, or else enter a different login directory.\n");
			continue;	/* go again */
		    } else {
		    /* leading directory path is a directory, good! */
		    }
		}
		else {
printf("\nSorry, leading directory path '%s' does not exist.\n",templdir);
printf("You must create directory '%s' first in order to\n",templdir);
printf("use '%s' as a login directory.\n", np->cpw_dir);
		    continue;		/* go again */
		}
	    }
	}
	/* if get to here, then things are OK */
	done=TRUE; 	/* name reasonable */
    }
    if (strcmp(np->cpw_dir, myolddir) == 0) {
        /*
         * they kept the old dir, so want
         * to keep old DontClobberDir.
         */
	 DontClobberDir=lastDCD;	/* restore original value */
    }
    return(OK);
}

/* GetEnv
 *	Returns 1 for Use SYSTEM V Programming Environment.
 *	otherwise, 0.
 *	The default, which may be overridden, is "ULTRIX".
 */
GetEnv(buf)
char *buf;
{
	printf("Set up for SYSTEM V environment  [n] ? ");
	Helpcode=H_9;
	if (YesNo('n')) {
	    printf("\n");
	    strcpy(buf, "yes");
	    return(1);
	} else {
	    printf("\n");
	    strcpy(buf, "no");
	    return(0);
	}
}

/* GetLogSH
 *	returns the new user's login shell directory.  The default,
 *	which may be overridden, is StrV("DefShell").
 */
GetLogSH(buf)
char *buf;
{
    int done;

    done = FALSE;
    while (NOT done) {
	printf("\nEnter shell");
	UseDefault(buf, StrV ("DefShell"));
	if ((strcmp(buf, "?") == 0 || (strcmp (buf, "help") == 0))) {
	    puts(h_shell);
	    continue;
	}
	if (access(buf, 0) == -1) {	/* was fopen() which never closed */
	    printf("\nThe file '%s' does not currently exist.  ", buf);
	    printf("Are you sure \nthis is correct?  [n] ");
	    Helpcode=H_5;
	    if (NOT YesNo('n'))
		continue;
	}
	if (index (buf, ':'))
	    printf("\nSorry, shell file name must not contain ':'\n");
	else
	    done = TRUE;
    }
    printf("\n");
}

/* System
 *	
 *	Like the regular "system", but does the right thing with ^C
 */
System(cmdstring)
char *cmdstring;
{
    int status, pid, waitstat;

    wasinterrupted = FALSE;

    if ((pid = fork()) == 0 ) {
	execl("/bin/sh", "sh", "-c", cmdstring, 0);
	fprintf(stderr, "\nexecl /bin/sh sh -c '%s' failed.\n", cmdstring);
	perror("execl");
	_exit(127);
    }
    if (pid == -1) {
	return(-1);
    }
    while ((waitstat = wait(&status)) != pid && waitstat != -1)
	;
    if (waitstat == -1)
	status = -1;
    return(status);
}

/* CallSys
 *	repeats a System call until the call returns without error,
 *	or until it returns with an error but without the flag being
 *	set indicating that an interrupt occurred during the call.
 *	This flag, 'wasinterrupted', is set FALSE just before a
 *	System call, and is set to TRUE only if the Catch routine is
 *	called during the critical (uninterruptable) section of the
 *	program.
 */
CallSys(cmd)
char *cmd;
{
    register int status;

    while (status = System(cmd))
	if (NOT wasinterrupted)		/* regular system error */
	    return(status);		/* system call bombed */
    return(OK);			/* executed without problem */
}

/* DoCommand
 *	calls 'Callsys' to execute the string 'cmd', and supplies
 *	some messages.  Exits if 'fatal' is TRUE.
 */
DoCommand(cmd, fatal, safe)
char *cmd;
int fatal, safe;
{
    register int status;

    if (IntV("Debug") != 0)
	;
    if (IntV("Debug") == 0 || safe) {
	if ((status = CallSys(cmd)) != 0) {
	    printf("\nnu: '%s' failed, status %d\n", cmd, status);
	    if (fatal)
		leave(ERROR);
	}
    }
    else {
	printf("\nUnsafe command, skipped in Debug mode.\n");
	fflush(stdout);
    }
}

/* AddToPasswd
 *	takes buf to hold the new entry for the passwd file, and
 *	inserts this new entry at the end of the file.
 */
AddToPasswd(buf)
char *buf;
{
    FILE *pwfile;

    printf("\nAdding entry to passwd file:\n\n%s\n", buf);

    if ((pwfile = fopen(StrV("PasswdFile"), "a")) == NULL) {
	printf("\nError: cannot open %s\n", StrV("PasswdFile"));
	leave(ERROR);
    }
    else {
	fprintf(pwfile, "%s", buf);
    }
    fclose (pwfile);
}

/* LogAddition
 *	creates an entry for a file that holds information about
 *	newuser's, corresponding to the information in the passwd
 *	file.  This file is used only informally, to keep track of
 *	recent additions.
 */
LogAddition(buf)
char *buf;
{
    FILE *logf;
    long clock;

    if (logf = fopen(StrV("Logfile"), "a")) {
	clock = time(0);
	fprintf(logf,"%s\tadded by %s on %s\n",buf,editor,ctime(&clock));
	fclose (logf);
    }
    else
	fprintf(stderr, "\nCannot open log file '%s'\n", StrV("Logfile") );
}

/* LogRemoval
 *	creates an entry for a file that holds information about
 *	changed passwd entries.  This file is used only informally,
 *	to keep track of recent changes.
 */
LogRemoval(buf)
char *buf;
{
    FILE *logf;
    long clock;

    if (logf = fopen(StrV("Logfile"), "a")) {
	clock = time(0);
	fprintf(logf,"\n%s \tremoved by %s on %s\n",buf, editor,ctime(&clock));
	fclose (logf);
    }
    else
	fprintf(stderr, "\nCannot open log file '%s'\n", StrV("Logfile") );
}

/* CreateDir
 *	Calls a shell script that creates a new directory
 *	which will be the new user's login directory.
 *	This new user becomes the owner of the directory.
 */
CreateDir(np, clobber)
struct cpasswd *np;
{
    struct stat sb;	/* to see if clobbering a real directory */

    if (IntV("Debug") == 0) { 	/* NOT in debug mode */
	if (clobber == 1) {
	    if (oktorm(np->cpw_dir) < 0) {
		return(FALSE);
	    }
	    if (stat(np->cpw_dir, &sb) < 0)
		;	/* OK, file/directory not found */
	    else {
	        printf("\nDeleting old directory (\"rm -rf %s\" is next...)\n",
	 	    np->cpw_dir);
	        sleep(5);	/* time to catch it */
	        printf("\n");
	        sleep(2);	/* plus a couple more...  ;-) */
	    }
	}
    } else {
	    printf("\n*** DEBUG MODE ***  not doing the real commands:\n\n");
    }

    sprintf(cmd, "%s %d %d %s %d %d\n",
	    StrV("CreateDir"),
	    np->cpw_uid,
	    np->cpw_gid,
	    np->cpw_dir,
	    clobber,
	    IntV("Debug")
	);
    DoCommand(cmd, FATAL, SAFE);
}

/* InstallFiles
 *	Call the shell script that puts files in the new directory
 *	and makes sure they have the right ownership.
 */
InstallFiles(np)
struct cpasswd *np;
{
    printf("\n");
    sprintf(cmd, "%s %d %d %s %d %s\n",
	    StrV("CreateFiles"),
	    np->cpw_uid,
	    np->cpw_gid,
	    np->cpw_dir,
	    IntV("Debug"),
	    np->cpw_env
	);
    DoCommand(cmd, NONFATAL, SAFE);

}
/* PwPrint
 *	Print the fields of a passwd structure.
 */
PwPrint(cpw)
struct cpasswd *cpw;
{
    printf("   1)  login ...... %s\n", cpw->cpw_name);
    printf("   2)  password ... ");
    if (cpw->cpw_passwd[0]) {
	if (NOT strcmp(cpw->cpw_passwd, "NOLOGINS"))
	    printf("%s (no logins allowed)\n", cpw->cpw_passwd);
	else
	    printf("%s (encrypted)\n", cpw->cpw_passwd);
    }
    else
	printf("(none)\n");
    printf("   3)  name ....... %s\n", cpw->cpw_person);
    printf("   4)  userid ..... %d\n", cpw->cpw_uid);
    printf("   5)  groupid .... %d (%s)\n", cpw->cpw_gid, cpw->cpw_group);
    printf("   6)  login dir .. %s\n", cpw->cpw_dir);
    printf("   7)  login sh ... %s\n", cpw->cpw_shell[0] ? cpw->cpw_shell
	    : "/bin/sh (default)");
}

/* Verified
 *  print the current account data, ask if it is OK
 *  Returns:
 *	TRUE if data is acceptable,
 *	FALSE if we wish to continue changing this entry,
 *	-1 otherwise.
 */
Verified(np)
struct cpasswd *np;
{
/*
 *  Clear all waiting input and output chars.
 *  Actually we just want to clear any waiting input chars so
 *  we have a chance to see the values before confirming them.
 *  We have to sleep a second to let waiting output chars print.
 */
    sleep(1);
    ioctl(0, TIOCFLUSH, 0);

    PwPrint(np);

    printf("\nAre these values OK?  [y] ");
    fflush(stdout);
    Helpcode=H_6;
    if (YesNo('y')) {
	return(TRUE);	/* yes, values are OK */
    } else {
	printf("\nDo you wish to continue with this entry?  [y] ");
	if (YesNo('y'))
	    return(FALSE);  /* yes, they wish to continue */
	else
	    return(QUIT);
    }
}

/* PasswdLocked
 *	attempts to get exclusive access to the passwd file.  By
 *	convention, any program that wants to write to the passwd
 *	file will try to create a link with the name 'LINKFILE'.  If
 *	such a link already exists, link() returns -1, indicating
 *	that someone else is currently writing to the passwd file.
 */
PasswdLocked () {
    if (creat(StrV("Dummyfile"), 0) < 0) {
	fprintf(stderr, "\ncannot create %s\n",StrV("Dummyfile"));
	return(-1);
    }

    if (IntV("Debug"))
	return(0);
    else {
	strcpy(LINKFILE, StrV("Linkfile")); /* if Linkfile comes back ok, then
					    use it, else we will have exited. */
	return(link (StrV("Dummyfile"), LINKFILE));
    }
}

/* Catch
 *	is called whenever ^C is typed at the terminal.  If the
 *	critical section flag is set, the program should not be
 *	aborted, and so the routine just returns.  If the flag is not
 *	set, the terminate routine is called to halt the program.
 */
Catch()
{
    signal(SIGINT, SIG_IGN);		/* ignore ^C's for a short time */

    if (incritsect) {
	printf("\n/etc/nu is in a critical section.  You can't quit now!\n");
	wasinterrupted=TRUE;
	signal(SIGINT, Catch);
	return(TRUE);
    }
    else {
	signal(SIGINT, Catch);
	printf("\n");
/*	return;			*/
	leave(OK);
    }
}

/* Additions
 *	This is the driver routine for adding new users.
 */
Additions()
{
    int done;
    char buffer[BUF_LINE];
    struct cpasswd new;

#ifdef ALIASES
    dbminit(ALIASFILE);
#endif ALIASES
    done = FALSE;	/* done adding users */

    printf("\nThis section adds new user accounts to your system.\n");
    printf("For help on any of the questions, enter '?'.\n");

    while (NOT done) {
	if (GetLogName(new.cpw_name) == CTRLD) {
	    done = TRUE;
	    continue;
	}
	GetPassword(new.cpw_asciipw, new.cpw_passwd);
	if (GetRealName(new.cpw_person) == CTRLD) {
	    done = TRUE;
	    continue;
	}
	new.cpw_uid = GetUserID();
	new.cpw_gid = GetGroupID(new.cpw_group);
	GetLogDir(&new);
	GetLogSH(new.cpw_shell);
	GetEnv(new.cpw_env);

    while(1) {
	if ((retval = Verified(&new)) == TRUE) {
	    sprintf(buffer, "%s:%s:%d:%d:%s:%s:%s\n",
		    new.cpw_name, new.cpw_passwd, new.cpw_uid, new.cpw_gid,
		    new.cpw_person, new.cpw_dir, new.cpw_shell);

	    incritsect = TRUE;		/* should not be interrupted */
	    AddToPasswd(buffer);
	    if (DontClobberDir == 1) {
		CreateDir(&new, 0);	/* preserve existing directory */
	    }
	    else {
		CreateDir(&new, 1);	/* clobber dir with new one */
		InstallFiles(&new);
	    }
	    LogAddition(buffer);
	    incritsect = FALSE;
	    break;	/* break the while(1) since we got by OK */

	} else if (retval == FALSE) {	/* continue with this entry */
	    /*
	     * NEW OPTION HERE: they aren't happy
	     * with the entry, so allow them to modify it.
	     */
	    if (domods(&new) == OK)
	        continue;	/* while(1) */
	/*  else fall through to generic 'break' below */
	}
	/*
	 * Generic case for bad input, abort this entry.
	 */
	 break;

    }	/* end while(1), changes */

    printf("\nDo you wish to add more new users?  [y] ");
    done = NOT YesNo('y');

    }	/* end while(NOT done) */
}

/* 
 * Do some modifications on a newly added entry if
 * they didn't like the entry.  This is always called
 * from and always returns to Additions() above.
 *
 * Return value:
 *	OK if OK,
 *	QUIT if CTRL/D was entered (although this is not looked at)
 *
 * ent is later verified for correctness, and a chance to change yet again.
 */
domods(ent)
struct cpasswd *ent;
{
    char reply[BUF_WORD];
    int done = FALSE;

    while (NOT done) {
	printf("\nEntry is now:\n");
	PwPrint(ent);

	printf("\nSelect field to be modified ");
	printf("(1 2 3 4 5 6 7  q [quit] )  : ");

	if (gets(reply) == NULL) {
	    printf("\n");	/* ^D entered */
	    return(QUIT);
	}
	switch(*reply) {
	    case '1': 			/* get new login */
		GetLogName(ent->cpw_name);
		break;
	    case '2': 			/* get new password */
		GetPassword(ent->cpw_asciipw, ent->cpw_passwd);
		break;
	    case '3': 			/* get new name */
		GetRealName(ent->cpw_person);
		break;
	    case '4':			/* change userid */
		ent->cpw_uid = GetUserID();
		continue;
	    case '5': 			/* get groupid */
		ent->cpw_gid = GetGroupID(ent->cpw_group);
		break;
	    case '6': 			/* get login directory */
		GetLogDir(ent);
		break;
	    case '7': 			/* get login shell */
		GetLogSH(ent->cpw_shell);
		break;
	    case 'Q':
	    case 'q': 			/* done */
		printf("\n");		/* gets verified later */
		return(OK);
		break;
	    case '?': 			/* help */
		printf("\nEnter the number of the field you wish to modify,\n(e.g. enter '2' to change a user's password).\n");
		puts(h_previous);
		break;
	    default:
		if (reply[0] != '\n')
		    printf("\nSorry, '%s' is not a valid choice. \n", reply);
		puts(h_previous);
	}
    }
}

/* Xfer
 *	copies the values from a 'passwd' structure (which
 *	is static in a system 'getpw---' routine) into a
 *	'cpasswd' structure.  This is done so that multiple
 *	'passwd' entries can be saved.
 */
Xfer(pwd, cpw)
struct passwd  *pwd;
struct cpasswd *cpw;
{
    struct group *agrp;

    strcpy(cpw->cpw_name, pwd->pw_name);
    strcpy(cpw->cpw_passwd, pwd->pw_passwd);
    cpw->cpw_asciipw[0] = 0;
    cpw->cpw_uid = pwd->pw_uid;
    cpw->cpw_gid = pwd->pw_gid;
    if (agrp = getgrgid(pwd->pw_gid))
	strcpy(cpw->cpw_group, agrp->gr_name);
    else
	strcpy(cpw->cpw_group, "unknown");
    strcpy(cpw->cpw_person, pwd->pw_gecos);
    strcpy(cpw->cpw_dir, pwd->pw_dir);
    strcpy(cpw->cpw_shell, pwd->pw_shell);
}

/* PromptForID
 *	queries the user interactively for the identifier of
 *	an entry in /etc/passwd.  If the ID is numeric, it
 *	is assumed to be the userid; otherwise, it is assumed
 *	to be the login name. 
 *	A pointer to a structure holding the 'passwd' entry is returned.
 *	The routine will not terminate until a valid entry is found, or
 *	the user types a CTRL/D, (then it returns a NULL)
 */
struct passwd *
PromptForID()
{
    char resp[BUF_WORD];
    int theuid, done;
    struct passwd  *pwd;

    done = FALSE;
    while (NOT done) {
	printf("\nEnter user identifier (login or uid): ");
	if (gets(resp) == NULL) {
	    printf("\n");
	    return(NULL);
	}
	if (resp[0] == '?') {	/* help? */
	    puts(h_uid);
	    puts(h_lognames);
	    continue;
	}
	if (isdigit(*resp)) {
    /* presumably, a uid has been entered */
	    theuid = atoi(resp);
    /* search passwd for entry with uid = theuid */
	    setpwent();
	    if ((pwd = getpwuid(theuid)) == NULL)
		printf("\nSorry, that uid is not in use\n");
	    else
		done = TRUE;
	}
	else {
    /* if the entry is sensible, it is a login-name */
	    setpwent();
	    if ((pwd = getpwnam(resp)) == NULL)
		printf("\nSorry, that login is not in use\n");
	    else
		done = TRUE;
	}
    }
    return(pwd);
}

/* Get Mod
 *	Asks user to identify a particular passwd entry, prints
 *	the entry, prompts for changes to the entry, and leaves
 *	'ent' pointing to an appropriately modified copy of the entry.
 *	Returns TRUE if changes were made and they are to be saved,
 *	   else QUIT if no changes were made.
 */
GetMod(ent)
struct cpasswd *ent;
{
    struct passwd  *pwd;
    char reply[BUF_WORD];
    int needID = TRUE;
    int did_change = 0;	/* was anything really changed? */
    int nextent = 0;	/* proceed to next entry */

top:
    while (1) {
	if (needID) {
	/* If don't need id, then user is still in the process of
	 * changing the last entry; if we need an ID, then check
	 * if the last id was really a valid change.  Return TRUE
	 * if valid change, hence we are done and can go on to the
	 * next one, or QUIT if quitting, or CTRLD if aborting.
	 */
	    if (did_change) {	/* this must be the first one checked */
		/*
		 * A valid change was entered at some point,
		 * so we break to 'Q' & 'q' code below.
		 */
		break;
	    }
	    if (nextent) {
		return(QUIT);
	    }
	    if ((pwd = PromptForID()) == NULL) {
		return(QUIT);
	    }

	    Xfer(pwd, ent);		/* Copy to local storage */
	    needID = FALSE;
	}

	printf("\nEntry is now:\n");
	PwPrint(ent);
	printf("\nSelect field to be modified ");
	printf("(1 2 3 4 5 6 7  q [quit] )  : ");

	if (gets(reply) == NULL) {
	    printf("\n");	/* ^D entered */
	    needID = TRUE;
	    nextent++;
	    continue;
	}
	switch(*reply) {
	    case '1': 			/* get new login */
		if (GetLogName(ent->cpw_name) == CTRLD) {
		    needID=TRUE;
		    nextent++;
		} else
		    did_change++;
		continue;	/* while (1) */

	    case '2': 			/* get new password */
		GetPassword(ent->cpw_asciipw, ent->cpw_passwd);
		did_change++;
		continue;	/* while (1) */

	    case '3': 			/* get new name */
		if (GetRealName(ent->cpw_person) == CTRLD) {
		    needID=TRUE;
		    nextent++;
		} else
		    did_change++;
		continue;	/* while (1) */

	    case '4':
		printf("\nSorry, you cannot change the user id number.\n");
		continue;	/* while (1) */

	    case '5': 			/* get groupid */
		ent->cpw_gid = GetGroupID(ent->cpw_group);
		did_change++;
		continue;	/* while (1) */

	    case '6': 			/* get login directory */
		GetLogDir(ent);
		did_change++;
		continue;	/* while (1) */

	    case '7': 			/* get login shell */
		GetLogSH(ent->cpw_shell);
		did_change++;
		continue;	/* while (1) */

	    case 'Q':
	    case 'q': 			/* done */
		needID = TRUE;
		nextent++;
		break;	/* this is the only true break in the switch */

	    case '?': 			/* help */
		printf("\nEnter the number of the field you wish to modify,\n(ie. enter '2' to change a user's password).\n");
		puts(h_previous);
		continue;	/* while (1) */

	    default:
		if (reply[0] != '\n')
		    printf("\nSorry, '%s' is not a valid choice. \n", reply);
		puts(h_previous);
		continue;	/* while (1) */
	}
	break;	/* only Q or q breaks to here */
    }

    /* Broke the while(1) loop above, which means we quit
     * through normal channels (using 'q' above).
     * Now need to verify that what they entered
     * for changes are acceptable.
     */
    if ((retval = Verified(ent)) == TRUE) {
	return(TRUE);  /* they wish to save these changes */
    } else if (retval == FALSE) {  /* they wish to continue */
	needID = FALSE;
	goto top;  /* continue with this same entry... */
    } else {
	return(QUIT);
    }
}

/* Linearize
 *	takes a 'cpasswd' structure and converts it into the proper
 *	form for insertion into the passwd file.
 */
char *
Linearize(p)
struct cpasswd *p;
{
    static char buff[BUF_LINE];
    sprintf(buff, "%s:%s:%d:%d:%s:%s:%s\n", p->cpw_name,
	    p->cpw_passwd, p->cpw_uid, p->cpw_gid, p->cpw_person,
	    p->cpw_dir, p->cpw_shell);
    return(buff);
}

/* SwapFiles
 *	copies the current passwd file onto the backup file.  The
 *	modified version of the passwd file is then copied over
 *	the current version.  The temporary modified version is
 *	then removed.
 */
SwapFiles(FromWhat)
char *FromWhat;
{
    sprintf(cmd, "cp %s %s\n", FromWhat, StrV("Backupfile"));
    DoCommand (cmd, FATAL, SAFE);

    sprintf(cmd, "cp %s %s\n", StrV("Tempfile"), FromWhat);
    DoCommand(cmd, FATAL, SAFE);

    sprintf(cmd, "rm %s\n", StrV("Tempfile"));
    DoCommand(cmd, FATAL, SAFE);
}

/* Update_Passwd
 *	merges the modified passwd entry with the current
 *	version of the passwd file to create the new version.
 *	Changes to the passwd file are logged in "Logfile".
 */
UpdatePasswd(stk)
struct cpasswd *stk;
{
    FILE *tempf;
    struct passwd  *pwd;
    struct cpasswd  cpw_buf;
    long clock;

    tempf = fopen(StrV ("Tempfile"), "w");
    logf = fopen(StrV ("Logfile"), "a");

    setpwent();				/* rewind passwd file ptr */
    while (pwd = getpwent()) {
	Xfer(pwd, &cpw_buf);
	if (cpw_buf.cpw_uid == stk->cpw_uid) {

    /* substitute new version of passwd entry in /etc/passwd file */
	    fprintf(tempf, "%s", Linearize(stk));

    /* make an entry in the log file */
	    fprintf(logf, "%s\tchanged to\n", Linearize(&cpw_buf));
	    fprintf(logf, "%s", Linearize(stk));
	    clock = time(0);
	    fprintf(logf, "\tby %s on %s\n", editor, ctime(&clock));
	}
	else {
	/* otherwise uid didn't match, so write original passwd entry back */
	    fprintf(tempf, "%s", Linearize(&cpw_buf));
	}
    }
    fclose(tempf);
    fclose(logf);
    endpwent();

 /* Save the old passwd file and replace it with the new version */
    SwapFiles(StrV("PasswdFile"));
}

struct cpasswd mods;	/* the modified entry (this is global) */
/*
 * Modify a passwd entry
 */
Modify()
{
#ifdef ALIASES
    dbminit(ALIASFILE);
#endif ALIASES

    printf("\nThis section modifies passwd entries.  Enter '?' for help.\n");

    if ((retval = GetMod(&mods)) == TRUE) {
        incritsect = TRUE;
        UpdatePasswd(&mods);
        incritsect = FALSE;
        printf("done.");
    }
    printf("\n");
}

/* KillUser - Kill user accounts entirely
 *	Asks user to identify a particular passwd entry,
 *	prints the entry, prompts to double check if entry
 *	should be deleted, removes the password entry and
 *	user's home directory (ie: "rm -rf $HOME/*").
 */
KillUser()
{
    struct passwd *pwd;	/* struct used by 'getpw--' routines */
    struct cpasswd ent;
    char cmd[BUF_LINE];
    int done=FALSE;

    printf("\n");
    printf("This section DESTROYS a user's account entirely.\n");

    while (NOT done) {
	if ((pwd = PromptForID()) == NULL) {
	    done=TRUE;		 /* entered ^D */
	    continue;
	}
	Xfer(pwd, &ent);	/* Copy to local storage */
	printf("\nEntry to be destroyed is:\n");
	PwPrint(&ent);
	printf("\nDestroy this user's account entirely?  [y] ");
	Helpcode=H_7;
	if (NOT YesNo('y')) {
	    printf("\nNothing changed.\n");
	}
	else {
	    if (IntV("Debug") == 0) {
		if (oktorm(&ent.cpw_dir) < 0) {
		    return(FALSE);
		}
		/* WARNING */
		printf("\nDestroying %s (\"rm -rf %s\" is next...)\n", ent.cpw_name,ent.cpw_dir);
		sleep(5);	/* time to catch it */
		printf("\n");
		sleep(2);	/* plus a couple more...  ;-) */
	    } else {
		printf("\n*** DEBUG MODE ***  not doing the real commands:\n\n");
	    }
	    sprintf(cmd, "%s %s %s %s %d\n",
		StrV("KillAccts"),
		ent.cpw_name,
		ent.cpw_dir,
		StrV("Logfile"),
		IntV("Debug")
		);
	    DoCommand(cmd, FATAL, SAFE);
	    if (IntV("Debug") == 0) {
		LogRemoval(ent.cpw_name);	 /* record in the LOGFILE */
	    }
	    printf("\nDo you wish to destroy any more accounts?  [y] ");
	    done = (NOT YesNo('y'));
	}
    }
    printf("\n");
    return(0);
}

/*
 * Delete accounts: Do what KillUser() does but don't erase
 * /etc/passwd entries so accounting information is available.
 * Also, work interactively; structurally similar to Modify(),
 * except that it doesn't postpone updates, so it can bomb out
 * if an error occurs.
 */
Del_Accounts()
{
	struct cpasswd del;	/* structure we are deleting */
	int done;

	puts(h_delacct);
	done = FALSE;
#ifdef ALIASES
	dbminit(ALIASFILE);
#endif ALIASES

	while (NOT done) {
	    if (GetDel(&del) == 1) {
		incritsect = TRUE;
		UpdatePasswd (&del);
		incritsect = FALSE;
	    }
	    if (NOT done) {
		printf("\nDo you wish to delete any more users?  [y] ");
		done = NOT YesNo ('y');
	    }
	}
}

/* GetDel
 *	asks user to identify a particular passwd entry, prints
 *	the entry, prompts to check if entry should be deleted,
 *	modifies the password field if so, and leaves
 *	'ent' pointing to an appropriately modified copy of the
 *	entry.  Returns false if no deletion is wanted.
 *	Returns NULL on ^D (and then returns to main menu)
 */
GetDel(ent)
struct cpasswd *ent;
{
	struct passwd *pwd;	/* struct used by system 'getpw--' routines */
	char delcmd[BUF_LINE];

	if ((pwd = PromptForID()) == NULL) {
	    return(NULL);
	}
	Xfer(pwd, ent);		/* Copy to local storage */
	printf("\nEntry is now:\n");
	PwPrint(ent);

	if (NOT strcmp(ent->cpw_passwd, "NOLOGINS")) {
	    printf("\nThe account '%s' has already been deleted.\n", ent->cpw_name);
	    return(FALSE);
	}
	printf("\nDo you want to delete this entry?  [y] ");
	Helpcode=H_8;
	if (NOT YesNo('y')) {
	    return(FALSE);	/* no deletion */
	}
	if (IntV("Debug") == 0) {
	    if (oktorm(ent->cpw_dir) < 0) {
		return(FALSE);
	    }
	    /* WARNING */
	    printf("\ndeleting %s (\"rm -rf %s\" is next...)\n",
		 ent->cpw_name,ent->cpw_dir);
	    sleep(5);	/* time to catch it */
	    printf("\n");
	    sleep(2);	/* plus a couple more...  ;-) */
    	} else {
	    printf("\n*** DEBUG MODE ***  not doing the real commands:\n\n");
	}
	sprintf(delcmd, "%s %s %s %s %d\n",
		StrV("DeleteAccts"),
		ent->cpw_name,
		ent->cpw_dir,
		StrV("Logfile"),
		IntV("Debug")
		);
	DoCommand(delcmd, FATAL, SAFE);

	strcpy(ent->cpw_asciipw, "[no logins]");
	strcpy(ent->cpw_passwd, "NOLOGINS");

	return(1);
}

/* 
  This procedure reads in the nu.cf configuration
  file and uses its contents to initialize various
  tables and configuration variables.
*/
ReadCf()
{
    FILE *cfile;
    char lbuf[BUF_LINE];
    char mylbuf[BUF_LINE];	/* local buffer to hold the entire line */
    char *cp, *op, *name, *sv;
    long iv;
    int i, istat;
    int DefGHasHome = 0;
    if ((cfile = fopen(CONFIGFILE, "r")) == NULL) {
	fprintf(stderr, "\nnu: cannot open configuration file: %s.\n",
		CONFIGFILE);
	exit(1);
    }
    strcpy(LINKFILE, DEFLINKFILE);	/* set up LINKFILE before anything
					   goes wrong and we have to exit.
					   This way, we know it is defined. */

    while (fgets(lbuf, BUF_LINE, cfile) != NULL) {
	sv = name = (char *) 0;
	iv = 0;
	op = (char *) 0;
	strcpy(mylbuf, lbuf);	/* save the whole line for error messages */
	for (cp=lbuf; *cp != 0; cp++) {
	    switch(*cp) {
		case '\t': 
		case ' ': 
		    *cp = 0;
		    continue;
		case ';': 
		    goto exitforloop;
		case '=': 
		    if ((name = (char *) malloc(cp - op + 2)) == NULL) {
			printf ("\nmalloc: out of space! Too many definitions in %s!\n", CONFIGFILE);
			printf("nu: choked on line:\n%s\n",mylbuf);
			return (ERROR);
		    }
		    *cp = 0;
		    strcpy(name, op);
		    cp++;
		    while (*cp == ' ' || *cp == '\t')
			cp++;

		    if (strcmp(name, "GroupHome") == 0) {
			iv = atoi(cp);		/* was atol */
			for (; *cp != '"' && *cp != 0; cp++)
			    ;
			cp++;
			for (op=cp; *cp != '"' && *cp != 0; cp++)
			    ;
			if ((sv = (char *) malloc(cp - op + 2)) == NULL) {
			    printf ("\nmalloc: out of space! Too many definitions in %s!\n", CONFIGFILE);
			    printf("nu: choked on line:\n%s\n",mylbuf);
			    return(ERROR);
		    	}
			*cp = 0;
			strcpy(sv, op);
			if (numtopnodes + 1 < MAXGROUPS) {
			    topnode[numtopnodes].gid = (int) iv;
			    topnode[numtopnodes].topnodename = sv;
			    topnode[numtopnodes + 1].gid = 0;
			} else {
			    printf("Too many GroupHomes, ignoring: %s",mylbuf);
			}
			numtopnodes++;
			if (iv == IntV("DefGroup"))
			    DefGHasHome = 1;
		    }
		    else {
			if (*cp == '"') {
			    cp++;
			    for (op=cp; *cp != '"' && *cp != 0; cp++);
			    if ((sv = (char *) malloc(cp - op + 2)) == NULL) {
			printf ("\nmalloc: out of space! Too many definitions in %s!\n", CONFIGFILE);
			        printf("nu: choked on line:\n%s\n",mylbuf);
				return (ERROR);
			    }
			    *cp = 0;
			    strcpy(sv, op);
			}
			else {
			    iv = atoi(cp);	/* was atol */
			}
			if (nsymbols < MAXSYMBOLS) {
			    Symbols[nsymbols].SymbName = name;
			    Symbols[nsymbols].Svalue = sv;
			    Symbols[nsymbols].ivalue = iv;
			} else {
			    printf("Too many symbols, ignoring: %s",mylbuf);
			}
			nsymbols++;
		    }
		    goto exitforloop;
		default: 
		    if ((int) op == 0)
			op = cp;
		    continue;
	    }
	}
exitforloop:
	    continue;
    }
    if (numtopnodes == 0) {
	fprintf(stderr, "\nnu: GroupHome information not specified in %s", CONFIGFILE);
	fprintf(stderr, h_seealso);
	exit(1);
    }
    if (numtopnodes >= MAXGROUPS) {
	fprintf(stderr, "\nnu: %s defines %d GroupHomes; the limit is %d.\n", CONFIGFILE, numtopnodes, MAXGROUPS - 1);
	fprintf(stderr, "    You must specify fewer GroupHome declarations in %s.", CONFIGFILE);
	fprintf(stderr, h_seealso);
	exit(1);
    }
    if (nsymbols > MAXSYMBOLS) {
	fprintf(stderr, "\nnu: %s defines %d symbols; the limit is %d.\n", CONFIGFILE, nsymbols, MAXSYMBOLS);
	fprintf(stderr, "    You must specify fewer symbol declarations in %s.", CONFIGFILE);
	fprintf(stderr, h_seealso);
	exit(1);
    }
    for (i = 0; topnode[i].gid; i++) {
	istat = stat(topnode[i].topnodename, &statbuf);
	if (istat != 0) {
	    fprintf(stderr,"\nnu: a GroupHome declaration names %s as the home\n", topnode[i].topnodename);
	    fprintf(stderr,"    directory for group %d, but that directory does not exist.\n", topnode[i].gid);
	    fprintf(stderr,"    Please create directory %s first.", topnode[i].topnodename);
	    fprintf(stderr, h_seealso);
	    exit(1);
	}
	if (NOT ((statbuf.st_mode) & S_IFDIR)) {
	    fprintf(stderr,"\nnu: a GroupHome declaration names %s as the home\n", topnode[i].topnodename);
	    fprintf(stderr,"    directory for group %d, but %s is not a directory.\n", topnode[i].gid, topnode[i].topnodename);
	    fprintf(stderr,"    Please change %s to be a directory, or else fix\n", topnode[i].topnodename);
	    fprintf(stderr,"    the GroupHome specification in %s.", CONFIGFILE);
	    fprintf(stderr, h_seealso);
	    exit(1);
	}
    }

    if (NOT DefGHasHome) {
	fprintf(stderr, "\nnu: %s defines DefGroup=%d, but there is no\n", CONFIGFILE, IntV("DefGroup"));
	fprintf(stderr, "    GroupHome declaration for group %d.  Be careful.\n", IntV("DefGroup"));
    }
}

/* IntV and StrV return integer and string values that were defined in
   the configuration file CONFIGFILE. */
int
IntV(name)
char *name;
{
    int j;
    for (j=0; j <= nsymbols; j++) {
	if (strcmp(Symbols[j].SymbName, name) == 0)
	    return((Symbols[j].ivalue));
    }
    notdef(name);
    /*NOTREACHED*/
}

char *
StrV(name)
char *name;
{
    int j;
    for (j=0; j <= nsymbols; j++) {
	if (strcmp(Symbols[j].SymbName, name) == 0)
	    return((char *) (Symbols[j].Svalue));
    }
    notdef(name);
    /*NOTREACHED*/
}

notdef(na)
char *na;
{
    fprintf(stderr, "\nnu: no definition for %s in %s.  Please fix %s.", na, CONFIGFILE, CONFIGFILE);
    fprintf(stderr, h_seealso);
    leave(ERROR);
}

/*
 *	M A I N   P R O G R A M
 */

main(argc, argv)
int argc;
char *argv[];
{
    char *p;
    char *def_luh="local_unknown_host";
    struct passwd *pwd;

    if (argc != 1)
	usage();
    incritsect = FALSE;
    signal(SIGINT, Catch);		/* catch ^C's */
    if (gethostname(This_host, 32) < 0) {
	strcpy(This_host, def_luh);
	printf("Warning: cannot get system hostname, using hostname=\"%s\".",
		def_luh);
    }
/*  printf("nu 3.3 [10 Oct 1984] (%s:%s)\n", This_host, CONFIGFILE); */
    if (ReadCf() == ERROR) {
	printf("Cannot get complete information from %s.  ",CONFIGFILE);
	printf("You have defined \ntoo many symbols in your configuration ");
	printf("file.  You must define \nfewer symbols in %s and then ",
		CONFIGFILE);
	printf("try running /etc/nu again.\n");
	leave(ERROR);
    }

    if (IntV("Debug")) {
	printf(">>> Debugging Mode (no dangerous system calls) Debugging Mode <<<\n");
    }
    else {
	if (geteuid()) {
	    printf("\nnu: must be superuser!\n\n");
	    leave(ERROR);
	}
    }
#ifdef DEBUG
    pr_them();	/* print out symbols and values, can tell right away if
		 getting any malloc errors because they won't all print out! */
#endif DEBUG

    if (PasswdLocked()) {
	printf("\nPassword file is locked - Try again later (see vipw(8))\n");
	leave(ERROR);
    }
    if (p = getlogin())
	strcpy(editor, p);
    else {
	pwd = getpwuid(getuid());
	if (pwd)
	    strcpy(editor, pwd->pw_name);
	else
	    strcpy(editor, "Unknown");
    }
    if (argc != 1)
	usage();
    else for (;;) {	/* get commands until exit called */
	printf("\nUser Accounts: <add delete modify kill help exit> ? ");
	    switch (getcmd()) {
		case ADD:	/* add new accounts */
		    Additions();
		    break;
		case MODIFY:	/* modify existing accounts */
		    Modify();
		    break;
		case DELETE:	/* delete existing accounts */
		    Del_Accounts();
		    break;
		case KILL:	/* kill (purge) old accounts */
		    KillUser();
		    break;
		case EXIT:
		case QUIT:
		    printf("\n");
		    leave(OK);
		    break;
		case HELP:
		    puts(h_mainmenu);
		    /* falls through to next one, case NOREPLY: */
		case NOREPLY:
		default: 	/* default covers 'bad' commands too */
		    puts(h_previous);
		    break;
	    }
    }
    leave(OK);
}

#ifdef DEBUG
pr_them()
{
    int j;

    	printf("Backupfile 	%s\n", StrV("Backupfile") );
	printf("CreateDir 	%s\n", StrV("CreateDir") );
	printf("CreateFiles 	%s\n", StrV("CreateFiles") );
	printf("DefHome 	%s\n", StrV("DefHome") );
	printf("DefShell 	%s\n", StrV("DefShell") );
	printf("DeleteAccts 	%s\n", StrV("DeleteAccts") );
    	printf("Dummyfile 	%s\n", StrV("Dummyfile") );
	printf("KillAccts 	%s\n", StrV("KillAccts") );
	printf("Linkfile 	%s\n", StrV("Linkfile") );
    	printf("Logfile 	%s\n", StrV("Logfile") );
    	printf("PasswdFile 	%s\n", StrV("PasswdFile") );
    	printf("Tempfile	%s\n", StrV("Tempfile") );
	printf("-----------\n");
    	printf("DefGroup	%d\n",	IntV("DefGroup"));
	printf("MaxNameLength	%d\n",	IntV("MaxNameLength"));
    	printf("Debug		%d\n",	IntV("Debug"));

    for (j=0; j <= nsymbols; j++) {
   	printf("%s ", Symbols[j].SymbName);
	printf(" %d	", Symbols[j].ivalue);
	printf(" %s\n", Symbols[j].Svalue);
    }

}
#endif DEBUG

/*
 * Read a line, decode the command, return command id
 */
getcmd()
{
	int i, length;
	register char *q;
	char line[BUF_LINE];	/* input line */

	if (fgets(line, BUF_LINE, stdin) == NULL) {	/* EOF (^D) */
	    printf("\n");
	    return(QUIT);
	}
	if (line[0] == '\n')
	    return(NOREPLY);

	if (line[strlen(line) - 1] == '\n')
	    line[strlen(line) - 1] = NULL;

	for(q=line; isspace(*q); q++)		/* strip leading blanks */
		;
	MapLowerCase(line);
	length = strlen(line);
	for (i=0; cmdtyp[i].cmd_name; i++) {
	    if (strncmp(cmdtyp[i].cmd_name, line, length) == 0) {
		    return(cmdtyp[i].cmd_id);	/* command id */
	    }
	}
	return(BADCMD);
}

/*
 * returns -1 if a System Directory, else 0
 */
oneofthem(p)
char *p;
{
       if ((strcmp(p,"/") == 0)
	|| (strcmp(p, "/bin") == 0)
	|| (strcmp(p, "/dev") == 0)
	|| (strcmp(p, "/etc") == 0)	
	|| (strcmp(p, "/lib") == 0)
	|| (strcmp(p, "/opr") == 0)
	|| (strcmp(p, "/sys") == 0)	
	|| (strcmp(p, "/tmp") == 0)
	|| (strcmp(p, "/usr") == 0)	
	|| (strcmp(p, "/usr/adm") == 0)
	|| (strcmp(p, "/usr/bin") == 0)	
	|| (strcmp(p, "/usr/crash") == 0)
	|| (strcmp(p, "/usr/dict") == 0)
	|| (strcmp(p, "/usr/doc") == 0)
	|| (strcmp(p, "/usr/etc") == 0)	
	|| (strcmp(p, "/usr/lib") == 0)
	|| (strcmp(p, "/usr/man") == 0)	
	|| (strcmp(p, "/usr/orphan") == 0)
	|| (strcmp(p, "/usr/preserve") == 0) 
	|| (strcmp(p, "/usr/spool") == 0)
	|| (strcmp(p, "/usr/src") == 0)	
	|| (strcmp(p, "/usr/sys") == 0)
	|| (strcmp(p, "/usr/ucb") == 0)	
	|| (strcmp(p, "/usr/usep") == 0)) {
		return(-1);
	}
return(0);
}

/* baddir()
 * 	decides whether login directory is safe
 * to use, since could possibly result in "rm -rf dir"
 * which is deadly in most cases.
 *
 * returns -1 if not OK to use, else 0.
 */
baddir(p)
char *p;	/* pointer to new directory */
{
    if (oneofthem(p) < 0) {
	printf("\n		    WARNING");
	printf("\nYou should not use directory \"%s\" as a login\n", p);
	printf("directory.  This directory is used for system files,\n");
	printf("which are used by other users on the system.  Please\n");
	printf("pick something else.\n");
	return(-1);
    }
    return(0);
}

/* oktorm()	OK-to-remove (not a system directory)
 * decides whether current login directory is OK
 * to remove, since "rm -rf logindir" is not always safe.
 * Returns -1 if NOT OK, else 0.
 */
oktorm(p)
char *p;	/* pointer to directory */
{
    if (oneofthem(p) < 0) {
	printf("\n			SORRY");
	printf("\nYou cannot remove this account because this user's home\n");
	printf("directory (%s) is used for system files or commands.\n", p);
	printf("If the nu program were to delete this directory, it would\n");
	printf("use a command like 'rm -rf %s', which you most certainly\n", p);
	printf("don't want to have happen.  Your only choice is to remove\n");
	printf("this account by hand, if you still want to delete it.\n");
	return(-1);
    }
    return(0);
}

usage()
{
    fprintf(stderr, "\nusage: /etc/nu\n\n");
    exit(0);
}

leave(status)
int status;
{
    unlink (LINKFILE);
    exit (status);
}
