
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)elp3.c	3.0	4/21/86";
/*
 * ULTRIX-11 error log report program (elp) - PART 3
 * Fred Canter 12/12/82
 *
 * The input for the report is taken from the current
 * error log or the optional [file].
 *
 * PART 3 contains the mass of text and pointers
 * needed to generate the full error report.
 *
 */

#include "elp.h"


/* Device register name arrays */

char	*rx_reg[] =
{
	"\tRX2CS",
	"\tRX2ES",
	0,
};

char	*ml_reg[] =
{
	"\tMLCS1",
	"\tMLWC",
	"\tMLBA",
	"\tMLDA",
	"\tMLCS2",
	"\tMLDS",
	"\tMLER",
	"\tMLAS",
	"\tMLPA",
	"\tMLPB",
	"\tMLMR",
	"\tMLDT",
	"\tMLSN",
	"\tMLE1",
	"\tMLE2",
	"\tMLD1",
	"\tMLD2",
	"\tMLEE",
	"\tMLEL",
	"\tMLPD",
	"\tMLBAE",
	"\tMLCS3",
	0,
};

char	*rk_reg[] =
{
	"\tRKDS",
	"\tRKER",
	"\tRKCS",
	"\tRKWC",
	"\tRKBA",
	"\tRKDA",
	"\tRKMR",
	"\tRKDB",
	0,
};

char	*rp_reg[] =
{
	"\tRPDS",
	"\tRPER",
	"\tRPCS",
	"\tRPWC",
	"\tRPBA",
	"\tRPCA",
	"\tRPDA",
	"\tRPM1",
	"\tRPM2",
	"\tRPM3",
	"\tRPSUCA",
	"\tRPSILO",
	0
};

/* This is a dummy array, it is not used ! */
char	*ra_reg[] =
{
	0
};

char	*tm_reg[] =
{
	"\tTMER",
	"\tTMCS",
	"\tTMBC",
	"\tTMBA",
	"\tTMDB",
	"\tTMRD",
	0
};

char	*tk_reg[] =
{
	0
};

char	*hp_reg[] =
{
	"\tRPCS1",
	"\tRPWC",
	"\tRPBA",
	"\tRPDA",
	"\tRPCS2",
	"\tRPDS",
	"\tRPER1",
	"\tRPAS",
	"\tRPLA",
	"\tRPDB",
	"\tRPMR",
	"\tRPDT",
	"\tRPSN",
	"\tRPOF",
	"\tRPCA",
	"\tRPCC",
	"\tRPER2",
	"\tRPER3",
	"\tRPPOS",
	"\tRPPAT",
	"\tRPBAE",
	"\tRPCS3",
	0
};

char	*rm_reg[] =
{
	"\tRMCS1",
	"\tRMWC",
	"\tRMBA",
	"\tRMDA",
	"\tRMCS2",
	"\tRMDS",
	"\tRMER1",
	"\tRMAS",
	"\tRMLA",
	"\tRMDB",
	"\tRMMR1",
	"\tRMDT",
	"\tRMSN",
	"\tRMOF",
	"\tRMCA",
	"\tRMHR",
	"\tRMMR2",
	"\tRMER2",
	"\tRMPOS",
	"\tRMPAT",
	"\tRMBAE",
	"\tRMCS3",
	0
};

char	*ht_reg[] =
{
	"\tTMCS1",
	"\tTMWC",
	"\tTMBA",
	"\tTMFC",
	"\tTMCS2",
	"\tTMDS",
	"\tTMER",
	"\tTMAS",
	"\tTMCK",
	"\tTMDB",
	"\tTMMR",
	"\tTMDT",
	"\tTMSN",
	"\tTMTC",
	"\tTMBAE",
	"\tTMCS3",
	0
};

char	*hk_reg[] =
{
	"\tRKCS1",
	"\tRKWC",
	"\tRKBA",
	"\tRKDA",
	"\tRKCS2",
	"\tRKDS",
	"\tRKERR",
	"\tRKAS",
	"\tRKDC",
	"\tRKDUM",
	"\tRKDB",
	"\tRKMR1",
	"\tRKEC1",
	"\tRKEC2",
	"\tRKMR2",
	"\tRKMR3",
	0
};

char	*ts_reg[] =
{
	"   TSBA/TSDB",
	"\tTSSR",
	"\ttscom",
	"\ttsba",
	"\ttsbae",
	"\ttsbc",
	"\tmsbptr",
	"\tmsbae",
	"\tmsbsiz",
	"\tmschar",
	"\tmshdr",
	"\tmssiz",
	"  RBPCR/msresid",
	"\tmstsx0",
	"\tmstsx1",
	"\tmstsx2",
	"\tmstsx3",
	0
};

