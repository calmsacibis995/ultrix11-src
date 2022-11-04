
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)lprsetup.c	3.0	4/21/86";

/************************************************************
*
*	Printer Installation/Setup  Program
*
*	This program helps system administrators set-up
*	printers for their system.  It guides through the
*	steps in setting up /etc/printcap, makes all of the
*	necessary links and directories for each printer,
*	and insures everything necessary for successful
*	printer operation was specified.
*
************************************************************/

#include "lprsetup.h"
#include "globals.h"
#include <sys/types.h>
#include <sys/stat.h>

long time();
extern int errno;

main (argc, argv)
int argc;
char *argv;
{
    int	i;

    if (argc != 1)
	usage();

    /**********************
    * check prerequisites
    **********************/
    if (getuid () != 0)
    {
	printf ("\n%s: must be superuser!\n\n", progname);
	leave (ERROR);
    }

    /* for Popen later (in misc.c) */
    for (i = 3; i < 20; i++)
	close(i);
    dup2(1, 3);

    if (fopen (PRINTCAP, "r") == NULL)
    {
	perror (PRINTCAP);
	leave (ERROR);
    }

    printf ("\nULTRIX-11 Printer Setup Program\n");

    /******************
    * loop until done
    ******************/
    for (;;)
    {
        /********************************
        *  clear changable table values
        ********************************/
        for (i = 0; tab[i].name != 0; ++i)
        {
	    tab[i].used = NO;
	    tab[i].nvalue = 0;
        }
	/*
	 * clear modify flag after each time
	 * through, used to keep track of when to
	 * link the filter(s).
	 */
	modifying = FALSE;

	printf ("\nCommand  < add modify delete exit help >: ");

	strcpy(pname, "");
	strcpy(pnum, "");

	switch (getcmd ())
	{
	    case ADD:
		DoAdd ();
		break;
	    case MODIFY:
		DoModify ();
		break;
	    case DELETE:
		DoDelete ();
		break;
	    case QUIT:
		printf("\n");
		leave (OK);
		break;
	    case HELP:
		printf (h_help);
		break;
	    case NOREPLY:
		break;
	    default:
		printf ("\nSorry, invalid choice.  Type '?' for help.\n");
		break;
	}
	freemem ();
    }
}

/*****************************************
*  add new entry to printcap and create
*  corresponding special device
*****************************************/
DoAdd ()
{
    int     done;
    int     status;

    printf("\nAdding printer entry, type '?' for help.\n");

    /***********************
    * get printer number
    ***********************/
    done = FALSE;
    while (!done)
    {
	int c;
	strcpy(pnum, "0");	/* assume zero if the next 9 don't match */
	strcpy(pname, "");

	/*
	 * See what the lowest numbered printer is, then
	 * use that as the default.  Only checks for printers
	 * 0 through 9, even though we support numbers up to 99.
	 * If an empty slot is not found, the printer number
	 * is reset to 0.
	 */
	symbolname[1] = '\0';
	for (c = '0'; c < ':'; c++) {	/* ':' comes after '9' */
		symbolname[0] = c;
		status = pgetent(bp, symbolname);
		if (status == -1) {
			badfile(PRINTCAP);
		}
		if (status == 1)
			continue;	/* this printer exists, try the next */
		else {
			strcpy(pnum, symbolname);
			break;
		}
	}
	if (strchr ("0123456789", pnum[0]) == NULL) {
		strcpy(pnum, "0"); /* should never happen but just in case... */
	}

	printf ("\nEnter printer number to add [%s] : ", pnum);
	switch (getcmd ())
	{
	    case NOREPLY:
		strcpy(symbolname, pnum); /* he entered the default printer # */
		/* no break! ...falls through to case GOT_SYMBOL: */
	    case GOT_SYMBOL:
		if ((status = strchr ("0123456789", symbolname[0])) == 0)
		     printf("\nSorry, bad printer number '%s'.  Enter a single digit [0-9]\n", symbolname);
		else {
		    status = pgetent (bp, symbolname);
		    if (status == -1) {
			badfile(PRINTCAP);
		    }
		    if (status == 1)
			printf("\nSorry, printer '%s' already exists.\n", symbolname);
		    else
			done = TRUE;
		}
		break;
	    case QUIT:
		return (QUIT);
		break;
	    case HELP:
		printf (h_doadd);
		break;
	    default:
		printf ("\nInvalid choice.  Type '?' for help.\n");
		break;
	}
    }
    strcpy (pnum, symbolname);
    sprintf(pname, "lp%s", pnum);

    if (AddField () != QUIT)
    {
	if (AddEntry () == ERROR)
	    printf("\nError in adding printcap entry, try again.\n");
	else
	if (AddDevice () == ERROR)
	    printf("\nError in associating printer files/directories.\n");
    }
    freemem ();
    printf("\n");
    return (OK);
}

