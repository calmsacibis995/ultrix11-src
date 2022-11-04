
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)malloc.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>

/*
 *
 * Ohms 4/1/85
 *
 * The maps have changed. System V compatability routines (messages and 
 * semaphores) use malloc amd mfree to manage their local maps.
 * The first location in a map no longer contains normal map information.
 *
 * SYSTEM V macros:
 * 	MAPSTART macro chooses the first real map entry.
 * 	MAPSIZE is used to detect a full map in SYSV. We stick with our method.
 * 	MAPWANT is used for wakeups.
 */

/*
 * Allocate 'size' units from the given
 * map. Return the base of the allocated
 * space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 * The core map unit is 64 bytes; the swap map unit
 * is 512 bytes.
 * Algorithm is first-fit.
 */
malloc(mp, size)
struct map *mp;
unsigned int size;
{
	register unsigned int a;
	register struct map *bp;

 	for (bp = MAPSTART(mp); bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
 				MAPSIZE(mp)++;
			}
			return(a);
		}
	}
	return(0);
}

/*
 * Free the previously allocated space aa
 * of size units into the specified map.
 * Sort aa into map and combine on
 * one or both ends if possible.
 */
mfree(mp, size, a)
struct map *mp;
unsigned int size;
register unsigned int a;
{
	register struct map *bp;
	register unsigned int t;

	if ((bp = MAPSTART(mp))==MAPSTART(coremap) && runin) {
		runin = 0;
		wakeup((caddr_t)&runin);	/* Wake scheduler when freeing core */
	}
	for (; bp->m_addr<=a && bp->m_size!=0; bp++);
	if (bp>MAPSTART(mp) && (bp-1)->m_addr+(bp-1)->m_size == a) {
		(bp-1)->m_size += size;
		if (a+size == bp->m_addr) {
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
			MAPSIZE(mp)++;
		}
	} else {
		if (a+size == bp->m_addr && bp->m_size) {
			bp->m_addr -= size;
			bp->m_size += size;
		} else if (size) {
			/*
			 * We now must add a new entry to the map. If
			 * there isn't room for it, then throw away
			 * either the first or last entry, which ever
			 * is smaller. We lose that segment forever, but
			 * it is either that or throw away this new
			 * chunk that is in the middle of the map, which
			 * would probably cause more serious fragmenting.
			 */
			if (MAPSIZE(mp) == 0) {
				/* oh boy, no more room. time to fragment... */
				register struct map *lmp;

				printf("%s mapsize exceeded (%o)\n",
					mp == coremap ? "core" :
					mp == swapmap ? "swap" : "???", mp);
				/* find the last non-zero entry. */
				lmp = MAPSTART(mp);
				while (lmp->m_size)
					lmp++;
				lmp--;
				if (lmp->m_size > (MAPSTART(mp))->m_size) {
					/* throw away first entry */
					/* by shuffling backwards */
					while (--bp >= MAPSTART(mp)) {
						t = bp->m_addr;
						bp->m_addr = a;
						a = t;
						t = bp->m_size;
						bp->m_size = size;
						size = t;
					}
					return;
				} else {
					/* throw away last entry */
					lmp->m_size = 0;
					lmp->m_addr = 0;
					MAPSIZE(mp)++;
				}
			}
			do {
				t = bp->m_addr;
				bp->m_addr = a;
				a = t;
				t = bp->m_size;
				bp->m_size = size;
				bp++;
			} while (size = t);
			MAPSIZE(mp)--;
		}
	}
 	if (MAPWANT(mp)) {
 		MAPWANT(mp) = 0;
 		wakeup((caddr_t)mp);
	}
}
