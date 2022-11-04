
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)trap.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/reg.h>
#include <sys/file.h>
#include <sys/seg.h>
#include <sys/errlog.h>
#include <sys/inline.h>

#define	EBIT	1		/* user error bit in PS: C-bit */
#define	SETD	0170011		/* SETD instruction */
#define	SYS	0104400		/* sys (trap) instruction */
#define	USER	020		/* user-mode flag added to dev */
#define	MEMORY	((physadr)0177740)	/* 11/70 "memory" subsystem */
#define	MPCSR	((physadr)0172100)	/* memory parity CSR base address */

/*
 * Error log buffer for memory parity errors
 */

struct
{
	int	m_nmsr;		/* number of memory system error reg.'s */
	int	m_mlea;		/* low error address */
	int	m_mhea;		/* hi  error address */
	int	m_mser;		/* memory system error reg. */
	int	m_mscr;		/* memory system control reg. */
	int	m_prcw;		/* parity CSR configuration word */
	int	m_pcsr[32];	/* parity CSR contents and error address */
} mp_ebuf = 0;

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[] =
{
	R0, R1, R2, R3, R4, R5, R6, R7, RPS, ROVN
};

int	osp;

/*
 * Called from l.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */

int	tnt = 0;	/* trap N trap */

#define	CD_SAVE	((physadr)040)	/* save crash info */

mapinfo	kernelmap;	/* saved mapping info on kernel-mode trap */
extern int fpemulation;

/*
 * 1/16/86 -- Fred Canter
 *	If non zero, fd_udno disables (default is enabled) the last
 *	minute fix for the partial file descriptor allocation bug,
 *	which occurs when an open() system call that has blocked
 *	gets interrupted (by an alarm in the test case).
 *	See Y2.4 field test QAR # 94 (/arch/qar)
 *	The reference to open() is just so we know its address.
 */
int	fd_undo = 0;
extern int open();