/***********************************
*  modify existing printcap entry
************************************/
DoModify ()
{
    int     done = FALSE;
    int     status;

    printf("\nModifying a printer entry, type '?' for help.\n");
    while (!done)
    {
	/*
	 * flag the fact that we are modifying an entry and
	 * don't want to later attempt to link a filter since
	 * the second time around, the filter name is already
	 * linked to the proper filter.
	 */
	modifying = TRUE;

	strcpy(pnum, "");
	strcpy(pname, "");
	strcpy(longname, "");	/* in case of previous modify */

	printf ("\nEnter printer number to modify: ");

	switch (getcmd ())
	{
	    case GOT_SYMBOL:
		if ((status = strchr ("0123456789", symbolname[0])) == 0)
		     printf("\nSorry, bad printer number '%s'.  Enter a single digit [0-9]\n", symbolname);
		else
		{
		    status = pgetent (bp, symbolname);
		    if (status == -1) {
			badfile(PRINTCAP);
		    }
		    if (status == 0)
		        printf ("\nSorry, printer number '%s' is not in %s.\n", symbolname, PRINTCAP);
		    else
		    {
    		        strcpy(pnum, symbolname);
			sprintf(pname, "lp%s", pnum);
			findptype();		/* get correct global ptype */
		        ModifyEntry();
			done = TRUE;	/* get back to main menu */
		    }
		}
		break;
	    case QUIT:
		done = TRUE;
		break;
	    case HELP:
		printf (h_domod);
		break;
	    case NOREPLY:
		break;
	    default:
		printf ("\nInvalid choice, try again.\n");
		break;
	}
    }
    freemem();
    return (OK);
}

/***********************************
*  delete existing printcap entry
************************************/
DoDelete ()
{
    int     done = FALSE;
    int     status;

    printf ("\nDeleting a printer entry, type '?' for help.\n");
    while (!done)
    {
	strcpy(pnum, "");
	strcpy(pname, "");

	printf ("\nEnter printer number to delete: ");
	switch (getcmd ())
	{
	    case GOT_SYMBOL:
		if ((status = strchr ("0123456789", symbolname[0])) == 0)
		     printf("\nSorry, bad printer number '%c'.  Enter a single digit [0-9]\n", symbolname[0]);
		else
		{
		    status = pgetent (bp, symbolname);
		    if (status == -1) {
			badfile(PRINTCAP);
		    }
		    if (status == 0)
		        printf ("\nCannot delete printer %s, entry not found.\n", symbolname);
		    else
		    {
			strcpy(pnum, symbolname);
			sprintf(pname, "lp%s", pnum);
		        done = TRUE;
		    }
		}
		break;
	    case QUIT:
		return (QUIT);
		break;
	    case HELP:
		printf (h_dodel);
		break;
	    case NOREPLY:
		break;
	    default:
		printf ("\nInvalid choice, try again.\n");
	}
    }

    /*********************************
    *  read printcap into tab for
    *  final confirmation
    **********************************/
    CopyEntry ();

    Print (USED);
    printf ("\nDelete %s, are you sure? [n]  ", pname);

    if (YesNo ('n') == TRUE) {
	DeleteEntry ();
    }
    else {
	printf ("\n%s not deleted.\n", pname);
    }

    freemem ();
    printf("\n");
    return (OK);
}

