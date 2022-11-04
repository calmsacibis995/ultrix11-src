
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 MSCP disk file sises print command (rasize)
 *
 * Fred Canter
 *
 * This command tells the gives the user the following information:
 *
 * 1.	The type and micro-code revision level of each MSCP
 *	controller present on the system.
 *
 * 2.	The drive type of each available unit.
 *
 * 3.	Which file systems on that unit may be used.
 *
 * 4.	The size to be supplied to `/etc/mkfs' when making a
 *	file system on any usable disk partition.
 *
 * This is necessary because the total number of logical blocks on
 * each volume is not fixed, i.e., it is read from the disk when it
 * is placed on-line. Also the last (1000, 102, 32) logical blocks of each disk
 * are reserved for a maintenance area so that the RA disk exerciser
 * will always have a free fire zone on the disk, where it can write
 * without destroying the customer's data.
 *
 *	*************************************************
 *	*						*
 *	*	Assumes namelist in `/unix' and		*
 *	*	core is `/dev/mem'.			*
 *	*						*
 *	*						*
 *	*************************************************
 *
 */

static char Sccsid[] = "@(#)rasize.c 3.1 3/26/87";
#include <sys/param.h>
#include <sys/devmaj.h>
#include <sys/ra_info.h>
#include <stdio.h>
#include <a.out.h>

struct	nlist	nl[] =
{
	{ "_ra_size" },
	{ "_ra_drv" },
	{ "_ra_ctid" },
	{ "_rootdev" },
	{ "_swapdev" },
	{ "_el_dev" },
	{ "_swplo" },
	{ "_nswap" },
	{ "_el_sb" },
	{ "_el_nb" },
	{ "_ra_mas" },
	{ "_nuda" },
	{ "_nra" },
	{ "_ra_inde" },
	{ "" },
};

int	rootdev;
int	swapdev;
int	el_dev;
daddr_t	swplo;
int	nswap;
daddr_t	el_sb;
int	el_nb;

int	ra_saddr[MAXUDA];
struct rasize rasizes[MAXUDA][8];

struct	ra_drv	ra_drv[MAXUDA*8];

int	nuda;		/* number of configured MSCP controllers */
char	nra[MAXUDA];	/* number of drives configured per controller */
char	ra_ctid[MAXUDA];	/* controller type ID and micro-code revision */
daddr_t	ra_mas[MAXUDA];	/* RA maint. area size */
char	ra_index[MAXUDA];	/* index into non-semetrical arrays (ra_drv) */
int	raunits;	/* total number of units */

char	fn[50];

main()
{
	register int	i, j, k;
	daddr_t	sz;
	int	mem;
	int	fi;
	char	*p;
	int	drv_root, drv_swap, drv_elog;
	long	n;

	nlist("/unix", nl);
	if((nl[0].n_type == 0) || (nl[1].n_type == 0) || (nl[11].n_type == 0)) {
no_mscp:
		printf("\nrasize: /unix not configured for MSCP disks !\n");
		exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		printf("\nrasize: Can't open /dev/mem\n");
		exit(1);
	}
/*
 * Get # controllers, type of each,
 * and # drives on each.
 */
	lseek(mem, (long)nl[11].n_value, 0);	/* nuda */
	read(mem, (char *)&nuda, sizeof(nuda));
	if(nuda == 0)
		goto no_mscp;
	lseek(mem, (long)nl[12].n_value, 0);	/* nra */
	read(mem, (char *)&nra, nuda);
	raunits = 0;
	for(i=0; i<nuda; i++)
		raunits += nra[i];
	lseek(mem, (long)nl[13].n_value, 0);	/* ra_index */
	read(mem, (char *)&ra_index, MAXUDA);
	lseek(mem, (long)nl[2].n_value, 0);	/* ra_ctid */
	read(mem, (char *)&ra_ctid, MAXUDA);
	lseek(mem, (long)nl[1].n_value, 0);	/* ra_drv */
	read(mem, (char *)&ra_drv, sizeof(struct ra_drv) * raunits);
/*
 * Update ra_drv[] by attempting to open
 * any drive that is not on-line.
 */
	for(k=0; k<nuda; k++)
		for(i=0; i<nra[k]; i++) {
			if(ra_drv[ra_index[k]+i].ra_online)
				continue;
			switch((ra_ctid[k]>>4)&017) {
/*			case KDA25:	*/
			case KDA50:
			case UDA50:
			case UDA50A:
				sprintf(&fn, "/dev/ra%o7", i);
				j = open(fn, 0);
				close(j);
				break;
			case KLESI:
				sprintf(&fn, "/dev/rc%o7", i);
				j = open(fn, 0);
				close(j);
				break;
			case RQDX1:
			case RQDX3:
				sprintf(&fn, "/dev/rx%o", i);
				if(ra_drv[ra_index[k]+i].ra_dt == 0) {
					j = open(fn, 0);
					close(j);
				}
				sprintf(&fn, "/dev/rd%o7", i);
				j = open(fn, 0);
				close(j);
				break;
			case RUX1:
				sprintf(&fn, "/dev/rx%o", i);
				if(ra_drv[ra_index[k]+i].ra_dt == 0) {
					j = open(fn, 0);
					close(j);
				}
				break;
			}
		}
/*
 * get updated drive types
 */
	lseek(mem, (long)nl[1].n_value, 0);	/* ra_drv */
	read(mem, (char *)&ra_drv, sizeof(struct ra_drv) * raunits);
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&ra_saddr, sizeof(int) * nuda);
	for(i=0; i<nuda; i++) {
		lseek(mem, (long)ra_saddr[i], 0);
		read(mem, (char *)&rasizes[i][0], sizeof(rasizes)/MAXUDA);
	}
	lseek(mem, (long)nl[10].n_value, 0);
	read(mem, (char *)&ra_mas, sizeof(long) * nuda);
	lseek(mem, (long)nl[3].n_value, 0);
	read(mem, (char *)&rootdev, sizeof(rootdev));
	lseek(mem, (long)nl[4].n_value, 0);
	read(mem, (char *)&swapdev, sizeof(swapdev));
	lseek(mem, (long)nl[5].n_value, 0);
	read(mem, (char *)&el_dev, sizeof(el_dev));
	lseek(mem, (long)nl[6].n_value, 0);
	read(mem, (char *)&swplo, sizeof(swplo));
	swplo++;	/* kernel actually uses swplo - 1 */
	lseek(mem, (long)nl[7].n_value, 0);
	read(mem, (char *)&nswap, sizeof(nswap));
	lseek(mem, (long)nl[8].n_value, 0);
	read(mem, (char *)&el_sb, sizeof(el_sb));
	lseek(mem, (long)nl[9].n_value, 0);
	read(mem, (char *)&el_nb, sizeof(el_nb));
	k = 0;
loop:
	i = (ra_ctid[k] >> 4) & 017;
	switch(i) {
	case UDA50:
		p = "UDA50";
		break;
	case KDA50:
		p = "KDA50";
		break;
/*	case KDA25:		*/
/*		p = "KDA25";	*/
/*		break;	*/
	case KLESI:
		p = "KLESI";
		break;
	case UDA50A:
		p = "UDA50A";
		break;
	case RQDX1:
		p = "RQDX1";
		break;
	case RQDX3:
		p = "RQDX3";
		break;
	case RUX1:
		p = "RUX1";
		break;
	default:
		p = "UNKNOWN";
	}
	j = ra_ctid[k] & 017;
	printf("\n\n\t****** Controller %d is %s ", k, p);
	printf("at Micro-code Revision %d ******", j);
	printf("\n\nUnit\tDrive\t-------- File System Size (512 byte ");
	printf("physical blocks) ---------");
	printf("\nNumber\tType\t0\t1\t2\t3\t4\t5\t6\t7\n");
	for(i=0; i<nra[k]; i++) {
		/*
		 * Find out if root, swap, or error log in this drive.
		 */
		drv_root = drv_swap = drv_elog = 0;
		if((rootdev & ~7) == ((RA_BMAJ << 8) | (k << 6) | (i << 3)))
			drv_root++;
		if((swapdev & ~7) == ((RA_BMAJ << 8) | (k << 6) | (i << 3)))
			drv_swap++;
		if((el_dev & ~7) == ((RA_RMAJ << 8) | (k << 6) | (i << 3)))
			drv_elog++;
		printf("\n%d\t", i);
		switch(ra_drv[ra_index[k]+i].ra_dt) {
		case RC25:
			printf("RC25\t");
			break;
		case RX33:
			printf("RX33\t");
			break;
		case RX50:
			printf("RX50\t");
			break;
		case RD31:
			printf("RD31\t");
			break;
		case RD32:
			printf("RD32\t");
			break;
		case RD51:
		case RD52:
		case RD53:
		case RD54:
			printf("RD%d\t", ra_drv[ra_index[k]+i].ra_dt);
			break;
		case RA60:
		case RA80:
		case RA81:
			printf("RA%d\t", ra_drv[ra_index[k]+i].ra_dt);
			break;
		default:
			printf("NED\t");
			break;
		}
		for(j=0; j<8; j++) {
			if((ra_drv[ra_index[k]+i].ra_dt != 0) &&
			   (ra_drv[ra_index[k]+i].ra_dt != RX33) &&
			   (ra_drv[ra_index[k]+i].ra_dt != RX50) &&
			   (ra_drv[ra_index[k]+i].ra_online == 0)) {
				printf("?\t");
				continue;
			}
			switch(ra_drv[ra_index[k]+i].ra_dt) {
			case 0:
				printf("X\t");
				break;
			case RX50:
				if(j != 7)
					printf("X\t");
				else
					printf("800\t");
				break;
			case RX33:
				if(j != 7)
					printf("X\t");
				else
					printf("2400\t");
				break;
			case RD32:
			case RD51:
			case RD52:
			case RD53:
			case RD54:
				if((j == 5) || (j == 6))
				    printf("X\t");
				else if((j == 1) || (j == 2)) {
				    if(ra_drv[ra_index[k]+i].ra_dt == 51)
					    printf("X\t");
				    else
					printf("%D\t", rasizes[k][j].nblocks);
				} else if(j == 3) {
				    if(ra_drv[ra_index[k]+i].ra_dt == 51)
					    printf("X\t");
				    else
					    printf("%D\t",
		ra_drv[ra_index[k]+i].d_un.ra_dsize-rasizes[k][j].blkoff-(long)ra_mas[k]);
				} else if(j == 4) {
				    if(ra_drv[ra_index[k]+i].ra_dt != 51)
					    printf("X\t");
				    else
					    printf("%D\t",
		ra_drv[ra_index[k]+i].d_un.ra_dsize-rasizes[k][j].blkoff-(long)ra_mas[k]);
				} else if(j == 7) {
				    if(drv_root || drv_swap || drv_elog)
					printf("*");
			    printf("%D\t",ra_drv[ra_index[k]+i].d_un.ra_dsize-ra_mas[k]);
				} else {
				    if((ra_drv[ra_index[k]+i].ra_dt == 51) &&
				      (drv_root || drv_swap || drv_elog)) {
					if(el_sb < swplo)
						n = el_sb;
					else
						n = swplo;
					printf("%D\t", n);
				    } else
					printf("%D\t", rasizes[k][j].nblocks);
				}
				break;
			case RD31:
				if((j >= 1) && (j <= 4))
				    printf("X\t");
				else if(j == 5) {
				    printf("%D\t", rasizes[k][j].nblocks);
				} else if(j == 6) {
					printf("%D\t",
	ra_drv[ra_index[k]+i].d_un.ra_dsize-rasizes[k][j].blkoff-(long)ra_mas[k]);
				} else if(j == 7) {
				    if(drv_root || drv_swap || drv_elog)
					printf("*");
			    printf("%D\t",ra_drv[ra_index[k]+i].d_un.ra_dsize-ra_mas[k]);
				} else {
					printf("%D\t", rasizes[k][j].nblocks);
				}
				break;
			case RC25:
				if((j >= 3) && (j < 7))
					printf("X\t");
				else if(rasizes[k][j].nblocks > 0)
					printf("%D\t", rasizes[k][j].nblocks);
				else
					goto prsiz;
				break;
			case RA80:
				if((j == 4) || (j == 5) || (j == 6))
					printf("X\t");
				else if(rasizes[k][j].nblocks > 0)
					printf("%D\t", rasizes[k][j].nblocks);
				else {
				prsiz:
					if((j==7)&&(drv_root||drv_swap||drv_elog))
						printf("*");
					sz = ra_drv[ra_index[k]+i].d_un.ra_dsize 
						- (long)ra_mas[k]
						- rasizes[k][j].blkoff;
					printf("%D\t", sz);
				}
				break;
			case RA60:
				if(j == 6)
					printf("X\t");
				else if(rasizes[k][j].nblocks > 0)
					printf("%D\t", rasizes[k][j].nblocks);
				else
					goto prsiz;
				break;
			case RA81:
				if(rasizes[k][j].nblocks > 0)
					printf("%D\t", rasizes[k][j].nblocks);
				else
					goto prsiz;
				break;
			default:
				printf("\nrasize: Unknown drive type !\n");
				exit(1);
			}
		}
	}
	if(++k < nuda)
		goto loop;
	printf("\n\n? = Disk is OFFLINE, sizes cannot be determined.");
	printf("\n\nX = Disk partition is not used.");
	printf("\n\n* = DO NOT USE, file system would overwrite ");
	printf("ROOT, SWAP, or ERROR LOG.");
	printf("\n\n");
}
