
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include <stdio.h>
#include <a.out.h>
#include <core.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/tty.h>
#include <sys/dir.h>

#include <sys/user.h>

static char Sccsid[] = "@(#)getu.c	3.0	4/21/86";

struct user u;
struct proc prc;
long sekpnt, psek;
int i, cnt, mem, val, *intpnt, onetime, nprc, rtn;
char *memfil = "/dev/mem";
char *nlfil = "/unix";

struct nlist nli[] = {
	{ "_proc" },
	{ "_nproc" },
	{ "" }
};
#define CHAR 1
#define INT  2
#define LONG 3
#define FLOAT 4
#define DOUBLE 5
#define STRING 6

struct {
	char *tnam;
	char *tfmt;
	int  tnum;
	int  ttyp;
}ustr[] = {
	"u_comm", "%-14s ", 1, STRING,
	"u_base", "%-6o ", 1, INT,
	"u_count", "%-6o ", 1, INT,
	"u_offset", "%-6O ", 1, LONG,
	"u_rsav", "%-6o ", 7, INT,
	"u_inemt", "%-6o ", 1, INT,
	"u_fpsaved", "%-6o ", 1, INT,
	"u_fps.u_fpsr", "%-6o ", 1, INT,
	"u_fps.u_fpregs", "%f ", 6, DOUBLE,
	"u_segflg", "%-6o ", 1, CHAR,
	"u_error", "%-6o ", 1, CHAR,
	"u_uid", "%-6o ", 1, INT,
	"u_gid", "%-6o ", 1, INT,
	"u_ruid", "%-6o ", 1, INT,
	"u_rgid", "%-6o ", 1, INT,
	"u_procp", "%-6o ", 1, INT,
	"u_ap", "%-6o ", 1, INT,
	"u_r.r_val1", "%-6o ", 1, INT,
	"u_r.r_val2", "%-6o ", 1, INT,
	"u_cdir", "%-6o ", 1, INT,
	"u_rdir", "%-6o ", 1, INT,
	"u_dbuf", "%-14s ", 1, STRING,
	"u_dirp", "%-6o ", 1, INT,
	"u_dent.d_name", "%-14s ", 1, STRING,
	"u_pdir", "%-6o ", 1, INT,
	"u_uisa", "%-6o ", 8, INT,
	"u_udsa", "%-6o ", 8, INT,
	"u_uisd", "%-6o ", 8, INT,
	"u_udsd", "%-6o ", 8, INT,
	"u_pofile", "%-6o ", NOFILE, CHAR,
	"u_ofile", "%-6o ", NOFILE, INT,
	"u_arg", "%-6o ", 5, INT,
	"u_tsize", "%-6o ", 1, INT,
	"u_dsize", "%-6o ", 1, INT,
	"u_ssize", "%-6o ", 1, INT,
	"u_qsav", "%-6o ", 7, INT,
	"u_ssav", "%-6o ", 7, INT,
	"u_signal", "%-6o ", NSIG, INT,
	"u_utime", "%-10O ", 1, LONG,
	"u_stime", "%-10O ", 1, LONG,
	"u_cutime", "%-10O ", 1, LONG,
	"u_cstime", "%-10O ", 1, LONG,
	"u_ar0", "%-6o ", 1, INT,
	"u_prof.pr_base", "%-6o ", 1, INT,
	"u_prof.pr_size", "%-6o ", 1, INT,
	"u_prof.pr_off", "%-6o ", 1, INT,
	"u_prof.pr_scale", "%-6o ", 1, INT,
	"u_intflg", "%-6o ", 1, CHAR,
	"u_sep", "%-6o ", 1, CHAR,
	"u_ttyp", "%-6o ", 1, INT,
	"u_ttyd", "%-6o ", 1, INT,
	"u_exdata.ux_mag", "%-6o ", 1, INT,
	"u_exdata.ux_tsize", "%-6o ", 1, INT,
	"u_exdata.ux_dsize", "%-6o ", 1, INT,
	"u_exdata.ux_bsize", "%-6o ", 1, INT,
	"u_exdata.ux_ssize", "%-6o ", 1, INT,
	"u_exdata.ux_entloc", "%-6o ", 1, INT,
	"u_exdata.ux_unused", "%-6o ", 1, INT,
	"u_exdata.ux_relflg", "%-6o ", 1, INT,
	"u_start", "%-10O ", 1, LONG,
	"u_acflag", "%-6o ", 1, CHAR,
	"u_fpflag", "%-6o ", 1, INT,
	"u_cmask", "%-6o ", 1, INT,
	"u_fperr.f_fec", "%-6o ", 1, INT,
	"u_fperr.f_fea", "%-6o ", 1, INT,
	"u_ovdata.uo_curov", "%-6o ", 1, INT,
	"u_ovdata.uo_ovbase", "%-6o ", 1, INT,
	"u_ovdata.uo_dbase", "%-6o ", 1, INT,
	"u_ovdata.uo_ov_offst", "%-6o ", 8, INT,
	"u_ovdata.uo_nseg", "%-6o ", 1, INT,
	"u_eosys", "%-6o", 1, CHAR,
	"", 0, 0, 0
};

