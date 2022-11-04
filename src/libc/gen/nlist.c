
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)nlist.c	3.0	4/22/86
 */
#include <a.out.h>
int a_magic[] = {
	A_MAGIC1, A_MAGIC2, A_MAGIC3,
	A_MAGIC4, A_MAGIC6, A_MAGIC7,
	A_MAGIC8, A_MAGIC9, 0
};
#define SPACE 100               /* number of symbols read at a time */

nlist(name, list)
char *name;
struct nlist *list;
{
        register struct nlist *p, *q;
        int f, n, m, i;
        long sa;
        struct exec buf;
        struct nlist space[SPACE];
	unsigned ovsizes[8];

        for(p = list; p->n_name[0]; p++) {
                p->n_type = 0;
                p->n_value = 0;
        }
        f = open(name, 0);
        if(f < 0)
                return(-1);
        read(f, (char *)&buf, sizeof buf);
        for(i=0; a_magic[i]; i++)
                if(a_magic[i] == buf.a_magic) break;
        if(a_magic[i] == 0){
                close(f);
                return(-1);
        }
	/*
	 * sa will eventually contain the offset into the file to get
	 * to the symbol table.  The offset is at:
	 *   header+ovhdr+ovhdr2+(relocatable?2:1)*(text+data)+overlays
	 * We do the text+data first, then the headers & overlays.
	 */
        sa = buf.a_text + (long)buf.a_data;
        if(buf.a_flag != 1)
		sa *= 2;
	sa += sizeof buf;
	if (buf.a_magic >= 0430) {
		read(f, (char *)ovsizes, sizeof ovsizes);
		sa += sizeof ovsizes;
		for (i = 1; i < 8; i++)
			sa += ovsizes[i];
	}
	if (buf.a_magic >= 0450) {
		read(f, (char *)ovsizes, sizeof ovsizes);
		sa += sizeof ovsizes;
		for (i = 0; i < 8; i++)
			sa += ovsizes[i];
	}
        lseek(f, sa, 0);
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
