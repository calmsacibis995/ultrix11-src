
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)ipc_test.c 3.1 3/4/87";

/*************************************************************************
*  ipc_test.c
*
*  USAT test for IPC facilities
*
*  Bob Bagwill
*************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/maus.h>

#include <ndir.h>
#include <fcntl.h>
#include <a.out.h>
#include <setjmp.h>

#define PASS	0
#define FAIL	1
#define STRING	"hello, world\n"
#define MSG	0
#define SEM	1
#define MAUS	2
#define CHILDREN 5

/* name list to check for ipc facilities */
struct nlist    nl[] =
{ { "_msgque" }, { "_sema" }, { "_nmausen" }, { NULL } };

extern int  errno;

main (argc, argv)
int     argc;
char  **argv;
{
    int     status = 0;		/* return status	 */

    /************************************
    * check for ipc facilities in /unix
    ************************************/
    nlist ("/unix", nl);

    /************************************
    * check for messages
    ************************************/
    if (nl[MSG].n_value)  
    {
	fprintf (stdout, "messages configured\n");
	fflush (stdout);
	if (messages () == FAIL)
	{
	    fprintf (stdout, "messages test failed\n");
	    fflush (stdout);
	    ++status;
	}
	else
	{
	    fprintf (stdout, "messages test passed\n");
	    fflush (stdout);
	}
    }
    else
    {
	fprintf (stdout, "messages not configured\n");
	fflush (stdout);
	++status;
    }   

    /************************************
    * check for semaphores
    ************************************/
    if (nl[SEM].n_value)
    {
	fprintf (stdout, "semaphores configured\n");
	fflush (stdout);
	if (semaphores () == FAIL)
	{
	    fprintf (stdout, "semaphores test failed\n");
	    fflush (stdout);
	    ++status;
	}
	else
	{
	    fprintf (stdout, "semaphores test passed\n");
	    fflush (stdout);
	}
    }
    else
    {
	fprintf (stdout, "semaphores not configured\n");
	fflush (stdout);
	++status;

    }

    /************************************
    * check for maus
    ************************************/
    if (nl[MAUS].n_value)
    {
	fprintf (stdout, "maus configured\n");
	fflush (stdout);
	if (maus () == FAIL)
	{
	    fprintf (stdout, "maus test failed\n");
	    fflush (stdout);
	    ++status;
	}
	else
	{
	    fprintf (stdout, "maus test passed\n");
	    fflush (stdout);
	}
    }
    else
    {
	fprintf (stdout, "maus not configured\n");
	fflush (stdout);
	++status;

    }

    return (status);
}

/***********************************
* message facility test
***********************************/
messages ()
{
    int     msqid;		/* message queue id		 */
    int     msgflg;		/* message permissions flag	 */
    int     msgsz;		/* message size		 */
    int     i;			/* temp index			 */
    key_t key;			/* message key			 */
    struct msqid_ds buf;	/* message queue data structure */
    struct msgbuf		/* message data structure	 */
    {
	long    mtype;
	char    mtext[80];
    }               msgp;
    int     status;		/* child exit status		 */
    int	    ret = PASS;		/* return value */

    /***************************************
    *  fork processes to test queues
    ***************************************/
    for (i = 0; i < CHILDREN; ++i)
    {
	if (fork () == 0)
	{
	    /*************************
	    *  request message queue
	    *************************/
	    key = i+1;
	    msgflg = IPC_CREAT | 0666;
	    if ((msqid = msgget (key, msgflg)) < 0){
			perror("msgget");
		if (msqid != -1) {
			printf("msgqid limit exceeded\n");
		}
		exit (FAIL);
	    }

	    /*************************
	    *  post message to queue
	    *************************/
	    msgp.mtype = 1;
	    strcpy (msgp.mtext, STRING);
	    msgsz = strlen (msgp.mtext);
	    msgflg |= IPC_NOWAIT;

	    if (msgsnd (msqid, &msgp, msgsz, msgflg) < 0) {
		perror("msgsnd");
		exit (FAIL);
	    }

	    /*******************************************
	    *  check message queue for correct posting
	    ********************************************/
	    if (msgctl (msqid, IPC_STAT, &buf) < 0){
		perror("msqid");
		exit (FAIL);
	    }

	    if ((buf.msg_qnum != 1) || (buf.msg_lspid != getpid ())) {
		printf("wrong message queue\n");
		exit (FAIL);
	    }

	    /***************************
	    *  read message from queue
	    ***************************/
	    msgflg = IPC_NOWAIT | MSG_NOERROR;
	    if (msgrcv (msqid, &msgp, msgsz, 0L, msgflg) < 0) {
		perror("msgrcv");
		exit (FAIL);
	    }

	    if ((msgp.mtype != 1) || (strcmp (msgp.mtext, STRING) != 0)) {
		printf("wrong message received\n");
		exit (FAIL);
	    }

	    /**********************************************
	    *  check message queue for correct unposting
	    ***********************************************/
	    if (msgctl (msqid, IPC_STAT, &buf) < 0) {
		perror("msgctl");
		exit (FAIL);
	    }
	    if ((buf.msg_qnum != 0) || (buf.msg_lrpid != getpid ()) ||
		    (buf.msg_rtime <= time ())) {
		printf("message parameters do not match\n");
		exit (FAIL);
	    }

	    /************************************
	    *  request removal of message queue
	    ************************************/
	    if (msgctl (msqid, IPC_RMID, &buf) < 0) {
		perror("msgctl");
		exit (FAIL);
	    }

	    /**************************************
	    *  check for removal of message queue
	    **************************************/
	    if (msgctl (msqid, IPC_RMID, &buf) != -1) {
		perror("msgctl");
		exit (FAIL);
	     }

	    exit (PASS);
	}
    }

    /***********************************************
    * wait for children and shift for exit status
    ***********************************************/
    for (i = 0; i < CHILDREN; ++i)
    {
	wait (&status);
	if ((status >> 8) == FAIL)
	    ret = FAIL;
    }

    return (ret);
}

