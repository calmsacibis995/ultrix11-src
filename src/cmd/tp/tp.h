/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*	c-version of tp?.s
 *	SCCSID: @(#)tp.h	3.0	4/22/86
 *
 * Chung-Wu Lee, Oct-23-85
 *
 *	start supporting rx50 and 6250 BPI (gt) magtape.
 *
 * Chung-Wu Lee, Oct-15-85
 *
 *	start supporting TK50.
 *
 *	M. Ferentz
 *	August 1976
 *
 *	revised July 1977 BTL
 */

#define	MDIRENT	496		/* magtape - must be zero mod 8 */
#define	KDIRENT	992		/* TK50 - must be zero mod 8 */
#define	XDIRENT	248		/* rx50 - must be zero mod 8 */
#define DIRSZ	sizeof(struct dent)
#define MAPSIZE 4096
#define MAPMASK 07777
#define NAMELEN 32
#define BSIZE   512
/* #define	TCSIZ	578
 * #define TCDIRS	192	not supported */
#define	MTSIZ	32767
#define	TKSIZ	32767
#define	RXSIZ	800
#define TPB	(BSIZE/sizeof(struct tent))
#define	OK	0100000
#define	BRKINCR	512

#define	tapeblk	&tpentry[0]
#define tapeb	&tpentry[0]

struct 	tent	{	/* Structure of a tape directory block */
	char	pathnam[NAMELEN];
	short	mode;
	char	uid;
	char	gid;
	char	spare;
	char	size0;
	unsigned short	size1;
	long	time;
	unsigned short	tapea;	/* tape address */
	short	unused[8];
	short	cksum;
}	tpentry[TPB];

struct	dent {	/* in core version of tent with "unused" removed
		 * and pathname replaced by pointer to same in a
		 * packed area (nameblock).
		 */
	char	*d_namep;
	int	d_mode;
	int	d_uid;
	int	d_gid;
	long	d_size;
	long	d_time;
	int	d_tapea;
}  dir[KDIRENT];

char	map[MAPSIZE];
char	name[NAMELEN];
char	name1[NAMELEN];
extern	char mt[];
extern	char ht[];
extern	char gt[];
extern	char tk[];
extern	char rx[];
/* extern	char tc[];	not supported */
char	*tname;
extern	char mheader[];
extern	char theader[];

int	narg, rnarg;
char	**parg;
int	wseeka,rseeka;
int	tapsiz;
int	tapdir;
int	fio;
short	ndirent, ndentb;
struct	dent	*edir;
struct	dent *lastd;		/* for improvement */
char	*sbrk();
char	*strcpy();
char	*strncpy();
long	lseek();
int	(*command)();

char	*nameblk;
char	*top;
char	*nptr;

extern	int	flags;
#define	flc	0001
#define	fli	0004
#define	flm	0010
#define	flu	0020
#define	flv	0040
#define	flw	0100
#define fls	0200
#define flk	0400