int *uofst[] = {
	&u.u_comm,
	&u.u_base,
	&u.u_count,
	&u.u_offset,
	&u.u_rsav,
	&u.u_inemt,
	&u.u_fpsaved,
	&u.u_fps.u_fpsr,
	&u.u_fps.u_fpregs,
	&u.u_segflg,
	&u.u_error,
	&u.u_uid,
	&u.u_gid,
	&u.u_ruid,
	&u.u_rgid,
	&u.u_procp,
	&u.u_ap,
	&u.u_r.r_val1,
	&u.u_r.r_val2,
	&u.u_cdir,
	&u.u_rdir,
	&u.u_dbuf,
	&u.u_dirp,
	&u.u_dent.d_name,
	&u.u_pdir,
	&u.u_uisa[0],
	&u.u_uisa[8],
	&u.u_uisd[0],
	&u.u_uisd[8],
	&u.u_pofile,
	&u.u_ofile,
	&u.u_arg,
	&u.u_tsize,
	&u.u_dsize,
	&u.u_ssize,
	&u.u_qsav,
	&u.u_ssav,
	&u.u_signal,
	&u.u_utime,
	&u.u_stime,
	&u.u_cutime,
	&u.u_cstime,
	&u.u_ar0,
	&u.u_prof.pr_base,
	&u.u_prof.pr_size,
	&u.u_prof.pr_off,
	&u.u_prof.pr_scale,
	&u.u_intflg,
	&u.u_sep,
	&u.u_ttyp,
	&u.u_ttyd,
	&u.u_exdata.ux_mag,
	&u.u_exdata.ux_tsize,
	&u.u_exdata.ux_dsize,
	&u.u_exdata.ux_bsize,
	&u.u_exdata.ux_ssize,
	&u.u_exdata.ux_entloc,
	&u.u_exdata.ux_unused,
	&u.u_exdata.ux_relflg,
	&u.u_start,
	&u.u_acflag,
	&u.u_fpflag,
	&u.u_cmask,
	&u.u_fperr.f_fec,
	&u.u_fperr.f_fea,
	&u.u_ovdata.uo_curov,
	&u.u_ovdata.uo_ovbase,
	&u.u_ovdata.uo_dbase,
	&u.u_ovdata.uo_ov_offst,
	&u.u_ovdata.uo_nseg,
	&u.u_eosys,
	0
};


	int scnt, brkcnt, entcnt, tval, tflg;
	char tststr[30];
	char cval, *pcval;
	int ival, *pival;
	long lval, *plval;
	float fval, *pfval;
	double dval, *pdval;

