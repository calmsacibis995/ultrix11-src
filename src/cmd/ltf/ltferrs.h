
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ltferrs.h
 *
 */
/**/
/*
 *
 *	File name:
 *
 *	    ltferrs.h
 *
 *	Source file description:
 *
 *		This file contains definitions of all
 *		Labeled Tape Facility (LTF) error messages
 *		and error messages macros.
 *
 *
 *	Functions:
 *
 *		n/a
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *		#include "ltferrs.h"	Include local error message defs.
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	 01.0		12-April-85	Ray Glaser
 *			Create orginal version.
 *	
 *	 01.1		4-Sep-85	Suzanne Logcher
 *			Put FATAL, Warning, or Info in errors
 *			Add some more
 */
/**/
/*
 * ->	General Print ERROR macro
 */

#define PERROR fprintf(stderr,
#define PROMPT fprintf(stderr,

/*
 * ->	Error messages.
 */
#ifdef MAINC	/* Only compile the actual messges when compiling the
		 * main line logic, else - define as externals below.
		 * (otherwise, all messages would be multiply defined
	         * at link time.
		 */
/*_A_*/
char *ALTERN	= "Alternative pathname ('return' to bypass extract)?";
char *ANSIV	= "  Volume is:  ANSI Version";

/*_B_*/
char *BADCENT	= "Warning > Invalid century in creation date, 20th century default used";
char *BADCNT1	= "Warning > For file";
char *BADCNT2	= "File character count =";
char *BADCNT3	= "!= bytes read from volume =";
char *BADFILT	= "FATAL > File type in question, not dumped ->";
char *BADSSC	= "Warning > Bad sscanf call on ->";
char *BADST	= "FATAL > Bad stat call on path name ->";
char BELL	= {007};
char *BFRCNE	= "FATAL > Begining & final FUF record counts not equal ->";
char *BYTE	= " byte";
char *BYTES	= " bytes";

/*_C_*/
char *CANTAT	= "Warning > Cannot write as TEXT file";
char *CANTBUF	= "FATAL > Cannot read buffer offset field of size ->";
char *CANTBSF	= "FATAL > Cannot backspace (file) ->";
char *CANTCGET	= "FATAL > Cannot get status of device ->";
char *CANTCLS	= "FATAL > Cannot close device ->";
char *CANTCHD	= "FATAL > Cannot change directory to ->";
char *CANTCHW	= "Warning > Cannot change directory to ->";
char *CANTCRE	= "FATAL > Cannot create file ->";
char *CANTFPW	= "Warning > Cannot find user name in password file, UID = ";
char *CANTFSF	= "FATAL > Cannot forwardspace (file) ->";
char *CANTFSR	= "FATAL > Cannot forwardspace (record) ->";
char *CANTL1	= "Warning > Cannot find link file for ->";
char *CANTLF	= "Warning > Cannot link";
char *CANTMKD	= "FATAL > Cannot make directory ->";
char *CANTOD	= "FATAL > Cannot open directory ->";
char *CANTOPEN	= "FATAL > Cannot open ->";
char *CANTOPW	= "Warning > Cannot open ->";
char *CANTPER	= "Warning > p key only used with x function";
char *CANTRL	= "FATAL > Cannot read label ->";
char *CANTRD	= "FATAL > Cannot read from ->";
char *CANTREW	= "FATAL > Cannot rewind. Busy or not online ->";
char *CANTRSL	= "Warning > Cannot read symbolic link ->";
char *CANTSTAT	= "FATAL > Cannot execute file stat call for file ->";
char *CANTSTS	= "Warning > Cannot execute file stat call for symbolic link file of ->";
char *CANTSTW	= "Warning > Cannot execute file stat call for file ->";
char *CANTWEOF = "FATAL > Cannot write EOF on ->";
char *CANTWVL	= "FATAL > Cannot write VOL1 on ->";
char *CONFF	= "FATAL > Functions c, t, and x are mutually exclusive";

/*_D_*/
char *DIRCRE	= " (directory created)";

/*_E_*/
char *ENFNAM	= "Enter FILE name<cr>, or just 'return' to quit > ";
char *EINVLD	= "FATAL > -- END INVALID LABEL DATA --";
char *EOFINM	= "FATAL > EOF encountered in middle of file ->";
char *ERREOT	= "FATAL > End of Tape (EOT) encountered on ->";
char *ERRWRF	= "FATAL > Error writing file ->";
char *EXFNAM	= "Enter desired EXTRACTED file name, or 'return' for default > ";
char *EXISTS	= "Warning > File already exists ->";

/*_F_*/
char *FILENNG	= "Warning > File name cannot be reproduced on non-Ultrix systems";
char *FNTL	= "Warning > File name too long for ANSI label set ->";
char *FSTCB	= "Warning > First control byte in FUF is ->";
char *FUFTL	= "FATAL > Fortran Unformatted File record too long";

/*_G_*/
char *GETWDF	= "FATAL > Get working directory call (getwd) failure ->";

/*_H_*/
#ifndef U11	/* ULTRIX-11 has it's own help command */
char *HELP1	= "One of the following keys enclosed in {} is required\n";
char *HELP2	= "c = create a new volume, previous content is overwritten";
char *HELP3	= "H = help mode, print this summary";
char *HELP4	= "t = table the contents of the volume";
char *HELP5	= "x = extract files from the volume\n";
char *HELP6	= "Items enclosed in second [] are optional switches\n";
char *HELP7	= "a = output ANSI Version 3 format to volume";
#ifndef U11
char *HELP9	= "g = select 6250 GCR tape device (/dev/rmt8)";
#else
char *HELP9	= "g = select 6250 GCR tape device (/dev/rgt0)";
#endif U11
char *HELP10	= "h = output file pointed to by a symbolic link instead of symbolic link file";
#ifndef U11
char *HELP11	= "k = select TK50 tape device (/dev/rmt8)";
#else
char *HELP11	= "k = select TK50 tape device (/dev/rtk0)";
#endif U11
char *HELP12	= "n = select 800 bpi tape device (/dev/rmt0)";
char *HELP13	= "o = omit outputting directory blocks to volume";
char *HELP14	= "O = omit the usage of headers 3 to 9";
char *HELP30	= "p = change permissions and owner of extracted files to original values,";
char *HELP31	= "    must be super user";
char *HELP15	= "v = verbose mode, provide additional information about files/operation";
char *HELP16	= "V = big verbose mode, include directory information in table of contents";
char *HELP17	= "w = warn if a file name is truncated on creation or may be overwritten";
char *HELP18	= "    on extract";
char *HELP19	= "0..9 = select the unit number for the named tape device\n";
char *HELP20	= "Press RETURN to continue ...";
char *HELP21	= "Items enclosed in third [] are optional keys & require respective";
char *HELP22	= "arguments\n";
#ifndef U11
char *HELP23	= "B = set blocksize, max = 20480 bytes, default = 2048 bytes,";
char *HELP24	= "    min = 18 bytes, used only in creation";
char *HELP25	= "f = set device name, default = /dev/rmt8";
#else
char *HELP23	= "B = set blocksize, max & default = 2048 bytes, min = 18 bytes, used ";
char *HELP24	= "    only in creation";
char *HELP25	= "f = set device name, default = /dev/rht0";
#endif U11
char *HELP26	= "I = set input method, either by stdin or by providing a file name";
char *HELP27	= "L = set volume label, maximum six characters";
char *HELP28	= "P = set position number in form of #,# with # > 0, not used in creation";
char *HELP29	= "R = set record length, max & default = 512 bytes, min = 1 byte\n\n";
#endif NOT U11

char *HLINKTO	= "Info > Hard link to ->";
char *HOSTF	= "FATAL > Call to gethostname (HOSTNM) failed";

/*_I_*/
char *IMPIDC	= "Warning > Implementation ID changed to =>";
char *IMPIDM	= " Implementation ID is: ";
char *INTERCH	= "Interchange Name =";
char *INVBS	= "FATAL > Invalid block size. Min = 18 bytes, Max =";
char *INVOWN	= "FATAL > Non  'a'  characters in  OWNER ID  ->";
char *INVLD	= "FATAL > -- INVALID LABEL DATA FOLLOWS --";
char *INVLF	= "FATAL > Invalid label format in ->";
char *INVLNO	= " (invalid label number)";
char *INVNF	= "FATAL > Invalid numeric format in ->";
char *INVMETA	= "FATAL > Invalid use of meta characters";
char *INVPN	= "FATAL > Invalid position sequence number -> ";
char *INVPNUSE	= "Warning > P key only used with t or x function ";
char *INVPS	= "FATAL > Invalid position section number -> ";
char *INVRS	= "FATAL > Invalid record length. Min = 1 byte, Max =";
char *INVVID1	= "FATAL > Invalid characters in L key (see LTF(5))";
char *INVVID2	= "Warning > 'Z' Indicates invalid character(s) ->";

/*_J_*/ /*_K_*/

/*_L_*/
char *LINETL	= "Warning > Line too long in file to append as TEXT";

/*_M_*/
char *MHL	= "Head link file not extracted ?\n";
char *MS1	= "The  -u  key has precedence over the  -d  key";
char *MS2	= "The  -u  key does not apply when extracting binary files";
char *MS3	= "The  -d  key does not apply when extracting binary files";
char *MULTIV1	= "FATAL > EOV label encountered. Last input file not complete";

/*_N_*/
char *NOARGS	= "FATAL > No file names specified for c function";
char *NOBLK	= "FATAL > No blocksize specified with B key";
char *NOFIL	= "FATAL > No device specified with f key";
char *NOFUNC	= "FATAL > No function specified";
char *NOINP	= "FATAL > No input specified with I key";
char *NOMEM	= "FATAL > No free memory, exiting ...";
char *NOMDIR	= "Warning > No free memory for directory list";
char *NONAFN	= "Warning > Non 'A' characters in file name ->";
char *NOPOS	= "FATAL > No posnmbr specified with P key";
char *NOREC	= "FATAL > No reclen specified with R key";
char *NOTEX	= "FATAL > File not extracted";
char *NOTONP	= "Warning > File not found on volume after position number ->";
char *NOTONV	= "Warning > File not found on volume ->";
char *NOTSU	= "Warning > Not super user, cannot use p key with uid ->";
char *NOVALFI	= "FATAL > No valid files in argument list";
char *NOVOL	= "FATAL > No volumeid specified with L key";

/*_O_*/
char *OWNRID	= " Owner  ID is: ";
char *OVRWRT	= "Overwrite (y/n return=no) ?";

/*_P_*/ /*_Q_*/

/*_R_*/
char *RECLTS	= "FATAL > Record length too short";

/*_S_*/
char *SCNDCB	= "Warning > Second control byte in FUF is ->";
char *SLINKTO	= "Info > Symbolic link to ->";
char *SPCLDF	= "Warning > Cannot dump special device file ->";
char *STOPCRIN	= "Press y to quit OR return to skip unknown file when volume is created >";

/*_T_*/
char *TAPEB	= " Tape block";
char *TAPEBS	= " Tape blocks";
char *TMA	= "FATAL > Too many arguments, out of memory";
char *TRYBIN	= "Warning > Try appending as a binary file";
#ifndef U11
char *TRYHELP	= "Info > Type ltf H for explanation of the usage of the switches ";
#else
char *TRYHELP	= "Use 'help ltf' for an explanation of the switches.";
#endif
char *TRYNH3	= "Warning > Try reading the volume with the O key (Noheader3)";

/*_U_*/
char *UNF	= "FATAL > Unknown Function ->";
char *UNQ	= "FATAL > Unknown Qualifier ->";
char *USEDF	= "Warning > Ltf file type determination being overriden for file ->";
#ifndef U11
char *USE1	= "usage: ltf [-]{cHtx}[aghknoOpvVw0..9][BfILPR]  [blocksize] [devicefilename]";
#else
char *USE1	= "usage: ltf [-]{ctx} [aghknoOpvVw0..9] [BfILPR] [blocksize] [devicefilename]";
#endif
char *USE2	= "       [inputfile] [volumelabel] [positionnumber] [recordlength] [files...]\n";
char *UNSAV	= " (unsupported ANSI version)";

/*_V_*/
char *VOLCRE	= " Volume  created   on: ";
char *VOLIDTL	= "FATAL > Maximum volume id length is 6 a-characters";
char *VOLIS	= " Volume ID is: ";

/*_W_*/
char *WRLINM	= "FATAL > Wrong record length in middle of file ->";

/*_X_*/ /*_Y_*/ /*_Z_*/

#else	/* When compiling sub-modules, define error messages as
	 * externals to avoid multiply defined errors from the
	 * linkage editor.
	 */
/*_A_e_*/
extern *ALTERN;
extern *ANSIV;

/*_B_e_*/
extern *BADCENT, *BADCNT1, *BADCNT2, *BADCNT3;
extern *BADFILT, *BADSSC, *BADST;
extern char BELL;
extern *BFRCNE, *BYTE, *BYTES;

/*_C_e_*/
extern *CANTAT, *CANTBSF, *CANTBUF, *CANTCGET, *CANTCLS, *CANTCHD;
extern *CANTCHW, *CANTCRE, *CANTFPW, *CANTFSF, *CANTFSR, *CANTL1;
extern *CANTLF, *CANTMKD, *CANTOD, *CANTOPEN, *CANTOPW, *CANTPER;
extern *CANTRL, *CANTRD, *CANTREW, *CANTRSL, *CANTSTAT, *CANTSTS;
extern *CANTSTW, *CANTWEOF, *CANTWVL, *CONFF;

/*_D_e_*/
extern *DIRCRE;

/*_E_e_*/
extern *ENFNAM, *EINVLD, *EOFINM, *ERREOT, *ERRWRF, *EXFNAM, *EXISTS;

/*_F_e_*/
extern *FILENNG, *FNTL, *FSTCB, *FUFTL;

/*_G_e_*/
extern *GETWDF;

/*_H_e_*/
#ifndef U11
extern *HELP1, *HELP2, *HELP2, *HELP3, *HELP4, *HELP5, *HELP6;
extern *HELP7, *HELP9, *HELP10, *HELP11, *HELP12;
extern *HELP13, *HELP14, *HELP15, *HELP16, *HELP17, *HELP18;
extern *HELP19, *HELP20, *HELP21, *HELP22, *HELP23, *HELP24;
extern *HELP25, *HELP26, *HELP27, *HELP28, *HELP29, *HELP30, *HELP31;
#endif NOT U11
extern *HLINKTO, *HOSTF;

/*_I_e_*/
extern *IMPIDC, *IMPIDM, *INTERCH;
extern *INVBS, *INVLD, *INVLF, *INVLNO, *INVNF;
extern *INVMETA, *INVOWN, *INVPN, *INVPNUSE, *INVPS, *INVRS;
extern *INVVID1, *INVVID2;

/*_J_e_*/ /*_K_e_*/

/*_L_e_*/
extern *LINETL;

/*_M_e_*/
extern *MHL, *MS1, *MS2, *MS3, *MULTIV1;

/*_N_e_*/
extern *NOFUNC, *NOFIL, *NOMEM, *NOMDIR, *NONAFN, *NOTEX, *NOTONP; 
extern *NOTONV, *NOTSU, *NOVALFI, *NOVOL, *NOARGS, *NOBLK, *NOPOS;
extern *NOREC, *NOINP;
 
/*_O_e_*/
extern *OWNRID;
extern *OVRWRT;

/*_P_e_*/ /*_Q_e_*/

/*_R_e_*/
extern *RECLTS;

/*_S_e_*/
extern *SCNDCB, *SLINKTO, *SPCLDF, *STOPCRIN;


/*_T_e_*/
extern *TAPEB, *TAPEBS, *TMA, *TRYBIN, *TRYHELP, *TRYNH3;

/*_U_e_*/
extern *UNF, *UNQ;
extern *USEDF, *USE1, *USE2;
extern *UNSAV;

/*_V_e_*/
extern *VOLCRE, *VOLIDTL, *VOLIS;

/*_W_e_*/
extern *WRLINM;

/*_X_e_*/ /*_Y_e_*/ /*_Z_e_*/

#endif

/**\\**\\**\\**\\**\\**  EOM  ltferrs.h  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  ltferrs.h  **\\**\\**\\**\\**\\*/