/**************************
*  get fields for DoAdd
**************************/
AddField ()
{
    char    buf[LEN];		/* temp buffer		 */
    int     done;		/* flag			 */
    int     i;			/* temp index		 */

    /********************************
    *  clear changable tab values
    ********************************/
    for (i = 0; tab[i].name != 0; ++i)
    {
	tab[i].used = NO;
	tab[i].nvalue = 0;
    }

    if (MatchPrinter () == QUIT)
	return (QUIT);

    if (AddSyn () == QUIT)
	return (QUIT);

    if (strcmp (ptype, "lp11") != 0)
    {
	do {
	    printf ("\nSet printer baud rate 'br'");
	    sprintf (buf, "%s", "9600");
	} while (SetVal ("br", buf) < 0);

	do {
	    printf ("\nSet device pathname 'lp'");
	    /*
	     * If printer number is a single non-zero digit,
	     * then use that number for terminal line;
	     * otherwise, default to tty00.
	     */
	    if ((pnum[0] != '0') && (pnum[1] == '\0'))
                sprintf (buf, "%s%s", "/dev/tty0", pnum);
            else
                sprintf (buf, "%s", "/dev/tty00");
	} while (SetVal ("lp", buf) < 0);

	do {
	    printf ("\nSet output filter 'of'");
	    sprintf (buf, "%s", "/usr/lib/ulf");
	} while (SetVal ("of", buf) < 0);

	do {
	    printf ("\nSet accounting file 'af'");
            if (pnum[0] == '0' )
                sprintf (buf, "%s", "/usr/adm/lpacct");
            else
                sprintf (buf, "%s%s%s", "/usr/adm/lp", pnum, "acct");
	} while (SetVal ("af", buf) < 0);
    }

    /* These next values are set for ALL printers, including LP11 */

    do {
        printf ("\nSet spooler directory 'sd'");
        if (pnum[0] == '0' )
            sprintf (buf, "%s", "/usr/spool/lpd");
        else
            sprintf (buf, "%s%s", "/usr/spool/lpd", pnum);
    } while (SetVal ("sd", buf) < 0);

    do {
        printf ("\nSet printer error log file 'lf'");
        if (pnum[0] == '0' )
            sprintf (buf, "/usr/adm/lp.err");
        else
            sprintf (buf, "/usr/adm/lp%s.err", pnum);
    } while (SetVal ("lf", buf) < 0);

    /*********************************
    *  modify default field values
    *********************************/
    printf (h_symsel);
    for (i = 0; tab[i].name != 0; ++i)
        printf ("%s ", tab[i].name);
    printf ("\n");

    done = FALSE;
    for (;;)
    {
	while (!done)
	{
		printf ("\nEnter symbol name: ");
		switch (getcmd ())
		{
		    case GOT_SYMBOL:
			DoSymbol ();	/* Don't have to special case sd, the
					 * spooling directory since the entry
					 * is being added. It still can be
					 * changed since it doesn't exist yet.
					 */
			break;
		    case HELP:
			printf (h_symsel);
			for (i = 0; tab[i].name != 0; ++i)
			    printf ("%s ", tab[i].name);
			printf ("\n");
			break;
		    case NOREPLY:
			break;
		    case PRINT:
			Print (USED);
			break;
		    case LIST:
			Print (ALL);
			break;
		    case QUIT:
			done = TRUE;
			break;
		    default:
			printf ("\nInvalid choice, try again.\n");
			break;
		}
	}
	if (Verified () == TRUE)
	    break;
	else
	{
	    printf("Do you wish to continue with this entry?  [y] ");
	    if (YesNo ('y') == FALSE) {
		return(QUIT);		/* no, they wish to abort */
	    }
	    else
	        done = FALSE;
	}
    }
    return (OK);
}

/****************************************************
*  find matching printer by name and copy over fields
*  loop through default table, find match and assign.
*****************************************************/
MatchPrinter ()
{
    int     found, done;	/* flags	*/
    int     length;		/* strlen return */
    char   *addr;		/* malloc return */
    int     i, j, k;		/* temp indices	 */

    /****************************
    *  get printer type
    ****************************/
    found = FALSE;
    while (!found)
    {
	done = FALSE;
	while (!done)
	{
	    printf ("\nEnter printer type: ");
	    switch (getsymbol ())
	    {
		case GOT_SYMBOL:
		default:
		    strcpy (ptype, symbolname);
		    done = TRUE;
		    break;
		case HELP:
		    printf (h_type);
		    for (i = 0; printer[i].name != 0; ++i)
			printf ("%s ", printer[i].name);
		    printf ("\n");
		    break;
		case QUIT:
		    return (QUIT);
		    break;
		case NOREPLY:
		    printf ("\nUsing 'unknown' for printer type, OK? [ n ] ");
		    if (YesNo ('n') == TRUE) {
			strcpy (ptype, UNKNOWN);
		        done = TRUE;
		        found = TRUE;
		    }
		    break;
	    }
	}

	/******************************************
	*  loop through printer table, find match
	******************************************/
	for (i = 0; printer[i].name; ++i)
	    if (strncmp (printer[i].name, ptype, strlen (ptype)) == 0)
	    {
		found = TRUE;
		break;
	    }

	if (!found)
	{
	    printf ("\nDon't know about printer '%s'\n", ptype);
	    printf ("\nEnter 'y' to try again, or 'n' to use 'unknown' [y]: ");
	    if (YesNo ('y') == FALSE)
	    {
		strcpy (ptype, UNKNOWN);
		found = TRUE;
	    } else {
		found = FALSE;
	    }
	}
    }

    /**************************************************************
    * loop thru printer values and assign to corresponding
    * default new values - note: i contains correct printer index
    **************************************************************/
    for (j = 0; printer[i].entry[j].name; ++j)
    {
	/******************************
	*  loop through default table
	*******************************/
	for (k = 0; tab[k].name; ++k)
	{
	    if (strcmp (printer[i].entry[j].name, tab[k].name) == 0)
	    {
		length = strlen (printer[i].entry[j].svalue) + 1;
		if ((addr = malloc (length)) == NULL)
		{
		    printf ("\nmalloc: not enough space for symbols!\n");
		    return (ERROR);
		}
		tab[k].nvalue = addr;
		strcpy (tab[k].nvalue, printer[i].entry[j].svalue);
		tab[k].used = TRUE;
	    }
	}
    }
    return (OK);
}