char	*rl_reg[] =
{
	"\tRLCS",
	"\tRLBA",
	"\tRLDA",
	"\tRLMP",
	" STATUS RLMP",
	"\tRLBAE",
	0
};

/*
 * The following arays contain the text used
 * to print which bits in a device register
 * are set.
 */

/*
 * rhcs1 is now a special case, instead of a
 * print set bits case.
 */

char	*rhcs1s[] =
{
	"SC","TRE","MCPE","","DVA","PSEL","A17","A16",
	"RDY","IE","","","","","","GO",
	0
};
char	*rhcs2[] =
{
	"DLT","WCE","UPE","NED","NEM","PGE","MXF","MDPE",
	"OR","IR","CLR","PAT","BAI","U2","U1","U0",
	0
};
char	*hpds[] =
{
	"ATA","ERR","PIP","MOL","WRL","LST","PGM","DPR",
	"DRY","VV","","","","","","",
	0
};
char	*hper1[] =
{
	"DCK","UNS","OPI","DTE","WLE","IAE","AOE","HCRC",
	"HCE","ECH","WCF","FER","PAR","RMR","ILR","ILF",
	0
};
char	*mler[] =
{
	"DCK","UNS","OPI","","","IAE","AOE","",
	"","ECH","DPAR","","CPAR","RMR","ILR","ILF",
	0
};
char	*mlmrs[] =
{
	"","","","","","","","",
	"MAR","R/W","DIS","CLK","DDM","EN","DIS","EDM",
	0
};
char	*mlees[] = 
{
	"UNC","SGL","CRC","","","","","",
	"","","","","","","","",
	0
};
char	*rhas[] =
{
	"","","","","","","","",
	"ATA7","ATA6","ATA5","ATA4","ATA3","ATA2","ATA1","ATA0",
	0
};
char	*hpof[] =
{
	"SGCH","","","FMT22","ECCI","HCI","","",
	"","","","","","","","",
	0
};
char	*rmof[] =
{
	"","","","FMT16","ECCI","HCI","","",
	"OFD","","","","","","","",
	0
};
char	*hper2[] =
{
	"ACU","","PLU","30VU","IXE","NAS","MHS","WRU",
	"FEN","TUF","TDF","MSE","CSU","WSU","CSF","WCU",
	0
};
char	*hper3[] =
{
	"OCYL","SKI","","","","","","",
	"","ACL","DCL","PRE","UWR","","VUF","PSU",
	0
};
char	*rmer2[] =
{
	"BSE","SKI","OPE","IC","LSC","LBC","","",
	"DC","","","","DPE","","","",
	0
};
char	*rhbae[] =
{
	"","","","","","","","",
	"","","A21","A20","A19","A18","A17","A16",
	0
};
char	*rhcs3[] =
{
	"APE","DBEOW","DPEEW","WCEOW","WCEEW","DBL","","",
	"","IE","","","IPCK3","IPCK2","IPCK1","IPCK0",
	0
};
char	*htds[] =
{
	"ATA","ERR","PIP","MOL","WRL","EOT","","DPR",
	"DRY","SSC","PES","SDWN","IOB","TM","BOT","SLA",
	0
};
char	*hter[] =
{
	"COR/CRC","UNS","OPI","DTE","NEF","CS/ITM","FCE","NSG",
	"PEF/LRC","INC/VPE","DPAR","FMT","CPAR","RMR","ILR","ELF",
	0
};
char	*httc[] =
{
	"ACCL","TCW","FCS","EAODTE","","DEN2","DEN1","DEN0",
	"FMT3","FMT2","FMT1","FMT0","EVPAR","SS2","SS1","SS0",
	0
};
char	*rkds[] =
{
	"D2","D1","D0","DPL","RK05","UNS","SKI","SCOK",
	"RWSRDY","ARDY","WPS","SC=SA","SC3","SC2","SC1","SC0",
	0
};
char	*rker[] =
{
	"DE","OVR","WV","SE","PGE","MXM","DLT",
	"TE","NED","NEC","NES","","","","CSE","WCE",
	0
};
char	*rkcs[] =
{
	"ERR","HE","SI","","BAI","FMT","","SOSE",
	"CR","IE","A17","A16","","","","GO",
	0
};
char	*rpds[] =
{
	"SURDY","SUOL","RP03","HNF","SI","SU","FU","WP",
	"ATA7","ATA6","ATA5","ATA4","ATA3","ATA2","ATA1","ATA0",
	0
};
char	*rper[] =
{
	"WPV","FUV","NXC","NXT","NXS","PROG","FMTE","MODE",
	"LPE","WPE","CSME","TIMEE","WCE","NXME","EOP","DSKERR",
	0
};
char	*rpcss[] =
{
	"ERR","HE","AIE","MODE","HDR","DS2","DS1","DS0",
	"RDY","IDE","A17","A16","","","","GO",
	0
};
char	*tmer[] =
{
	"IC","EOF","CRC","PE","BGL","EOT","RLE","BTE",
	"NXM","SR","BOT","7CH","TSD","WL","RWDS","TUR",
	0
};
char	*tmcs[] =
{
	"ERR","DEN1","DEN0","PC","LP","U2","U1","U0",
	"CUR","IE","A17","A16","F2","F1","F0","GO",
	0
};
/*
 * hkcs1 is now a special case, instead of
 * a print set bits case.
 */

