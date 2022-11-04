
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)nami.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>

#ifndef	saveseg5
#include <sys/seg.h>
#endif

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 *
 * func = function called to get next char of name
 *	&uchar if name is in user space
 *	&schar if name is in system space
 *	flag =	LOOKUP if name is sought
 *		CREATE if name is to be created
 *		DELETE if name is to be deleted
 * follow = 1 if to follow links at end of name
 */
struct inode *
namei(func, flag, follow)
int (*func)();
{
	register struct direct *dirp;
	register struct inode *dp;
	int c;
	struct buf *bp;
	int nlink;
	dev_t d;
	off_t eo;
	int mountflag;
	unsigned ne;

	mountflag = 0;
	nlink = 0;
	u.u_sbuf = 0;
	/*
	 * If name starts with '/' start from
	 * root; otherwise start from current dir.
	 */

	dp = u.u_cdir;
	{
		register c2;
		if((c2=(*func)()) == '/')
			if ((dp = u.u_rdir) == NULL)
				dp = rootdir;
		iget(dp->i_dev, dp->i_number);
		while(c2 == '/')
			c2 = (*func)();
		if(c2 == '\0' && flag != LOOKUP)
			u.u_error = ENOENT;
		c = c2;
	}

	for(;;) {
		/*
		 * Here dp contains pointer
		 * to last component matched.
		 */

		if(u.u_error)
			break;
		if(c == '\0')
			return(dp);

		/*
		 * If there is another component,
		 * Gather up name into
		 * users' dir buffer.
		 */

		{
			register c2 = c;
			register char *cp;

			cp = &u.u_dbuf[0];
			while (c2 != '/' && c2 != '\0' && u.u_error == 0 ) {
				if(cp < &u.u_dbuf[DIRSIZ])
					*cp++ = c2;
				c2 = (*func)();
			}
			while(cp < &u.u_dbuf[DIRSIZ])
				*cp++ = '\0';
			while(c2 == '/')
				c2 = (*func)();
			c = c2;
		}

	seloop:
		/*
		 * dp must be a directory and
		 * must have X permission.
		 */

		if((dp->i_mode&IFMT) != IFDIR)
			u.u_error = ENOTDIR;
		/* we want to ignore permission bits of mounted on inode */
		if (mountflag)
			mountflag = 0;
		else
			access(dp, IEXEC);
		if(u.u_error)
			break;

		/*
		 * set up to search a directory
		 */
		u.u_offset = 0;
		u.u_segflg = 1;
		eo = 0;
		ne = 0;
		bp = NULL;
		if (dp == u.u_rdir && *((int *)u.u_dbuf) == '..' &&
		    u.u_dbuf[2] == 0)
			continue;

		for(;;) {		/* eloop */

			/*
			 * If at the end of the directory,
			 * the search failed. Report what
			 * is appropriate as per flag.
			 */

			if(u.u_offset >= dp->i_size) {
				if(bp != NULL) {
					mapout(bp);
					brelse(bp);
				}
				if(flag==CREATE && c=='\0') {
					if(access(dp, IWRITE))
						goto out;
					u.u_pdir = dp;
					if(eo)
						u.u_offset = eo-sizeof(struct direct);
					else
						dp->i_flag |= IUPD|ICHG;
					goto out1;
				}
				u.u_error = ENOENT;
				goto out;
			}

			/*
			 * If offset is on a block boundary,
			 * read the next directory block.
			 * Release previous if it exists.
			 */

			if((u.u_offset&BMASK) == 0) {
				if(bp != NULL) {
					mapout(bp);
					brelse(bp);
				}
				bp = bread(dp->i_dev,
					 bmap(dp, (daddr_t)(u.u_offset>>BSHIFT),
						B_READ));
				if (bp->b_flags & B_ERROR) {
					brelse(bp);
					goto out;
				}
				dirp = (struct direct *)mapin(bp);
			}

			/*
			 * Note first empty directory slot
			 * in eo for possible creat.
			 * String compare the directory entry
			 * and the current component.
			 * If they do not match, go back to eloop.
			 */
			{
				register struct direct *ndirp;
				struct direct *srchd();

				ndirp = srchd(dirp, u.u_dbuf, &ne);
				if((ne != 0) && (eo == 0))
					eo = u.u_offset + (ne - (unsigned)dirp);
				if (ndirp == NULL) {
					u.u_offset += BSIZE;
					continue;
				}
				u.u_offset += (unsigned)ndirp - (unsigned)dirp +
							sizeof(struct direct);
				dirp = ndirp;
			}
			break;
		}
		u.u_dent = *dirp;

		/*
		 * Here a component matched in a directory.
		 * If there is more pathname, go back to
		 * the top of the loop, otherwise return.
		 */

		if(bp != NULL) {
			mapout(bp);
			brelse(bp);
		}
		if(flag==DELETE && c=='\0') {
			if(access(dp, IWRITE))
				break;
			return(dp);
		}
		d = dp->i_dev;
		if (u.u_dent.d_ino == ROOTINO &&
		    dp->i_number == ROOTINO &&
		    u.u_dent.d_name[1] == '.') {
			register int i;

			for(i=1; i<nmount; i++)
				if (mount[i].m_inodp != NULL && mount[i].m_dev == d) {
					iput(dp);
					dp = mount[i].m_inodp;
					dp->i_count++;
					plock(dp);
					mountflag++;
					goto seloop;
				}
		}
		prele(dp);
		{
			register struct inode *pdp;

			if ((pdp = iget(d, u.u_dent.d_ino)) == NULL) {
				if (dp->i_flag & ILOCK)
					dp->i_count--;
				else
					iput(dp);
				goto out1;
			}
			if ((pdp->i_mode&IFMT)==IFLNK && (follow || c)) {
				if (pdp->i_size >= BSIZE-2 || ++nlink>8 ||
				    u.u_sbuf || !pdp->i_size) {
					u.u_error = ELOOP;
					iput(pdp);
					break;
				}
				u.u_sbuf = bread(pdp->i_dev, bmap(pdp, (daddr_t)0, B_READ));
				if (u.u_sbuf->b_flags & B_ERROR) {
					brelse(u.u_sbuf);
					iput(pdp);
					u.u_sbuf = 0;
					break;
				}
				/* Save our readahead chars at end of buffer, */
				/* get first symbolic link character */
				{
					segm save5;
					char *cp;

					if (c) /* space for readahead chars */
						u.u_slength = pdp->i_size+2;
					else	u.u_slength = pdp->i_size+1;
					u.u_soffset = 0;
					saveseg5(save5);
					mapin(u.u_sbuf);
					cp = (char *)SEG5;
					if (c)
						cp[u.u_slength-2] = '/';
					cp[u.u_slength-1] = c;
					c = cp[u.u_soffset++];
					mapout(u.u_sbuf);
					restorseg5(save5);
				}

				/* Grab the top-level inode for the new path */
				iput(pdp);
				if (c == '/') {
					iput(dp);
					if ((dp = u.u_rdir) == NULL)
						dp = rootdir;
					while (c == '/')
						c = (*func)();
					iget(dp->i_dev, dp->i_number);
				}
				else	plock(dp);
				continue;
			}
			else {
				if (dp->i_flag & ILOCK)
					dp->i_count--;
				else
					iput(dp);
				dp = pdp;
			}
		}
	}
out:
	iput(dp);
out1:
	if (u.u_sbuf) {
		brelse(u.u_sbuf);
		u.u_sbuf = u.u_slength = u.u_soffset = 0;
	}
	return(NULL);
}