/***************************************
* semaphore facility test
***************************************/
#define NSEMS 5			/* number of semaphores in set	 */

semaphore ()
{
    key_t key;			/* semaphore set key		 */
    int     semid;		/* semaphore set id		 */
    int     semflg;		/* semaphore permissions flag	 */
    int     nsems = NSEMS;	/* number of semaphores in set	 */
    int     semnum;		/* index within semaphore set	 */
    int     cmd;		/* semctl cmd			 */
    int     nsops;		/* number of semaphore operations */
    struct sembuf   sops[NSEMS];/* semaphore operations	 */
    union			/* semctl arg			 */
    {
	int     val;
	struct semid_ds *buf;
	ushort * array;
    } arg;
    struct semid_ds data;	/* semaphore data structure	 */
    ushort values[NSEMS];	/* semaphore values		 */
    int     i, j;		/* temp index			 */
    int     status;		/* child exit status		 */
    int     ret = PASS;

    semflg = IPC_CREAT | 0666;

    /***************************************
    *  fork children to test multi process
    ****************************************/
    for (i = 0; i < CHILDREN; ++i)
    {
	if (fork () == 0)
	{
	    key = i+1;
	    /*************************
	    *  request semaphore set
	    **************************/
	    if ((semid = semget (key, nsems, semflg)) < 0)
	    {
		perror("semget");
		if (semid != -1)
			printf("semaphore id limit exceeded\n");
		exit (FAIL);
	    }

	    /******************************
	    *  check for correct creation
	    *******************************/
	    arg.buf = &data;
	    cmd = IPC_STAT;

	    if (semctl (semid, semnum, cmd, arg) < 0)
	    {
		perror("semctl");
		exit (FAIL);
	    }
	    if ((arg.buf -> sem_perm.cuid != getuid ()) ||
		    (arg.buf -> sem_perm.cgid != getgid ()) ||
		    (arg.buf -> sem_perm.uid != getuid ()) ||
		    (arg.buf -> sem_perm.gid != getgid ()) ||
		    (arg.buf -> sem_nsems != nsems))
	    {
		printf("semaphore parameters do not match\n");
		exit (FAIL);
	    }

	    /***********************************
	    *  increment semaphores
	    ************************************/
	    for (j = 0; j < NSEMS; ++j)
	    {
		sops[j].sem_num = j;
		sops[j].sem_op = 1;
		sops[j].sem_flg = 0;
	    }
	    nsops = NSEMS;
	    if (semop (semid, sops, nsops) < 0)
	    {
		perror("semop");
		exit (FAIL);
	    }

	    cmd = GETALL;
	    arg.array = values;
	    if (semctl (semid, semnum, cmd, arg) < 0)
	    {
		perror("semctl");
		exit (FAIL);
	    }

	    for (j = 0; j < NSEMS; ++j)
		if (values[j] != 1) {
		    printf("semaphore value wrong\n");
		    exit (FAIL);
		}

	    /***********************************
	    *  decrement semaphores
	    ************************************/
	    for (j = 0; j < NSEMS; ++j)
	    {
		sops[j].sem_num = j;
		sops[j].sem_op = -1;
		sops[j].sem_flg = IPC_NOWAIT;
	    }
	    nsops = NSEMS;
	    if (semop (semid, sops, nsops) < 0)
	    {
		perror("semop");
		exit (FAIL);
	    }

	    cmd = GETALL;
	    arg.array = values;
	    if (semctl (semid, semnum, cmd, arg) < 0)
	    {
		perror("semctl");
		exit (FAIL);
	    }

	    for (j = 0; j < NSEMS; ++j)
		if (values[j] != 0) {
		    printf("semaphore value wrong\n");
		    exit (FAIL);
		}

	    /***********************************
	    *  request removal of semphore set
	    ************************************/
	    if (semctl (semid, semnum, IPC_RMID, arg) < 0)
	    {
		perror("semctl");
		exit (FAIL);
	    }

	    /***********************************
	    *  check removal of semphore set
	    ************************************/
	    if (semctl (semid, semnum, IPC_STAT, arg) != -1)
	    {
		perror("semctl");
		exit (FAIL);
	    }
	    exit (PASS);
	}
    }

    /*********************************************
    *  wait for children and shift for exit value
    **********************************************/
    for (i = 0; i < CHILDREN; ++i)
    {
	wait (&status);
	if ((status >> 8) == FAIL)
	    ret = FAIL;
    }
    return (ret);
}