char	*hkcs1s[] =
{
	"CERR","DI","DCPE","CFMT","CTO","","A17","A16",
	"CRDY","IE","","","","","","GO",
	0
};
char	*hkcs2[] =
{
	"DLE","WCE","UPE","NED","NEM","PGE","MDS","UFE",
	"OR","IR","SCLR","BAI","REL","DS2","DS1","DS0",
	0
};
char	*hkdss[] =
{
	"SVAL","CDA","PIP","","WRL","","","","DRDY",
	"VV","DROT","SPLS","ACLO","OFST","","DRA",
	0
};
char	*hkerr[] =
{
	"DCK","UNS","OPI","DTE","WLE","IDAE","COE","HVRC",
	"BSE","ECH","DTYE","FMTE","DRPAR","NXF","SKI","ILF",
	0
};

char	*hkas[] =
{
	"ATN7","ATN6","ATN5","ATN4","ATN3","ATN2","ATN1","ATN0",
	"OF7","OF6","OF5","OF4","OF3","OF2","OF1","OF0",
	0
};

/*
 * TS11 status and other registers,
 * most require special handling
 * of one sort or another !
 */

char	*tssrs[] =
{
	"SC","UPE","SPE","RMR","NXM","NBA","A17","A16",
	"SSR","OFL","","","","","","",
	0
};

char	*tscoms[] =
{
	"ACK","CVC","OPP","SWB","","","","",
	"IE","","","","","","","",
	0
};

/*
 * The hkfun[] array contains the text
 * used to print the decoded function bits
 * for the hkcs1 register.
 */

char	*hkfun[] =
{
	"SEL DRV",
	"PACK ACK",
	"DRIVE CLR",
	"UNLOAD",
	"START SPINDLE",
	"RECAL",
	"OFFSET",
	"SEEK",
	"READ DATA",
	"WRT DATA",
	"READ HDR",
	"WRT HDR",
	"WRT CK",
	0
};

/*
 * Text for decoded RL function fits.
 */
char	*rlfun[] =
{
	"NOP/MM",
	"WRT CHK",
	"GET STATUS",
	"SEEK",
	"RD HDR",
	"WRT DATA",
	"RD DATA",
	"RD NO HDR CK",
};

/*
 * Text for decoded RP function bits
 */
char	*rpfun[] =
{
	"IDLE",
	"WRITE",
	"READ",
	"WRT CHK",
	"SEEK",
	"WRT NO SEEK",
	"HOME SEEK",
	"READ NO SEEK",
};

/*
 Text for decoded RX function bits
 */
char	*rx2fun[] =
{
	"FILL BUF",
	"EMPTY BUF",
	"WRT SEC",
	"RD SEC",
	"SET DEN",
	"RD STAT",
	"WRT DEL DATA SEC",
	"RD ERR CODE",
};

/*
 * The tscom# [] arrays contain the text
 * for printing the TS function bit
 * expansions.
 */

char	*tscom1[] =
{
	"Read next (forward)",
	"Read previous (reverse)",
	"Reread previous (sp rev, read two)",
	"Reread next (sp fwd, read  rev)",
	0
};

char	*tscom8[] =
{
	"Space records forward",
	"Space records reverse",
	"Skip tape marks forward",
	"Skip tape marks reverse",
	"Rewind",
	0
};

char	*tscom9[] =
{
	"Write tape mark",
	"Erase",
	"Write tape mark entry (sp rev, erase, write TM)",
	0
};

char	*tscom10[] =
{
	"Message buffer release",
	"Rewind and unload",
	"Clean",
	0
};

/*
 * Message header word, message code and
 * class code bit expansion text arrays.
 */

