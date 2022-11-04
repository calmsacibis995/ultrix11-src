
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)tapeio.c	3.0	4/22/86
char	id_tapeio[] = "(2.9BSD)  tapeio.c  1.2";
 *
 * tapeio - tape device specific I/O routines
 *
 *	ierr = topen  (tlu, name, labelled)
 *	ierr = tclose (tlu)
 *	nbytes = tread  (tlu, buffer)
 *	nbytes = twrite (tlu, buffer)
 *	ierr = trewin (tlu)
 *	ierr = tskipf (tlu, nfiles, nrecs)
 *	ierr = tstate (tlu, fileno, recno, err, eof, eot, tcsr)
 */

#include	<ctype.h>
#include	<sys/ioctl.h>
#ifndef	MTIOCGET
#include	<sys/types.h>
#include	<sys/mtio.h>
#endif
#include	"../libI77/fiodefs.h"
#undef	fileno

#define	TU_NAMESIZE	22
#define	TU_MAXTAPES	4

struct tunits {
	char	tu_name[TU_NAMESIZE];	/* device name */
	int	tu_fd;			/* file descriptor */
	int	tu_flags;		/* state flags */
	int	tu_file;		/* current tape file number */
	int	tu_rec;			/* current record number in file */
} tunits[TU_MAXTAPES];

#define	TU_OPEN		0x1
#define	TU_EOF		0x2
#define	TU_ERR		0x4
#define	TU_READONLY	0x8
#define	TU_LABELLED	0x10
#define	TU_WRITING	0x20
#define	TU_EOT		0x40
#define	TU_RDATA	0x80

#ifdef	MTWEOF
struct mtget	mtget;		/* controller status */
#endif

ftnint	tclose_();

/*
 * Open a tape unit for I/O
 *
 * calling format:
 *	integer topen, tlu
 *	character*(*) devnam
 *	logical labled
 *	ierror = topen(tlu, devnam, labled)
 * where:
 *	ierror will be 0 for successful open; an error number otherwise.
 *	devnam is a character string
 *	labled should be .true. if the tape is labelled.
 */

ftnint
topen_(tlu, name, labelled, len)
ftnint	*tlu;
char	*name;
ftnint	*labelled;
ftnlen	len;
{
	struct tunits	*tu;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (tu->tu_flags & TU_OPEN)
		tclose_(tlu);

	if (len >= TU_NAMESIZE) {
		errno = F_ERARG;
		return((ftnint) -1);
	}

	g_char(name, len, tu->tu_name);

	if ((tu->tu_fd = open(tu->tu_name, 2)) < 0) {
		if ((tu->tu_fd = open(tu->tu_name, 0)) < 0)
			return((ftnint) -1);
		tu->tu_flags |= TU_READONLY;
	}
	tu->tu_flags |= TU_OPEN;
	tu->tu_file = tu->tu_rec = 0;
	if (*labelled)
		tu->tu_flags |= TU_LABELLED;
	return((ftnint) 0);
}

/*
 * Close a tape unit previously opened by topen_()
 *
 * calling sequence:
 *	integer tlu, tclose
 *	ierrno = tclose(tlu)
 * where:
 *	tlu is a previously topened tape logical unit.
 */

ftnint
tclose_(tlu)
ftnint	*tlu;
{
	struct tunits	*tu;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (!(tu->tu_flags & TU_OPEN))
		return((ftnint) 0);

	tu->tu_flags = 0;
	if (close(tu->tu_fd) < 0)
		return((ftnint) -1);
	return((ftnint) 0);
}

/*
 * Read from a tape logical unit
 *
 * calling sequence:
 *	integer tread, tlu
 *	character*(*) buffer
 *	ierr = tread(tlu, buffer)
 */

ftnint
tread_(tlu, buffer, len)
ftnint	*tlu;
char	*buffer;
ftnlen	len;
{
	struct tunits	*tu;
	int	nbytes;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (!(tu->tu_flags & TU_OPEN)) {
		errno = F_ERNOPEN;
		return((ftnint) -1);
	}
	if (tu->tu_flags & TU_WRITING) {
		errno = F_ERILLOP;
		return((ftnint) -1);
	}
	if (tu->tu_flags & (TU_EOF|TU_EOT))
		return((ftnint) 0);

	if ((nbytes = read(tu->tu_fd, buffer, (int)len)) > 0)
		tu->tu_flags |= TU_RDATA;

	if (nbytes == 0 && len != 0) {
		tu->tu_flags |= TU_EOF;
		if (tu->tu_rec == 0)
			tu->tu_flags |= TU_EOT;
	}
	if (nbytes < 0)
		tu->tu_flags |= TU_ERR;
	else
		tu->tu_rec++;

	return((ftnint)nbytes);
}

/*
 * Write to a tape logical unit
 *
 * calling sequence:
 *	integer twrite, tlu
 *	character*(*) buffer
 *	ierr = twrite(tlu, buffer)
 */

ftnint
twrite_(tlu, buffer, len)
ftnint	*tlu;
char	*buffer;
ftnlen	len;
{
	struct tunits	*tu;
	int	nbytes;
	ftnint	nf;
	ftnint	zero = (ftnint) 0;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (!(tu->tu_flags & TU_OPEN)) {
		errno = F_ERNOPEN;
		return((ftnint) -1);
	}
	if (tu->tu_flags & TU_READONLY) {
		errno = F_ERILLOP;
		return((ftnint) -1);
	}

	if (tu->tu_flags & TU_EOT) {	/* must backspace over last EOF */
		nf = (ftnint)tu->tu_file;	/* should be number to skip */
		trewin_(tlu);
		tskipf_(tlu, &nf, &zero);
	}

	nbytes = write(tu->tu_fd, buffer, (int)len);
	if (nbytes <= 0)
		tu->tu_flags |= TU_ERR;
	tu->tu_rec++;
	tu->tu_flags |= TU_WRITING;
	tu->tu_flags &= ~(TU_EOF|TU_EOT|TU_RDATA);
	return((ftnint)nbytes);
}