/*****************************
*  add synonyms
*****************************/
AddSyn ()
{
    int     done, status;

    sprintf (pname, "lp%s", pnum);
    strcat (pname, "|");
    if (pnum[0] == '0')		/* add lp entry if lp0 */
	    strcat(pname, "lp|");
    strcat (pname, pnum);

    done = FALSE;
    while (!done && strlen (pname) < 80)
    {
        printf ("\nEnter printer synonym: ");
	switch (getcmd ())
	{
	    case GOT_SYMBOL:
		status = pgetent (bp, symbolname);
		if (status == -1) {
			badfile(PRINTCAP);
		}
		if (status == 1)
		    printf ("\nSynonym is already in use, try something else.\n");
		else
		    if (strlen (pname) + strlen (symbolname) > 79)
			printf ("\nSynonym too long, truncating to 80 characters.\n");
		    else
		    {
			strcat (pname, "|");
			strcat (pname, symbolname);
		    }
		break;
	    case HELP:
		printf (h_synonym);
		break;
	    case QUIT:
		return (TRUE);
		break;
	    case NOREPLY:
		return (TRUE);
		break;
	    default:
		printf ("\nInvalid choice, try again.\n");
		break;
	}
    }
    return(OK);
}

/*********************************
*  set default values
*  Returns -1 on error, 0 if ok
*********************************/
SetVal (val, buf)
char   *val;			/* two-letter symbol	 */
char   *buf;			/* preset value		 */
{
    int     i;			/* temp index		 */
    char    line[LEN];		/* temp buffer		 */

    /******************
    *  find val
    ******************/
    for (i = 0; tab[i].name; ++i)
	if (strcmp (tab[i].name, val) == 0)
	    break;
    if (tab[i].name == 0) {
	printf("internal error: cannot find symbol %s in table\n", val);
	return (BAD);
    }

    /************************
    *  set by MatchPrinter
    ************************/
    if (tab[i].nvalue != 0)
    {
	if (UseDefault (line, tab[i].nvalue, i) == FALSE)/* 3rd arg is index */
	{
	    if (validate(i, line) < 0) {
		return(BAD);
	    }
	    if ((tab[i].nvalue = malloc (strlen (line) + 1)) == NULL)
	    {
	    	printf ("\nmalloc: no space for %s\n", tab[i].name);
		return (ERROR);
	    }
	    strcpy (tab[i].nvalue, line);
	}
    }
    /************************
    *  use default
    ************************/
    else
    {
	UseDefault (line, buf, i);

	if (validate(i, line) < 0) {
	    return(BAD);
	}
	if ((tab[i].nvalue = malloc (strlen (line) + 1)) == NULL)
	{
	    printf ("\nmalloc: no space for %s\n", tab[i].name);
	    return (ERROR);
	}
	tab[i].used = YES;
	strcpy (tab[i].nvalue, line);
    }
    return(OK);
}

/**********************************************
*  write new printcap entry to printcap file
***********************************************/
AddEntry ()
{
    FILE *ofp, *lfp;		/* ouput and log file pointers */
    long     timeval;		/* time for log file		*/
    char     buf[LEN];		/* temp buffer			*/

    /****************************************
    *  open output and log files
    ****************************************/
    if ((ofp = fopen (PRINTCAP, "a")) == NULL) {
	badfile(PRINTCAP);
    }
    if ((lfp = fopen (LOGCAP, "a")) == NULL) {
	badfile(LOGCAP);
    }

    WriteEntry (ofp);

    /****************************************
    *  write time stamp and entry to log
    ****************************************/
    timeval = time(0);
    strcpy (buf, "\nAdded ");
    strcat (buf, ctime (&timeval));
    fputs (buf, lfp);

    WriteEntry (lfp);

    fclose (ofp);
    fclose (lfp);
    return (OK);
}