char	*mshdr1[] =
{
	"On- or off-line",
	"Microdiagnostic failure",
	0
};

char	*mshdr2[] =
{
	"Serial bus parity error (packet bad)",
	"Other (ILC, ILA, NBA)",
	"Write lock error or non-executable function",
	"Microdiagnostic error",
	0
};

/*
 * The following four arrays are the bit
 * expansion text for the TS11 extended
 * status registers.
 */

char	*mstsx0[] =
{
	"TMK","RLS","LET","RLL","WLE","NEF","ILC","ILA",
	"MOT","ONL","IE","VCK","PED","WLK","BOT","EOT",
	0
};

char	*mstsx1[] =
{
	"DLT","","COR","CRS","TIG","DBF","SCK","",
	"IPR","SYN","IPO","IED","POS","POL","UNC","MTE",
	0
};

char	*mstsx2[] =
{
	"OPM","SIP","BPE","CAF","","WCF","","DTP",
	"DT7","DT6","DT5","DT4","DT3","DT2","DT1","DT0",
	0
};

char	*mstsx3[] =
{
	"","","","","","","","",
	"LMX","OPI","REV","CRF","DCK","NOI","LXS","RIB",
	0
};

char	*rlcss[] =
{
	"ERR","DE","","","","","","",
	"CRDY","IE","BA17","BA16","","","","DRDY",
	0
};

char	*rlmp[] =
{
	"WDE","CHE","WL","SKTO","SPE","WGE","VC","DSE",
	"DT","HS","CO","HO","BH","STC","STB","STA",
	0
};

char	*rx2css[] =
{
	"ERR","INIT","A17","A16","RX02","","","DEN",
	"TR","IE","DONE","USEL","","","","GO",
	0
};

char	*rx2es[] =
{
	"","","","","NXM","WCO","","USEL",
	"DRDY","DD","DEN","DEN ERR","AC LO","ID","","CRC",
	0
};
/*
 * UDA50/TK50 error status codes
 */

char	*ra_erstat[] =
{
	"Success",
	"Invalid command",
	"Command aborted",
	"Unit offline",
	"Unit available",
	"Media format error",
	"Write protected",
	"Compare error",
	"Data error",
	"Host buffer access error",
	"Controller error",
	"Drive error",
	"Formatter error",
	"BOT encountered",
	"Tape mark encountered",
	"",	/* unassigned */
	"Record data truncated",
	"Position lost",
	"Serious exception",
	"Leot detected",
	0
};

/*
 * UAD50/TK50 error message format codes
 */

char	*ra_efmc[] =
{
	"Controller error",
	"Host memory access error",
	"Disk transfer error",
	"SDI error",
	"Small disk error",
	"Tape transfer error",
	"STI communication/command error",
	"STI drive error",
	"STI formatter error",
	"Bad Block Replacement Attempt",
	0
};

char	*ra_nums[] =
{
	"One",
	"Two",
	"Three",
	"Four",
	"Five",
	"Six",
	"Seven",
	"Eight",
	0
};

/*
 * Text control array for other than the print text
 * for a set bit case.
 */

char *rhwc[] =	{"a",0};
char *rkda[] = {"b",0};
char *rpda[] = {"c",0};
/*	char *rfda[] = {"d",0};	no longer used */
/*	char 	hsda[] = {"e",0};	*/
char *hpda[] = {"f",0};
/*	char *hsdt[] = {"g",0};	*/
char *hpdt[] = {"h",0};
char *htdt[] = {"i",0};
char *rhsn[] = {"j",0};
char *rhca[] = {"k",0};
char *hpcs1[] = {"l",0};
char *hkcs1[] = {"m",0};
/*	char *hscs1[] = {"n",0};	*/
char *htcs1[] = {"o",0};
char *tsba[] = {"p",0};
char *tssr[] =	{"q",0};
char *tscom[] = {"r",0};
char *tsbc[] = {"s",0};
char *msbptr[] = {"t",0};
char *msbae[] = {"u",0};
char *mshdr[] = {"v",0};
char *mssiz[] = {"w",0};
char *msresid[] = {"x",0};
char	*mlda[] = {"y",0};
char	*mlmr[] = {"z",0};
char	*mlee[] = {"0",0};
char	*rlcs[] = {"1",0};
char	*rpcs[] = {"2",0};
char	*rx2cs[] = {"3",0};
char	*hkds[] = {"4",0};