/**********************************************
*  multiple access user space operations test
**********************************************/
jmp_buf env;			/* signal handling		 */
int     segv_flg;		/* segmentation violation flag	 */

maus ()
{
    extern struct nlist nl[];	/* name list			 */
    char   *maus;		/* pointer to maus data segment */
    char    path[MAXNAMLEN];	/* maus special device pathname */
    int     oflag = O_RDWR;	/* maus access permissions	 */
    int     mausdes;		/* maus data segment descriptor */
    int     segv ();		/* signal handling function	 */
    int     i, j, k;		/* temp indices		 */
    int     mem;		/* /dev/mem pseudo fd		 */
    int     nmausent;		/* number of maus segments	 */
    int     status;		/* child process returns	 */
    char    rdchar;		/* char read */
    int     ret = PASS;

    signal (SIGSEGV, segv);

    /********************************
    *  find number of maus segments
    *********************************/
    if ((mem = open ("/dev/mem", O_RDONLY)) < 0)
    {
	printf("cannot open /dev/mem \n");
	exit (1);
    }
    lseek (mem, (long) nl[MAUS].n_value, 0);
    read (mem, &nmausent, sizeof (nmausent));
    fprintf (stderr, "    %d memory segments configured\n", nmausent);

    /********************************
    * loop through maus devices
    ********************************/
    for (i = 0; i < nmausent; ++i)
    {
	/******************************************
	* fork children for multiple process test
	******************************************/
	for (j = 0; j < CHILDREN; ++j)
	{
	    if (fork () == 0)
	    {
		sprintf (path, "/dev/maus%d", i);
		if ((mausdes = getmaus (path, oflag)) < 0)
		    if (errno == EINVAL || errno == ENXIO) {
			perror("getmaus");
			exit (FAIL);
		    }

		if (( maus = enabmaus (mausdes)) == -1) {
		    perror("enabmaus");
		    exit (FAIL);
		}

		/************************************
		*  read from data space until end
		*  (causes segmentation violation)
		*  changed from write to read since we don't want to screw
		*  up users using the maus segments. There is no way of 
		*  checking if others are using the same segments unless 
		*  everyone agrees on some locking mechanism.
		*************************************/
		segv_flg = 0;
		for (k = 0; segv_flg == 0; ++k)
		{
		    rdchar = *(maus + k);
		    setjmp (env);
		}
		if (j == 0)
		{
		    fprintf (stderr, "    %s is %d bytes\n", path, --k);
		    fflush (stdout);
		}

		/************************
		*  detach and free maus
		*************************/
		if (dismaus (maus) != mausdes){
		    perror("dismaus");
		    exit (FAIL);
		}
		if (freemaus (mausdes) != 0){
		    perror("freemaus");
		    exit (FAIL);
		}

		exit (PASS);
	    }
	}
	/*************************************************
	* wait for children & shift to get exit status
	*************************************************/
	for (j = 0; j < CHILDREN; ++j);
	{
	    wait (&status);
	    if ((status >> 8) == FAIL)
		ret = FAIL;
	}
    }


    /**************************************
    * allow children to complete printf's
    **************************************/
    sleep (2);

    return (ret);
}

/***************************************
*  handling segmentation violation from
*  write after end of maus data space
***************************************/
segv ()
{
    signal (SIGSEGV, segv);
    ++segv_flg;
    longjmp (env, 0);
}

/****************************************************
*  end of ipc_test.c
****************************************************/