/***************************************
*  create special device, if necessary
***************************************/
AddDevice ()
{
    struct passwd  *passwd;	/* password file entry	 */
    char   *device;		/* parameter value ptr	 */
    int     mode;		/* chmod mode		 */
    int     i;			/* temp index		 */

    /*******************************
    *  get daemon id
    *******************************/
    if ((passwd = getpwnam (DAEMON)) == 0)
    {
	printf ("\ngetpwnam: cannot get id for %s\n", DAEMON);
	perror ();
	leave (ERROR);
    }

    /************************************
    *  chown and chmod device to daemon
    ************************************/
    for (i = 0; tab[i].name != 0; ++i)
	if (strcmp (tab[i].name, "lp") == 0)
	    break;

    if (tab[i].name == 0)
    {
	printf ("\nCannot find device pathname in table.\n");
	return (ERROR);
    }

    device = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    mode = 00700;
    if (chmod (device, mode) < 0)
    {
	printf ("\nCannot chmod %s to %o", device, mode);
	perror (device);
	/* continue anyway */
    }
    if (chown (device, passwd->pw_uid, passwd->pw_gid) < 0)
    {
	printf ("\nCannot chown %s to (%o/%o)\n", device,
	    passwd->pw_uid, passwd->pw_gid);
	perror (device);
	/* continue anyway */
    }

    /*******************************
    *  create spooling directory
    *******************************/
    MakeSpool (passwd);

    /***********************************
    *  link printer filter, if not LP11
    ***********************************/
    if (strcmp(ptype,"lp11") != 0)
	LinkFilter();
}

/******************************
*  Modify selected entry
*******************************/
ModifyEntry ()
{
    struct passwd *passwd;	/* passwd entry ptr		 */
    int     done;		/* flag				 */
    FILE    *ifp, *ofp;		/* input/ouput file pointers	 */
    char    keyname[LEN];	/* match name			 */
    char    buf[80];		/* read/write buffer		 */
    int	    i;			/* temp index			 */

    /*******************************
    *  get daemon id
    *******************************/
    if ((passwd = getpwnam (DAEMON)) == 0)
    {
	printf ("\ngetpwnam: cannot find %s uid in passwd file.\n", DAEMON);
	leave (ERROR);
    }

    /**********************
    *  modify fields
    **********************/
    CopyEntry ();	/* affects longname */

    for (;;)
    {
	done = FALSE;
	printf("\nEnter the name of the symbol you wish to change.\n");
	printf("Enter 'p' to print the current values, or 'q' to quit.\n");
	while (!done)
	{
	    printf ("\nEnter symbol name:  ");
	    switch (getcmd ())
	    {
		case GOT_SYMBOL:
		    if (strcmp("sd", symbolname) == 0)
			printf ("\nSorry, you cannot modify '%s'.\n",
				symbolname);
		    else
		    	DoSymbol ();
		    break;
		case HELP:
		    printf (h_symsel);
		    for (i = 0; tab[i].name != 0; ++i)
		        printf ("%s ", tab[i].name);
		    printf ("\n");
		    break;
		case NOREPLY:
		    break;
		case PRINT:
		    Print (USED);
		    break;
		case LIST:
		    Print (ALL);
		    break;
		case QUIT:
		    done = TRUE;
		    break;
		default:
		    printf ("\nInvalid choice, try again.\n");
		    break;
	    }
	}
	if (Verified() == TRUE)
	    break;
	else
	{
	    printf("Do you wish to continue with this entry?  [y] ");
	    if (YesNo ('y') == FALSE) {
		return(QUIT);	/* no, they wish to abort, although at this
			point, the return value (from here) is not checked.
			We just return early without actually do anything. */
	    }
	    else
	        done = FALSE;	/* not done yet */
	}
    }

    /**************************
    * save pname for match
    * and longname to rewrite
    **************************/
    strcpy (keyname, pname);	/* search later for keyname, like "lp2" */
    strcpy (pname, longname);	/* new pname contains the entire first line */

    /******************************
    *  open original and copy
    *******************************/
    if ((ifp = fopen (PRINTCAP, "r")) == NULL) {
	badfile(PRINTCAP);
    }
    if ((ofp = fopen (COPYCAP, "w")) == NULL) {
	badfile(COPYCAP);
    }

    /*************************************
    *  copy printcap to copy until entry
    **************************************/
    while (fgets (buf, 80, ifp) != 0)
    {
	if (strncmp (keyname, buf, strlen (keyname)) != 0)
	    fputs (buf, ofp);
	else
	    while (fgets (buf, 80, ifp) != 0)
		if (buf[strlen(buf) - 2] != '\\')
		{
		    WriteEntry (ofp);
		    break;
		}
    }

    fclose (ofp);
    fclose (ifp);

    /***************************
    * mv new file to old file
    ***************************/
    if (rename (COPYCAP, PRINTCAP) < 0)
    {
	printf ("\nCannot rename %s to %s (errno = %d).\n",
		COPYCAP, PRINTCAP, errno);
	/* don't know what best to do here...*/
    }

    /*******************************
    *  create spooling directory
    *******************************/
    MakeSpool (passwd);

    /***************************************
    * relink printer filter, if necessary
    ***************************************/
    /*
     * don't link filter for lp11 type lines; also, don't
     * want to re-link filter if we are only "modifying" the
     * entry, rather, link the filter only if this is a first
     * time entry creation for this printer.
     */
    if ((strcmp(ptype, "lp11") != 0) || (NOT modifying))
    	LinkFilter();
}


