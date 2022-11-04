/*
 * SCCSID: @(#)nlist.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include <a.out.h>
#ifndef	K450
int a_magic[] = {A_MAGIC1, A_MAGIC2, A_MAGIC3, A_MAGIC4, 0430, 0431, 0};
#else	K450
int a_magic[] = {A_MAGIC1, A_MAGIC2, A_MAGIC3, A_MAGIC4, 0430, 0431, 0450, 0451, 0};
#endif	K450
#define SPACE 100               /* number of symbols read at a time */

struct exec buf;
struct nlist space[SPACE];

nlist(name, list)
char *name;
struct nlist *list;
{
        register struct nlist *p, *q;
        int f, n, m, i;
        long sa;
	unsigned ovsizes[8];
	long soff;

        for(p = list; p->n_name[0]; p++) {
                p->n_type = 0;
                p->n_value = 0;
        }
        f = open(name, 0);
        if(f < 0)
                return(-1);
	lseek(f, (long)0, 0);
        read(f, (char *)&buf, sizeof buf);
	soff = sizeof buf;
        for(i=0; a_magic[i]; i++)
                if(a_magic[i] == buf.a_magic) break;
        if(a_magic[i] == 0){
                close(f);
                return(-1);
        }
#ifndef	K450
	if (buf.a_magic == 0430 || buf.a_magic == 0431) {
#else	K450
	if (buf.a_magic >= 0430) {
#endif	K450
		read(f, (char *)ovsizes, sizeof ovsizes);
		soff += sizeof ovsizes;
		for (i = 1; i < 8; i++) {
#ifndef	K450
			lseek(f, (long)(soff+ovsizes[i]), 0);
#endif	K450
			soff += ovsizes[i];
		}
	}
#ifdef	K450
	if (buf.a_magic >= 0450) {
		read(f, (char *)ovsizes, sizeof ovsizes);
		soff += sizeof ovsizes;
		for (i = 0; i < 8; i++)
			soff += ovsizes[i];
	}
#endif	K450
        sa = buf.a_text + (long)buf.a_data;
        if(buf.a_flag != 1) sa *= 2;
        lseek(f, (long)(sa+soff), 0);
	soff += sa;
        n = buf.a_syms;

        while(n){
                m = sizeof space;
                if(n < sizeof space)
                        m = n;
                read(f, (char *)space, m);
                n -= m;
                for(q = space; (m -= sizeof(struct nlist)) >= 0; q++) {
                        for(p = list; p->n_name[0]; p++) {
                                for(i=0;i<8;i++)
                                        if(p->n_name[i] != q->n_name[i]) goto cont;
                                p->n_value = q->n_value;
                                p->n_type = q->n_type;
                                break;
                cont:           ;
                        }
                }
        }
        close(f);
        return(0);
}