/*
 * The following (  _rbp[]) arrays contain pointers to the
 * above control arrays which specify what is to be printed
 * after the contents of each device register.
 *
 * If the control array pointer is zero nothing is printed.
 * Otherwise the pointer is the address of the text control
 * array. The first character of which specifies how the
 * printing of the device register (details) is handled.
 *
 * The values of the control array first character are as follows:
 *
 * a - rhwc	Print the word count in decimal.
 * b - rkda	rk11 disk address.
 * c - rpda	rp11 disk address.
 * d - rfda	rf11 disk address. (NO LONGER USED !)
 * e - hsda	rs03/4 disk address. DEFUNCT
 * f - hpda	rp04/5/6 or rm02/3 disk address.
 * g - hsdt	rs03/4 drive type. DEFUNCT
 * h - hpdt	rp04/5/6 of rm02/3 drive type.
 * i - htdt	tm02/3 drive type.
 * j - rhsn	Print serial number in decimal.
 * k - rhca	Print cylinder address in decimal.
 * l - rhcs1	Print bits set (15 -> 6), function, GO
 * m - hkcs1	Print bits set (15 -> 6), function, GO
 * n - hscs1	Print bits set (15 -> 6), function, GO DEFUNCT
 * o - htcs1	Print bits set (15 -> 6), function, GO
 * p - tsba	Octal command packet address
 * q - tssr	Prints bits set (15 -> 6), FC, TCC
 * r - tscom	Prints bits set (15 -> 12, & 7), function
 * s - tsbc	Decimal positive initial byte count
 * t - msbptr	Message buffer address lo
 * u - msbae	"	"	"      hi
 * v - mshdr	Message buffer header word (bit expansion)
 * w - mssiz	Decimal message size in bytes
 * x - msresid	Decimal psoitive residual byte count
 * y - mlda	decimal block number
 * z - mlmr	array size, array type, xfer rate, bits < 7 -> 0>
 * 0 - mlee	bits < 15 -> 13>, channel, error function
 * 1 - rlcs	bits < 15 & 14 >, error, unit, bits < 7 -> 4 >, function, DRDY
 * 2 - rpcs	bits < 15 -> 4 >, function, GO
 * 3 - rx2cs	bits <15 -> 4 >, function, GO
 * 4 - hkds	bits <15 -> 9>, drive type, bits < 7 -> 0>
 *
 * (other)	Each element in the control array points to
 *		the text string to be printed if the bit
 *		in the register is set.
 */

int	*rk_rbp[] =
{
	&rkds,&rker,&rkcs,&rhwc,0,&rkda,0,0
};
int	*rp_rbp[] =
{
	&rpds,&rper,&rpcs,&rhwc,0,&rhca,&rpda,
	0,0,0,&rhca,0
};
/* This is a dummy array, it is not used ! */
int	*ra_rbp[] =
{
	0
};
int	*tm_rbp[] =
{
	&tmer,&tmcs,0,0,0,0
};
int	*tk_rbp[] =
{
	0
};
int	*hp_rbp[] =
{
	&hpcs1,&rhwc,0,&hpda,&rhcs2,&hpds,&hper1,&rhas,0,0,0,
	&hpdt,&rhsn,&hpof,&rhca,&rhca,&hper2,&hper3,0,0,&rhbae,&rhcs3
};
int	*ml_rbp[] =
{
	&hpcs1,&rhwc,0,&mlda,&rhcs2,&hpds,&mler,&rhas,0,0,mlmr,
	&hpdt,&rhsn,0,0,0,0,&mlee,&mlda,0,&rhbae,&rhcs3
};
int	*rm_rbp[] =
{
	&hpcs1,&rhwc,0,&hpda,&rhcs2,&hpds,&hper1,&rhas,0,0,0,
	&hpdt,&rhsn,&rmof,&rhca,&rhca,0,&rmer2,0,0,&rhbae,&rhcs3
};
int	*ht_rbp[] =
{
	&htcs1,&rhwc,0,0,&rhcs2,&htds,&hter,&rhas,
	0,0,0,&htdt,&rhsn,&httc,&rhbae,&rhcs3
};
int	*hk_rbp[] =
{
	&hkcs1,&rhwc,0,&hpda,&hkcs2,&hkds,&hkerr,&hkas,
	&rhca,0,0,0,0,0,0,0
};
int	*ts_rbp[] =
{
	&tsba,&tssr,&tscom,0,&rhbae,&tsbc,&msbptr,&msbae,
	0,0,&mshdr,&mssiz,&msresid,&mstsx0,&mstsx1,&mstsx2,&mstsx3
};

int	*rl_rbp[] =
{
	&rlcs, 0, 0, 0, &rlmp, &rhbae
};

int	*rx_rbp[] =
{
	&rx2cs, &rx2es
};