/**********************************
*  delete existing printcap entry
*  unlink printer filter, change tty
*  back to owner root, 666.  Put
*  /etc/ttys file back to 00, disabled.
**********************************/
DeleteEntry ()
{
    FILE *ifp, *ofp, *lfp;	/* input/output file pointers	*/
    char    buf[LEN];		/* read/write buffer		*/
    long    timeval;		/* time in seconds for logfile	*/
    char    tempfile[LEN];	/* file name buffer 		*/
    int     i, mode;
    char   *curval;
    char   *device;

    /*********************************
    *  open original, copy, and log
    **********************************/
    if ((ifp = fopen (PRINTCAP, "r")) == NULL) {
	badfile(PRINTCAP);
    }

    sprintf (tempfile, "%s%d", COPYCAP, getpid());
    if ((ofp = fopen (tempfile, "w")) == NULL)
    {
	printf("\nCannot open intermediate file: %s.\n", tempfile);
	perror (tempfile);
	leave (ERROR);
    }

    if ((lfp = fopen (LOGCAP, "a")) == NULL) {
	badfile(LOGCAP);
    }

    timeval = time(0);
    strcpy (buf, "\nDeleted ");
    strcat (buf, ctime(&timeval));
    fputs (buf, lfp);

    /*****************************************
    *  copy printcap to copy until next entry
    *****************************************/
    while (fgets (buf, 80, ifp) != 0)
    {
	if (strncmp (pname, buf, strlen (pname)) != 0)
	    fputs (buf, ofp);
	else
	{
	    fputs (buf, lfp);
	    while ((fgets (buf, 80, ifp) != 0) && (buf[strlen(buf) - 2] != ':'))
	    {
		    fputs (buf, lfp);
	    }
	    /* write line with colon */
	    fputs (buf, lfp);
	}
    }

    if (rename (tempfile, PRINTCAP) < 0) {
	printf("\nCannot rename %s to %s (errno=%d).\n",
		tempfile, PRINTCAP, errno);
    }
    fclose (ofp);
    fclose (ifp);
    fclose (lfp);

    UnLinkFilter();
    UnLinkSpooler();

    /*
     * Put the /etc/ttys file entry
     * for this line to mode 00, "disabled"
     */
    for (i = 0; tab[i].name != 0; ++i) {
	if (strcmp(tab[i].name, "lp") == 0) {
	    curval=(tab[i].nvalue ? tab[i].nvalue : tab[i].svalue);
	    if (strncmp(curval, "/dev/tty", 8) == 0) {
		curval=index(curval, 't');
		fixtty(curval,"00");	/* yes, it is "/dev/ttyxx" */
		break;
	    }
        }
    }

   /*
    *  chown and chmod device back to root, 0666
    */
    for (i = 0; tab[i].name != 0; ++i)
	if (strcmp (tab[i].name, "lp") == 0)
	    break;

    if (tab[i].name == 0)
	return;		/* early return */

    device = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    if (chown (device, 0, 1) < 0)	/* ID's: root/other */
    {
	printf ("\nCannot chown %s to (0/1)\n", device);
	perror (device);
	return;		/* early return */
    }
    mode = 0666;
    if (chmod (device, mode) < 0)
    {
	printf ("\nCannot chmod %s back to %o", device, mode);
	perror (device);
	/* fall through to return */
    }
    return(OK);
}