main(argc, argp)
int argc;
char *argp[];
{
	char *ap;

	if(argc > 4) {
		printf("arg cnt ?\n");
		exit(1);
	}
    if(argc > 1){
	if(argp[1] [0] == '-') {
		if(argp[1][1] >= '0' && argp[1][1] <= '7'){
			sscanf(&argp[1][1], "%O", &sekpnt);
			sekpnt = sekpnt << 6;
			if(sekpnt < 0L){
			    printf("getu: %s invalid address\n", &argp[1][1]);
			    exit(1);
			}
			onetime++;
		}
	}
	if(onetime && argc == 4){
		memfil = argp[3];
		nlfil = argp[2];
	}
	else if(onetime && argc == 3)
		memfil = argp[2];
	else if(onetime == 0 && argc == 3){
		memfil = argp[2];
		nlfil = argp[1];
	}
	else if(onetime == 0 && argc == 2)
		memfil = argp[1];
   }
	nlist(nlfil, nli);
	if(nli[0].n_type == 0) {
		printf("\ngetu: Can't access namelist in %s\n", nlfil);
		exit(1);
	}
	if((mem = open(memfil, 0)) <= 0){
		printf("cannot open %s\n", memfil);
		exit(1);
	}
	if(onetime){
		printf("%-s %-s ",nlfil,memfil);
		printf(" at %O\t", sekpnt);
		fflush(stdout);
		system("date");
		fflush(stdout);
		showu(sekpnt);
		exit(0);
	}
	lseek(mem, (long)nli[1].n_value, 0);
	read(mem, &nprc, sizeof nprc);
	psek = (long)nli[0].n_value;
	lseek(mem, (long)nli[0].n_value, 0);
	for(i = 0; i < nprc; i++){
		lseek(mem, psek + (i * sizeof(struct proc)), 0);
		rtn = read(mem, &prc, sizeof(struct proc));
		if((prc.p_flag & SLOAD) == 0)
			continue;
		if(prc.p_stat == SZOMB)
			continue;
		printf("%c\r", '');
		if(i == 0){
			printf("%-s %-s ",nlfil,memfil);
			system("date");
		}
		sekpnt = (long)((unsigned)prc.p_addr);
		sekpnt = sekpnt << 6;
		showu(sekpnt);
	}
}

showu(pnt)
long pnt;
{
/*
	int scnt, brkcnt, entcnt, tval, tflg;
	char tststr[30];
	char cval, *pcval;
	int ival, *pival;
	long lval, *plval;
	float fval, *pfval;
	double dval, *pdval;
*/
	int i;
	if(onetime == 0){
		printf("address = %O pid = %d\n", pnt, prc.p_pid);
	}
	lseek(mem, pnt, 0);
	read(mem, &u, sizeof u);
	for(scnt = 0; *ustr[scnt].tnam; scnt++){
		brkcnt = 0;
		switch(ustr[scnt].ttyp){
			case CHAR:
				pcval = uofst[scnt];
				sprintf(&tststr, ustr[scnt].tfmt, (int)*pcval);
				break;
			case STRING:
				pcval = uofst[scnt];
				sprintf(&tststr, ustr[scnt].tfmt, pcval);
				break;
			case LONG:
				plval = uofst[scnt];
				sprintf(&tststr, ustr[scnt].tfmt, *plval);
				break;
			case FLOAT:
				pfval = uofst[scnt];
				sprintf(&tststr, ustr[scnt].tfmt, *pfval);
				break;
			case DOUBLE:
				pdval = uofst[scnt];
				sprintf(&tststr, ustr[scnt].tfmt, *pdval);
				break;
			case INT:
			default:
				pival = uofst[scnt];
				sprintf(&tststr, ustr[scnt].tfmt, *pival);
				break;
		}
		printf("%s", ustr[scnt].tnam);
		if(ustr[scnt].tnum > 1){
			printf("\n");
			tval = strlen(&tststr);
			tval *= ustr[scnt].tnum;
			if(tval > 76)
				brkcnt = (ustr[scnt].tnum/((tval/76)+1));
		}
		else
			printf(" = ");
		for(entcnt = 0, i = 0; entcnt < ustr[scnt].tnum; entcnt++, i++){
/*			if(brkcnt && entcnt == brkcnt) */
			if((i == brkcnt) && brkcnt) {
				printf("\n");
				i = 0;
			}
			switch(ustr[scnt].ttyp){
				case CHAR:
					ival = *pcval;
					printf(ustr[scnt].tfmt, ival);
					pcval++;
					break;
				case STRING:
					printf(ustr[scnt].tfmt, pcval);
					pcval++;
					break;
				case LONG:
					printf(ustr[scnt].tfmt, *plval);
					plval++;
					break;
				case FLOAT:
					printf(ustr[scnt].tfmt, *pfval);
					pfval++;
					break;
				case DOUBLE:
					printf(ustr[scnt].tfmt, *pdval);
					pdval++;
					break;
				case INT:
				default:
					printf(ustr[scnt].tfmt, *pival);
					pival++;
					break;
			}
		}
		printf("\n");
	}
	printf("\n\f");
}