/*
 * rewind a tape device
 */

ftnint
trewin_(tlu)
ftnint	*tlu;
{
	struct tunits	*tu;
	char	namebuf[TU_NAMESIZE];
	register char	*p, *q;
	int	munit;
	int	rfd;
	ftnint	labelled;
	ftnint	one	= (ftnint) 1;
	ftnint	zero	= (ftnint) 0;
	int	save_errno;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (!(tu->tu_flags & TU_OPEN)) {
		errno = F_ERNOPEN;
		return((ftnint) -1);
	}
	labelled = (tu->tu_flags & TU_LABELLED);
	tclose_(tlu);

	for (p = tu->tu_name, q = namebuf; *p; p++) {
		if (*p == 'n')	/* norewind name */
			continue;
		if (isdigit(*p)) {	/* might be norewind minor dev */
			munit = 0;
			while (isdigit(*p))
				munit = (10 * munit) + (*p++ - '0');
			*q++ = (munit & 03) + '0';
			while (*p)
				*q++ = *p++;
			break;
		}
		*q++ = *p;
	}
	*q = '\0';
	/* debug  printf("rewinding [%s]\n", namebuf); /* */

	if ((rfd = open(namebuf, 0)) < 0)
		save_errno = errno;
	else {
		save_errno = 0;
		close(rfd);
	}

	topen_(tlu, tu->tu_name, &labelled, (ftnint)strlen(tu->tu_name));
	if (labelled) {
		tskipf_(tlu, &one, &zero);
		tu->tu_file = 0;
	}
	if (save_errno) {
		errno = save_errno;
		return((ftnint) -1);
	}
	return((ftnint) 0);
}

/*
 * Skip forward files
 */

ftnint
tskipf_(tlu, nfiles, nrecs)
ftnint	*tlu;
ftnint	*nfiles;
ftnint	*nrecs;
{
	struct tunits	*tu;
	char	dummybuf[20];
	int	nf;
	int	nr;
	int	nb;
	int	empty;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (!(tu->tu_flags & TU_OPEN)) {
		errno = F_ERNOPEN;
		return((ftnint) -1);
	}
	if (tu->tu_flags & TU_WRITING) {
		errno = F_ERILLOP;
		return((ftnint) -1);
	}

	nf = (int)*nfiles;
	while (nf > 0) {
		if (tu->tu_flags & TU_EOT) {
			errno = F_ERILLOP;
			return((ftnint) -1);
		}
		if (tu->tu_flags & TU_EOF)
			tu->tu_flags &= ~TU_EOF;
		else {
			empty = ((tu->tu_flags & TU_RDATA) == 0);
			while ((nb = read(tu->tu_fd, dummybuf, sizeof dummybuf)) > 0)
				empty = 0;

			if (nb < 0) {
				tu->tu_flags |= TU_ERR;
				return((ftnint) -1);
			}
			if (empty)
				tu->tu_flags |= TU_EOT;
		}
		nf--;
		tu->tu_rec = 0;
		tu->tu_flags &= ~TU_RDATA;
		if (tu->tu_flags & TU_EOT)
			return((ftnint) -1);
		else
			tu->tu_file++;
	}

	nr = (int)*nrecs;
	while (nr > 0) {
		if (tu->tu_flags & (TU_EOT|TU_EOF)) {
			errno = F_ERILLOP;
			return((ftnint) -1);
		}

		empty = ((nb = read(tu->tu_fd, dummybuf, sizeof dummybuf)) <= 0);
		if (nb < 0) {
			tu->tu_flags |= TU_ERR;
			return((ftnint) -1);
		}
		if (empty) {
			tu->tu_flags |= TU_EOF;
			if (!(tu->tu_flags & TU_RDATA))
				tu->tu_flags |= TU_EOT;
		} else
			tu->tu_flags |= TU_RDATA;
		nr--;
		tu->tu_rec++;
	}
	return((ftnint) 0);
}

/*
 * Return status of tape channel
 */

ftnint
tstate_(tlu, fileno, recno, err, eof, eot, tcsr)
ftnint	*tlu, *fileno, *recno, *err, *eof, *eot, *tcsr;
{
	struct tunits	*tu;
	int		csr;

	if (*tlu < 0 || *tlu >= TU_MAXTAPES) {
		errno = F_ERUNIT;
		return((ftnint) -1);
	}

	tu = &tunits[*tlu];
	if (!(tu->tu_flags & TU_OPEN)) {
		errno = F_ERNOPEN;
		return((ftnint) -1);
	}

	*fileno = (ftnint)tu->tu_file;
	*recno = (ftnint)tu->tu_rec;
	*err = (ftnint)((tu->tu_flags & TU_ERR) != 0);
	*eof = (ftnint)((tu->tu_flags & TU_EOF) != 0);
	*eot = (ftnint)((tu->tu_flags & TU_EOT) != 0);
#ifdef	MTWEOF
	ioctl(tu->tu_fd, MTIOCGET, &mtget);
	*tcsr = (ftnint)mtget.mt_dsreg & 0xffff;
#else
	ioctl(tu->tu_fd, MTIOCGET, &csr);
	*tcsr = (ftnint)csr;
#endif
	return((ftnint) 0);
}