/*
 * Return the next character from the
 * kernel string pointed at by dirp.
 */
schar()
{
	register c;

	if (u.u_sbuf) {
		c = symchar();
		if (c >= 0)
			return(c);
	}
	return(*u.u_dirp++ & 0377);
}

/*
 * Return the next character from the
 * user string pointed at by dirp.
 */
#ifdef	notdef	/* generated in assembly by mkuchar */
uchar()
{
	register c;

	if (u.u_sbuf) {
		c = symchar();
		if (c >= 0)
			return(c);
	}
	c = fubyte(u.u_dirp++);
	if(c == -1)
		u.u_error = EFAULT;
	else if (c&0200)
		u.u_error = EINVAL;
	return(c);
}
#endif	notdef

/*
 *	Get a character from the symbolic name buffer
 */
symchar()
{
	segm save5;
	register char c;
	register char *cp;

	if (!u.u_sbuf)		/* Protect ourselves */
		return(-1);
	if (u.u_soffset > u.u_slength) {
		brelse(u.u_sbuf);
		u.u_soffset = u.u_slength = u.u_sbuf = 0;
		return(-1);
	}

	/* Get next character from symbolic link buffer */
	saveseg5(save5);
	mapin(u.u_sbuf);
	cp = (char *)SEG5;
	c = cp[u.u_soffset++];
	mapout(u.u_sbuf);
	restorseg5(save5);
	if (u.u_soffset >= u.u_slength) {
		brelse(u.u_sbuf);
		u.u_soffset = u.u_slength = u.u_sbuf = 0;
	}
	return(c);
};	/* end of symchar */