trap(dev, sp, r1, ov, nps, r0, pc, ps)
int *pc;
dev_t dev;
{
	register i;
	register *a;
	register struct sysent *callp;
	int (*fetch)();
	extern int fuword(), fuiword();
	time_t syst;
	int sz, j;
	int *opc;	/* save original pc in case we must restart syscall */
	struct	file	*fp;

	syst = u.u_stime;
	u.u_fpsaved = 0;
	if ((ps&(PS_CURMOD|PS_PRVMOD)) == (PS_CURMOD|PS_PRVMOD))
		dev |= USER;
	else
		savemap(kernelmap);
	u.u_ar0 = &r0;
	switch(minor(dev)) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * The numbers printed are used to
	 * find the hardware PS/PC as follows.
	 * (all numbers in octal 18 bits)
	 *	address_of_saved_ps =
	 *		(ka6*0100) + aps - 0140000;
	 *	address_of_saved_pc =
	 *		address_of_saved_ps - 2;
	 */
	default:
		if(tnt)		/* fatal trap - one and only one */
			for(;;) ;
		tnt++;
		printf("\nka6 = %o\n", ka6->r[0]);
		CD_SAVE->r[0] = &ps;
		CD_SAVE->r[1] = ka6->r[0];
		printf("aps = %o\n", &ps);
		printf("pc = %o ps = %o\n", pc, ps);
		if(_ovno >= 0)	/* if overlay kernel print overlay number */
			printf("ovno = %d\n", ov);
		printf("trap type %o\n", dev);
		panic("trap");

	case 0+USER: /* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 1+USER: /* illegal instruction */
		if (fpemulation == 1) {
			if ((i = fptrap(u.u_ar0, &u.u_fperr.f_fec)) == 0)
				goto out;	/* instruction was emulated */
			if (i == SIGTRAP)
				ps &= ~TBIT;
			break;
		}
		if (fuiword((caddr_t)(pc-1)) == SETD && u.u_signal[SIGILL] == 0)
			goto out;
		i = SIGILL;
		break;

	case 2+USER: /* bpt or trace */
		i = SIGTRAP;
		ps &= ~TBIT;
		break;

	case 3+USER: /* iot */
		i = SIGIOT;
		break;

	case 5+USER: /* emt */
		spl0();	/* the emt flag is set, no need for high pri */
		if(u.u_ovdata.uo_ovbase != 0 && r0 <= 7 && r0 > 0){
			ps &= ~EBIT;
			save(u.u_qsav);
			u.u_ovdata.uo_curov = r0;
			if(estabur(u.u_tsize,u.u_dsize,u.u_ssize,u.u_sep,RO)==0)
			{
				u.u_ar0[R0] = u.u_ovdata.uo_curov;
				u.u_inemt = 0;
				goto out;
			}
		}
		u.u_inemt = 0;
		i = SIGEMT;
		break;

	case 6+USER: /* sys call */
		u.u_error = 0;
		opc = pc - 1;		/* opc now points at syscall */
		i = fuiword((caddr_t)opc);
		callp = &sysent[i&0177];
		if (i & 0200) {		/* are argument(s) on the stack ? */
			a = sp;
			fetch = fuword;
		} else
			{
			a = pc;
			fetch = fuiword;
			pc += callp->sy_narg - callp->sy_nrarg;
		}
		if (callp == &sysent[SYSINDIR]) { /* indirect */
			a = (int *) (*fetch)((caddr_t)(a));
			i = fuword((caddr_t)a);
			a++;
			if (((i & ~0177) != SYS) || ((i &= 0177) > SYSMAX))
				i = SYSILLEGAL;
			callp = &sysent[i];
			fetch = fuword;
		} else if (callp == &sysent[BERKINDIR]) { /* Berkely indirect */
			a = (int *)(*fetch)((caddr_t)(a));
			i = fuword((caddr_t)a);
			a++;
			if (((i & ~077) != SYS) || ((i &= 077) > BERKMAX))
				i = SYSILLEGAL;
			else
				i += BERKOFFSET;
			callp = &sysent[i];
			fetch = fuword;
		}
		if (callp > &sysent[SYSMAX])
			callp = &sysent[SYSILLEGAL]; /* illegal */
		for (i=0; i<callp->sy_nrarg; i++)
			u.u_arg[i] = u.u_ar0[regloc[i]];
		for(; i<callp->sy_narg; i++)
			u.u_arg[i] = (*fetch)((caddr_t)a++);
		u.u_dirp = (caddr_t)u.u_arg[0];
		u.u_rval1 = u.u_ar0[R0];
		u.u_rval2 = u.u_ar0[R1];
		u.u_ap = u.u_arg;
		if (save(u.u_qsav)) {
			if (u.u_error == 0 && u.u_eosys == JUSTRETURN) {
			    u.u_error = EINTR;
			    /* 1/16/86 -- Fred Canter (start of new code) */
			    /* (see description above, just before of trap()) */
			    if((fd_undo == 0) && (callp->sy_call == &open)) {
				for(j=0; j<NOFILE; j++)
				    if(u.u_ofile[j] == 0)
					break;
				j--;
				fp = u.u_ofile[j];
				if(fp->f_flag&FPENDING) {
				    fp->f_count--;
				    fp->f_flag &= ~FPENDING;
				    u.u_ofile[j] = NULL;
				}
			    }
			    /* 1/16/86 -- Fred Canter (end of new code) */
			}
		} else {
			u.u_eosys = JUSTRETURN;
			(*callp->sy_call)();
		}
		if (u.u_eosys == RESTARTSYS)
			pc = opc;	/* backup pc to restart syscall */
		else if (u.u_eosys == SIMULATERTI)
			dorti(fuiword((caddr_t)opc) & 0200 ?
				callp->sy_narg - callp->sy_nrarg : 0);
		else
		if(u.u_error) {
			ps |= EBIT;
			u.u_ar0[R0] = u.u_error;
		} else {
			ps &= ~EBIT;
			u.u_ar0[R0] = u.u_rval1;
			u.u_ar0[R1] = u.u_rval2;
		}
		goto out;

#ifdef	FP
	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case 8: /* floating exception */
		stst(&u.u_fperr);	/* save error code */
		psignal(u.u_procp, SIGFPE);
		runrun++;
		return;

	case 8+USER:
		i = SIGFPE;
		stst(&u.u_fperr);
		break;
#endif	FP

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 11/40 this is not the case and
	 * the routine backup/l40.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case 9+USER: /* segmentation exception */
	{

		osp = sp;
		if(backup(u.u_ar0) == 0)
			if(grow((unsigned)osp))
				goto out;
		i = SIGSEGV;
		break;
	}

	/*
	 * If a memory parity error occurs in user mode
	 * log it, if one happens in kernel mode
	 * print the memory system registers and then `panic'.
	 * The 11/44, 11/60, & 11/70 are the only CPU's which
	 * have the memory system error registers.
	 * The other CPU's will have parity CSR's, if they
	 * have parity or ECC memory.
	 * The 11/44 & 11/60 have both, makes for nice clean code !
	 */
	case 10:
	case 10+USER:
#ifdef	PARITY
		mp_ebuf.m_nmsr = nmser;
		mp_ebuf.m_prcw = el_prcw;
		if(nmser) {
			if(nmser == 4) {
				mp_ebuf.m_mlea = MEMORY->r[0];
				mp_ebuf.m_mhea = MEMORY->r[1];
				if((dev & USER) == 0) {
					printf("\nMLEA %o", mp_ebuf.m_mlea);
					printf("\nMHEA %o", mp_ebuf.m_mhea);
					}
				}
			mp_ebuf.m_mser = MEMORY->r[2];
			mp_ebuf.m_mscr = MEMORY->r[3];
			MEMORY->r[2] = mp_ebuf.m_mser;	/* clear error bits */
			if((dev & USER) == 0) {
				printf("\nMSER %o", mp_ebuf.m_mser);
				printf("\nMSCR %o\n", mp_ebuf.m_mscr);
				}
			}
		j = 0;
		for(i=0; i<16; i++)
			if(el_prcw & (1 << i)) {
				mp_ebuf.m_pcsr[j] = MPCSR->r[i];
				MPCSR->r[i] |= 040000; /* get ext. addr if any */
				j++;
/* below line replaced to eliminate new c compiler warning message
   regarding e_pcsr not being a member of mp_buf struct. OHMS 4/30/85 */
	/*	mp_ebuf.m_pcsr[j] = (mp_ebuf.e_pcsr[j-1] >> 5) & 0177;   */
		mp_ebuf.m_pcsr[j] = (mp_ebuf.m_pcsr[j-1] >> 5) & 0177;
				mp_ebuf.m_pcsr[j] |= (MPCSR->r[i] << 2) & 03600;
				j++;
				}
		sz = 12 + (j * 2);
		if((dev & USER) == 0) {
			if(el_prcw)
				printf("\nCSR\tDATA\tA21->A11\n");
			for(i=0; i<16; i++)
				if(el_prcw & (1 << i))
/* below line replaced to eliminate new c compiler warning message
   regarding e_pcsr not being a member of mp_buf struct. OHMS 4/30/85 */
/*  printf("%d\t%o\t%o\n", i, mp_ebuf.m_pcsr[i*2], mp_ebuf.e_pcsr[(i*2)+1]);  */
    printf("%d\t%o\t%o\n", i, mp_ebuf.m_pcsr[i*2], mp_ebuf.m_pcsr[(i*2)+1]);
			}
/*
 * Attempt to log the error even though a panic
 * parity could occur before it gets written
 * out to the error log file.
 * This saves the memory error register contents
 * in the kernel error log buffer for later
 * crash dump analysis.
 */
			logerr(E_MP, &mp_ebuf, sz);
#endif	PARITY
			if(dev & USER) {
/*
 * Send SIGKIL to process instead of 
 * SIGBUS because SIGBUS causes a core dump
 * which will generate another parity
 * error, UPE in the disk controller.
 */
				i = SIGKILL;
				break;
			}
		panic("parity");

	/*
	 * Allow process switch
	 */
	case USER+12:
		goto out;

	/*
	 * FOLLOWING IS HISTORY -- Fred
	 * Locations 0-2 specify this style trap, since
	 * DEC hardware often generates spurious
	 * traps through location 0.  This is a
	 * symptom of hardware problems and may
	 * represent a real interrupt that got
	 * sent to the wrong place.  Watch out
	 * for hangs on disk completion if this message appears.
	case 15:
	case 15+USER:
		printf("Random interrupt ignored\n");
		return;
*/
	}
    {
	long sigmask = 1L << (i - 1);
	if (!(u.u_procp->p_ignsig & sigmask) && (u.u_signal[i] != SIG_DFL)
	    && !(u.u_procp->p_flag & STRC))
		sendsig(u.u_signal[i],i);
	else {
		psignal(u.u_procp, i);
	}
    }

out:
	if (u.u_inemt==0 && (u.u_procp->p_cursig || ISSIG(u.u_procp)))
		psig();
	curpri = setpri(u.u_procp);
	if (runrun)
		qswtch();
	if(u.u_prof.pr_scale)
		addupc((caddr_t)pc, &u.u_prof, (int)(u.u_stime-syst));
#ifdef	FP
	if (u.u_fpsaved)
		restfp(&u.u_fps);
#endif	FP
}

/*
 * nonexistent system call-- set fatal error code.
 */
nosys()
{
	u.u_error = EINVAL;
}

/*
 * Ignored system call
 */
nullsys()
{
}
