
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ipatch - patch inodes
 *
 *	Written 11-12-80 by Bill Shannon - DEC
 *	based on ipatch by Tom Duff, converted to V7
 *	Modified for Unix/v7m 10-16-82 by Fred Canter
 *
 * Usage:
 *		/etc/ipatch dev inode
 *
 *		dev	- file system, i.e., /dev/rhp00
 *		inode	- inode number
 * Prompt:
 *		.
 *
 * Commands:
 *		q - quit
 *		p - print inode contents
 *		f - modify flags
 *		l - modify link count
 *		u - modify uid
 *		g - modify gid
 *		s - modify size
 *		a - modify addr (a naddr addr)
 *		w - write patched inode to file system
 *		! - shell escape (DOES NOT WORK !)
 *
 *	Only one inode may be patched/examined per ipatch execution.
 *
 *	Examining inodes may be done at will, HOWEVER patching can be
 *	very dangerous and must only be done on dismounted file systems.
 */
static char Sccsid[] = "@(#)ipatch.c 3.0 4/21/86";
#include <sys/param.h>	/* does not matter which one ! */
#include <sys/ino.h>
#include <sys/filsys.h>
#include <setjmp.h>
#include <signal.h>

struct filsys sb;

struct dinode *dp;

char buf[BSIZE];

#ifdef	UCB_NKB
#define	NADDR	7
#else
#define	NADDR	13
#endif	UCB_NKB

daddr_t i_addr[NADDR];

jmp_buf env;

int	ibo;
daddr_t block;
ino_t inum;
int rdonly;
int f;
int pflag;
char lastc;
char peekc = '\0';
getc(){
	if(peekc!='\0'){
		lastc=peekc;
		peekc='\0';
	}
	else if(read(0, &lastc, 1)!=1)
		lastc='\0';
	return(lastc);
}
endline(){
	register char c;
	while(any(c=getc(), " \t"));
	if(c=='p'){
		pflag++;
		while(any(c=getc(), " \t"));
	}
	if(c!='\n')
		error();
}
error(){
	printf("\7?\n");
	if(lastc!='\n')
		while(getc()!='\n');
	longjmp(env, 1);
}
any(ac, as)
char as[];
{
	register char c, *s;
	c=ac;
	s=as;
	while(*s)
		if(c == *s++)
			return(1);
	return(0);
}
intr(){
	signal(SIGINT, intr);
	lastc='\n'; /* cryptic */
	error();
}
main(argc, argv)
char *argv[];
{
	register j, i;
	int n;
	char c;
	int savintr, savquit;
	extern char *ctime();

	if(argc!=3){
		write(2,"arg count\n",10);
		exit(1);
	}
	if((f=open(argv[1],2))<0){
		if ((f = open(argv[1], 0)) < 0) {
			perror(argv[1]);
			exit(1);
		}
		rdonly++;
		write(2, "read only\n", 10);
	}
	lseek(f, (off_t)BSIZE, 0);
	if (read(f, (char *)&sb, sizeof sb) != sizeof sb) {
		perror("super block");
		exit(1);
	}
	if(!iget(inum=atoi(argv[2]))){
		write(2, "can't read inode\n", 16);
		exit(1);
	}
	setjmp(env);
	signal(SIGINT, intr);
	for(;;){
		write(2,".",1);
		while(any(c=getc(), " \t"));
		switch(c){
		default:
			error();
		case 'q':
			endline();
		case '\0':
			exit(0);
		case '\n':
			break;
		case 'e':
			i=rin();
			endline();
			if(iget(i))
				inum=i;
			else
				error();
			break;
		case '!':
/*
			savintr=signal(INTR, 1);
			savquit=signal(QUIT, 1);
			i=fork();
			if(i== -1)
				printf("Try again.\n");
			else if(i==0){
				execl("/bin/sh", "sh", "-t", 0);
				perror("/bin/sh");
				exit(1);
			}
			else{
				wait();
				printf("!\n");
				signal(INTR, savintr);
				signal(QUIT, savquit);
			}
*/
			break;
		case 'p':
			endline();
			pflag++;
			break;
		case 'f':
			i=rin();
			endline();
			dp->di_mode = i;
			break;
		case 'l':
			i=rin();
			endline();
			dp->di_nlink=i;
			break;
		case 'u':
			i=rin();
			endline();
			dp->di_uid=i;
			break;
		case 'g':
			i=rin();
			endline();
			dp->di_gid=i;
			break;
		case 's':
			i=rin();
			endline();
			dp->di_size = i;
			break;
		case 'a':
			n=rin();
			if (n < 0 || n >= NADDR)
				error();
			i=rin();
			endline();
			i_addr[n]=i;
			break;
		case 'w':
			endline();
			iput();
			break;
		}
		if(pflag){
			pflag=0;
			printf("\nFile System\tInode\tBlock\tOffset");
			printf("\n%s\t%u\t%D\t%d\n\n",argv[1],inum,block,ibo);
			printf("flags: %o links: %d uid: %d gid: %d  size: %ld (0%lo)\n",
				dp->di_mode, dp->di_nlink,
				dp->di_uid, dp->di_gid,
				dp->di_size,
				dp->di_size);
			printf("addr:\n");
			for(i=0;i<NADDR;i++)
				printf("%ld\t(0%lo)\n",i_addr[i],i_addr[i]);
			printf("accessed: %s", ctime(&dp->di_atime));
			printf("modified: %s",ctime(&dp->di_mtime));
			printf("created:  %s",ctime(&dp->di_ctime));
		}
	}
}
rin(){
	char c;
	register n, base;
	n=0;
	while(any(c=getc(), " \t"));
	if(c<'0' || c>'9'){
		peekc=c;
		error();
	}
	base=c=='0'?8:10;
	do
		n=n*base+c-'0';
	while('0'<=(c=getc()) && c<='9');
	peekc=c;
	return(n);
}
iget(inum)
{
	register nread;
	struct dinode *p;
	char *p1, *p2;
	int i;

	block = itod(inum);
	if(block>=sb.s_isize+2 || block<2)
		return(0);
	lseek(f, (off_t)(block*BSIZE), 0);
	if (read(f, buf, sizeof buf) != sizeof buf) {
		perror("iget");
		return(0);
	}
	p = (struct dinode *)buf;
	ibo = itoo(inum);
	p += ibo;
	dp = p;		/* save for later */
	p1 = (char *)i_addr;
	p2 = (char *)dp->di_addr;
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
	return(1);
}


iput()
{
	register char *p1, *p2;
	register int i;

	if (rdonly) {
		write(2, "read only\n", 10);
		return;
	}
	p1 = (char *)dp->di_addr;
	p2 = (char *)i_addr;
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		if(*p2++ != 0)
			printf("iaddress > 2^24\n");
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
	lseek(f, (off_t)(block*BSIZE), 0);
	if (write(f, buf, sizeof buf) != sizeof buf)
		write(2, "write error\n", 12);
}
