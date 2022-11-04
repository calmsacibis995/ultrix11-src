
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)elp.h	3.0	4/21/86
 */
/*
 * ULTRIX-11 header file for elp
 *
 * Fred Canter 2/8/83
 */

#include <sys/param.h>

/*
 * Misc. definitions for  error report generator (elp).
 */

#define	R	0
#define	W	1
#define	RW	2
#define	DCK	0100000
#define	ECH	0100
#define	RP04	020
#define	RP05	021
#define	RP06	022
#define	RM03	024
#define	RM02	025
#define	RM05	027
#define	ML11A	0110
#define	ML11B	0111


/*
 * The el_info structure is used by the error log
 * report generator program (elp).
 */

struct	el_info
{
	char	*et;		/* error type - "su", "rp", "hm", etc */
	char	ehm;		/* index into header message text */
	char	edn;		/* index into device name text */
	int	*erp;		/* pointer into device register text */
	int	*ernp;		/* pointer to device reg. name text */
	char	bmaj;		/* block mode major device number */
	char	rmaj;		/* raw mode major deivce number */
	int	tec;		/* total error count */
	int	*hec;		/* pointer to hard error count */
	int	*sec;		/* pointer to soft error count */
	int	rtc;		/* error recovery retry count */
} elc[];
int	*rk_rbp[];
int	rk_hec[];
int	rk_sec[];
int	*rp_rbp[];
int	rp_hec[];
int	rp_sec[];
int	*ra_rbp[];
int	*rl_rbp[];
int	rl_hec[];
int	rl_sec[];
int	*rx_rbp[];
int	rx_hec[];
int	rx_sec[];
int	*tm_rbp[];
int	tm_hec[];
int	tm_sec[];
int	*tk_rbp[];
int	tk_hec[];
int	tk_sec[];
int	*ts_rbp[];
int	ts_hec[];
int	ts_sec[];
int	*ht_rbp[];
int	ht_hec[];
int	ht_sec[];
int	*hp_rbp[];
int	hp_hec[];
int	hp_sec[];
int	*hp_rbp[];	/* XXX */
int	hm_hec[];
int	hm_sec[];
int	hj_hec[];
int	hj_sec[];
int	*hk_rbp[];
int	hk_hec[];
int	hk_sec[];
int	*ml_rbp[];
int	*rm_rbp[];
char	*rk_reg[];
char	*rp_reg[];
char	*ra_reg[];
char	*rl_reg[];
char	*rx_reg[];
char	*tm_reg[];
char	*tk_reg[];
char	*ts_reg[];
char	*ht_reg[];
char	*hp_reg[];
char	*rm_reg[];
char	*hk_reg[];
char	*ml_reg[];
char	*et_head[];
char	*dntab[];
char	*ra_ctn[];
int	ra_cid[];
char	*radntab[];

long	el_sb;		/* error log start block */
int	el_nb;		/* error log length in blocks */
#define	MAXNBD	13	/* maximum number of block devices ELP can deal with */
int	nblkdev;	/* actual number of block devices */
int	nchrdev;	/* maximum number of character devices */
/*
 * (elp) - control flags
 */

char	sflag;
char	fflag;
char	uflag;
char	bflag;
char	rflag;
char	dflag;
char	etflag;
char	fnflag;

/*
 * Error log time variables
 */

time_t	timbuf;
time_t	lotim;
time_t	hitim;
int	timl[];
int	timh[];
int	elrtim[];

int	buf[];	/* (elp) - read buffer */

int	ersum();
int	erfull();

int	et, etdn, etcn, etct;
int	badrn;
char	badet, badsz;
daddr_t	badbn;