/***********************
*  copy entry into tab
************************/
CopyEntry ()
{
    char    line[LEN];		/* read buffer		 */
    char    tmpline[LEN];	/* temporary buffer	 */
    char   *lineptr;		/* read buffer ptr	 */
    char   *ptr;		/* temp pointer		 */
    int     num;		/* pgetnum return	 */
    int     i;			/* temp index 		 */

    /*************************
    *  save names for rewrite
    **************************/
    if ((ptr = index(bp, ':')) > 0) {
	strncpy (longname, bp, ptr - &bp);
    }
    longname[ptr - &bp] = '\0';
/*
 * The first character of the long name should always be 'l'.
 */
    if (longname[0] != 'l') {
    /*	printf("1061: bp[0] is not 'l'\n"); */
	longname[0] = 'l';
    }

    /****************************************************
    * loop thru table, changing values where appropriate
    ****************************************************/
    for (i = 0; tab[i].name != 0; ++i)
    {
	switch (tab[i].stype)
	{
	    case BOOL:
		if (pgetflag (tab[i].name) == TRUE)
		{
		    if ((tab[i].nvalue = malloc (strlen ("on") + 1)) == NULL)
		    {
			printf ("\nCannot malloc space for %s\n", tab[i].name);
		    }
		    else
		    {
		        strcpy (tab[i].nvalue, "on");
		        tab[i].used = YES;
		    }
		}
		break;
	    case INT:
		if ((num = pgetnum (tab[i].name)) >= 0)
		{
		    /* fc, fs, xc, xs are in octal, all others are decimal */
		    if ((strcmp(tab[i].name, "fc") == 0) ||
			(strcmp(tab[i].name, "fs") == 0) ||
			(strcmp(tab[i].name, "xc") == 0) ||
			(strcmp(tab[i].name, "xs") == 0)) {
			sprintf (tmpline, "%o", num);
			strcpy(line, "0");	/* put the zero out in front */
			strcat(line, tmpline);
		    } else {
		    	sprintf (line, "%d", num);
		    }
		    if ((tab[i].nvalue = malloc (strlen (line) + 1)) == NULL)
		    {
			printf ("\nCannot malloc space for %s\n", tab[i].name);
		    }
		    else
		    {
		    	strcpy (tab[i].nvalue, line);
		    	tab[i].used = YES;
		    }
		}
		break;
	    case STR:
		lineptr = line;
		if (pgetstr (tab[i].name, &lineptr) != NULL)
		{
		    *lineptr = 0;
		    if ((tab[i].nvalue = malloc (strlen (line) + 1)) == NULL)
		    {
			printf ("\nCannot malloc space for %s\n", tab[i].name);
		    }
		    else
		    {
		   	strcpy(tab[i].nvalue, line);
		    	tab[i].used = YES;
		    }
		}
		break;
	    default:
		printf ("\nBad type (%d) for %s\n", tab[i].stype, tab[i].name);
		break;
	}
    }
    return(OK);
}

/******************************
*  write single entry to file
*******************************/
WriteEntry (fp)
FILE * fp;
{
    int     i;			/* temp index			 */
    char   *curval;		/* pointer to current value	 */
    char localname[LEN];	/* output filter, lp2 or whatever */

    fprintf (fp, "%s:", pname);	/* here, pname is really the longname */
    for (i = 0; tab[i].name != 0; ++i)
    {
	if (tab[i].used == YES)
	{
	    curval = (tab[i].nvalue ? tab[i].nvalue : tab[i].svalue);
	    switch (tab[i].stype)
	    {
		case BOOL:
		    if (strcmp ("on", curval) == 0)
			fprintf (fp, "\\\n\t:%s:", tab[i].name);
		    break;
		case INT:
		    if (strcmp ("none", curval) != 0)
		        fprintf (fp, "\\\n\t:%s#%s:", tab[i].name, curval);
		    break;
		case STR:
		    if (strlen (curval) > 0) {
			/*
			 * Have to correct for proper 'of' in printcap file
			 */
			if (strcmp (tab[i].name, "of") == 0) {
			    /*
			     * If it's not an LP11, generate the
			     * correct output filter name.
			     */
			    if (strcmp(ptype, "lp11") != 0) {
				sprintf(localname, "/usr/lib/lp%s", pnum);
				curval = localname;
			    }
			}
		    	fprintf (fp, "\\\n\t:%s=%s:", tab[i].name, curval);
		    }
		    break;
		default:
		    printf ("\nbad type (%d) for %s\n", tab[i].stype, tab[i].name);
		    break;
	    }
	}
    }
    fprintf (fp, "\n");
    return(OK);
}

/*******************************
*  create spooling directory
*******************************/
MakeSpool (passwd)
struct passwd  *passwd;		/* password file entry	 */
{
    char   *spool;		/* parameter value ptr	 */
    int    i;			/* temp index	*/
    struct stat sb;

    for (i = 0; tab[i].name != 0; ++i)
	if (strcmp (tab[i].name, "sd") == 0)
	    break;

    if (tab[i].name == 0)
    {
	printf ("\nCannot find spooler directory entry in table!\n");
	printf ("No spooling directory created.\n");
	return (ERROR);
    }
    spool = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    /*
     * If stat fails, and then cannot mkdir, so exit.
     */
    if (stat(spool, &sb) < 0) {
	if (mkdir(spool, DIR) == -1) {
	    printf ("\nCannot make spooling directory %s\n", spool);
	    perror (spool);
	    return (ERROR);
        }
    } else {
	/* spooling directory exists... */
	if ((sb.st_mode&S_IFMT) != S_IFDIR) {
	    printf ("\nSpooling directory %s already exists, but is not a directory!\n", spool);
	    return (ERROR);
	}
    }

    if (chmod (spool, 00755) == -1)
    {
	printf ("\nCannot chmod %s to mode 0755\n", spool);
	perror (spool);
	return (ERROR);
    }

    if (chown (spool, passwd -> pw_uid, passwd -> pw_gid) == -1)
    {
	printf ("\nCannot chown %s to (%o/%o)\n", spool, passwd->pw_uid,
	    passwd->pw_gid);
	perror (spool);
	return (ERROR);
    }
    return(OK);
}

/**************************
*  link printer filter
**************************/
LinkFilter ()
{
    char   *filter;		/* parameter value ptr	 */
    char   *curval;		/* pointer to current value	 */
    char   localname[LEN];	/* temp filter if different */
    int    i;			/* temp index	*/

    for (i = 0; tab[i].name != 0; ++i)
	if (strcmp (tab[i].name, "of") == 0)
	    break;

    if (tab[i].name == 0)
    {
	printf ("\nCannot find value of output filter 'of' in table!\n");
	return (ERROR);
    }

    /***************************
    * link current filter
    ***************************/

    /* filter is the real one to link TO, (e.g.  ulf, lqf, ln01of, ln03of) */
    filter = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    /* localname is the link itself (e.g. /usr/lib/lp2, lp5)  */
    sprintf(localname, "/usr/lib/lp%s", pnum);

    /* see if in modify mode, if so, just return */
    if (modifying == TRUE)
	return(OK);

    printf("\nunlink(%s)\n", localname);
    unlink(localname);

    if (strcmp(filter, localname))
    {
	/* if filters are NOT the same, then link them */
        printf("link(%s, %s)\n", filter, localname);
	if (link (filter, localname) < 0)
	{
	    printf ("\nCannot link(%s, %s)\n", filter, localname);
	    perror (filter);
	    return (ERROR);
	}
    } else {
	/* else filter == localname so use ULF as default */
        printf("\nlink(%s, %s)\n", ULF, localname);
	if (link (ULF, localname) < 0)
	{
	    printf ("\nCannot link(%s, %s)\n", ULF, localname);
	    perror (ULF);
	    return (ERROR);
	}
    }
    /*
     * Change the /etc/ttys file entry
     * for this tty line to mode 30, "local - no logins".
     */
    for (i = 0; tab[i].name != 0; ++i) {
	if (strcmp(tab[i].name, "lp") == 0) {
	    curval=(tab[i].nvalue ? tab[i].nvalue : tab[i].svalue);
	    if (strncmp(curval, "/dev/tty", 8) == 0) {
		curval=index(curval, 't');
		fixtty(curval,"30");	/* yes, it is "/dev/ttyxx" */
		break;
	    }
        }
    }
    return(OK);
}

/**************************
*  unlink printer filter
**************************/
UnLinkFilter ()
{
    int    i;
    char  *filter;

    for (i = 0; tab[i].name != 0; ++i)
	if (strcmp (tab[i].name, "of") == 0)
	    break;

    if (tab[i].name == 0)
	return;		/* filter is just left laying around */

    filter = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    printf("\nunlink(%s)\n", filter);
    if (unlink(filter) < 0) {
	printf("couldn't unlink old printer filter (%s)\n", filter);
    }
}

/**************************
*  unlink spooler directory
**************************/
UnLinkSpooler()
{
    int    i;
    char  *spooler;

    for (i = 0; tab[i].name != 0; ++i)
	if (strcmp (tab[i].name, "sd") == 0)
	    break;

    if (tab[i].name == 0)	/* can't find 'sd' symbol */
	return;		/* spool directory is just left laying around */

    spooler = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    printf("rmdir(%s)\n", spooler);
    if (rmdir(spooler) < 0) {
	printf("couldn't unlink old spooler directory (%s)\n", spooler);
    }
}

/**************************
 * findptype:  get correct
 * value of printer type, most
 * importantly, is it LP11?
 **************************/
findptype()
{
	char s[128];
	char *p;
	int status;

	strcpy(s, pnum);
	status = pgetent(bp, s);
	if (status == -1) {
		badfile(PRINTCAP);
	}
	if (status == 1) {	/* printer exists */
		if ((p = pgetstr("lp", &bp)) == NULL) {
			printf("Warning: Cannot find symbol 'lp' in %s\n", PRINTCAP);
		/*	printf("Warning: using 'unknown' for printer type\n"); */
			strcpy(ptype, "unknown");
		} else
			if (strcmp(p, "/dev/lp") == 0)
				strcpy(ptype, "lp11");
	}
}

/**************************
 * badfile: print "cannot open
 * <filename>", and exit(1).
 **************************/
badfile(s)
char *s;
{
	printf("\nCannot open %s\n", s);
	perror(s);
	leave(ERROR);
}

/******************************************************************************
* end of lprsetup.c
******************************************************************************/
