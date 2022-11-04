
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*	SCCSID: @(#)tablqp02.c	3.0	4/22/86	*/
/*
 * LQP02 letter printer
 * nroff driving table
 * by Susan Smith
 */

#define INCH 240
struct termtable tlqp02 {
/*bset*/        0,		
/*breset*/	0,	
/*Hor*/ 	INCH/120,
/*Vert*/	INCH/12,	
/*Newline*/	INCH/6,	
/*Char*/	INCH/10,	
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/120,
/*twinit*/	"\033[?27l\033[w\033[z",
/*twrest*/	"\033[0w\033[0z",
/*twnl*/	"\n",		
/*hlr*/		"\033L",
/*hlf*/		"\033K",	
/*flr*/		"\033M",	
/*bdon*/	"\033[1m",
/*bdoff*/	"\033[0m",
/*ploton*/	"\033[;6 G",
/*plotoff*/	"\033[w",	
/*up*/		"\033L",	
/*down*/	"\033K",
/*right*/	" ",
/*left*/	"\b",

/* codetab */

"\001 ",	/*  space				    */
"\201!",	/*  exclamation point			    */
"\201\"",	/*  double quotation mark		    */
"\201#",	/*  pound sign				    */
"\201$",	/*  dollar sign				    */
"\201%",	/*  percent sign			    */
"\201&",	/*  ampersand				    */
"\201'",	/*  close single quotation mark		    */
"\201(",	/*  open parenthesis			    */
"\201)",	/*  close parenthesis			    */
"\201*",	/*  asterisk				    */
"\201+",	/*  plus				    */
"\201,",	/*  comma				    */
"\201-",	/*  hyphen				    */
"\201.",	/*  period				    */
"\201/",	/*  slash				    */
"\2010",	/*  zero				    */
"\2011",	/*  one					    */
"\2012",	/*  two					    */
"\2013",	/*  three				    */
"\2014",	/*  four				    */
"\2015",	/*  five				    */
"\2016",	/*  six					    */
"\2017",	/*  seven				    */
"\2018",	/*  eight				    */
"\2019",	/*  nine				    */
"\201:",	/*  colon				    */
"\201;",	/*  semicolon				    */
"\201<",	/*  open angle bracket			    */
"\201=",	/*  equal				    */
"\201>",	/*  close angle bracket			    */
"\201?",	/*  question mark			    */
"\201@",	/*  commercial at			    */
"\201A",	/*  uppercase A				    */
"\201B",	/*  uppercase B				    */
"\201C",	/*  uppercase C				    */
"\201D",	/*  uppercase D				    */
"\201E",	/*  uppercase E				    */
"\201F",	/*  uppercase F				    */
"\201G",	/*  uppercase G				    */
"\201H",	/*  uppercase H				    */
"\201I",	/*  uppercase I				    */
"\201J",	/*  uppercase J				    */
"\201K",	/*  uppercase K				    */
"\201L",	/*  uppercase L				    */
"\201M",	/*  uppercase M				    */
"\201N",	/*  uppercase N				    */
"\201O",	/*  uppercase O				    */
"\201P",	/*  uppercase P				    */
"\201Q",	/*  uppercase Q				    */
"\201R",	/*  uppercase R				    */
"\201S",	/*  uppercase S				    */
"\201T",	/*  uppercase T				    */
"\201U",	/*  uppercase U				    */
"\201V",	/*  uppercase V				    */
"\201W",	/*  uppercase W				    */
"\201X",	/*  uppercase X				    */
"\201Y",	/*  uppercase Y				    */
"\201Z",	/*  uppercase Z				    */
"\201[",	/*  open bracket			    */
"\201\\",	/*  back slash				    */
"\201]",	/*  close bracket			    */
"\201^",	/*  circumflex				    */
"\201_",	/*  under dash				    */
"\201`",	/*  open single quotation mark		    */
"\201a",	/*  lowercase A				    */
"\201b",	/*  lowercase B				    */
"\201c",	/*  lowercase C				    */
"\201d",	/*  lowercase D				    */
"\201e",	/*  lowercase E				    */
"\201f",	/*  lowercase F				    */
"\201g",	/*  lowercase G				    */
"\201h",	/*  lowercase H				    */
"\201i",	/*  lowercase I				    */
"\201j",	/*  lowercase J				    */
"\201k",	/*  lowercase K				    */
"\201l",	/*  lowercase L				    */
"\201m",	/*  lowercase M				    */
"\201n",	/*  lowercase N				    */
"\201o",	/*  lowercase O				    */
"\201p",	/*  lowercase P				    */
"\201q",	/*  lowercase Q				    */
"\201r",	/*  lowercase R				    */
"\201s",	/*  lowercase S				    */
"\201t",	/*  lowercase T				    */
"\201u",	/*  lowercase U				    */
"\201v",	/*  lowercase V				    */
"\201w",	/*  lowercase W				    */
"\201x",	/*  lowercase X				    */
"\201y",	/*  lowercase Y				    */
"\201z",	/*  lowercase Z				    */
"\201{",	/*  open brace				    */
"\201|",	/*  vertical bar			    */
"\201}",	/*  close brace				    */
"\201~",	/*  tilde				    */
"\000",		/*  narrow space			    */
"\201-",	/*  hyphen				    */
"\201o",	/*  bullet				    */
"\202[]",	/*  square				    */
"\201-",	/*  3/4 M dash				    */
"\001_",	/*  rule				    */
"\2031/4",	/*  1/4					    */
"\2031/2",	/*  1/2					    */
"\2033/4",	/*  3/4					    */
"\201-",	/*  minus				    */
"\202fi",	/*  fi					    */
"\202fl",	/*  fl					    */
"\202ff",	/*  ff					    */
"\203ffi",	/*  ffi					    */
"\203ffl",	/*  ffl					    */
"\201\341o\301",/*  degree				    */
"\201|\b-",	/*  dagger				    */
"\000",		/*  section				    */
"\201'",	/*  foot mark				    */
"\201'",	/*  acute accent			    */
"\201`",	/*  grave accent			    */
"\001_",	/*  underrule				    */
"\001/",	/*  longer slash			    */
"\000",		/*  half narrow space			    */
"\001 ",	/*  unpaddable space			    */
"\000",		/*  lowercase alpha			    */
"\000",		/*  lowercase beta			    */
"\000",		/*  lowercase gamma			    */
"\000",		/*  lowercase delta			    */
"\000",		/*  lowercase epsilon			    */
"\000",		/*  lowercase zeta			    */
"\000",		/*  lowercase eta			    */
"\000",		/*  lowercase theta			    */
"\000",		/*  lowercase iota			    */
"\000",		/*  lowercase kappa			    */
"\000",		/*  lowercase lambda			    */
"\000",		/*  lowercase mu			    */
"\000",		/*  lowercase nu			    */
"\000",		/*  lowercase xi			    */
"\000",		/*  lowercase omicron			    */
"\000",		/*  lowercase pi			    */
"\000",		/*  lowercase rho			    */
"\000",		/*  lowercase interior sigma		    */
"\000",		/*  lowercase tau			    */
"\000",		/*  lowercase upsilon			    */
"\000",		/*  lowercase phi			    */
"\000",		/*  lowercase chi			    */
"\000",		/*  lowercase psi			    */
"\000",		/*  lowercase omega			    */
"\000",		/*  uppercase gamma			    */
"\000",		/*  uppercase delta			    */
"\000",		/*  uppercase theta			    */
"\000",		/*  uppercase lambda			    */
"\000",		/*  uppercase xi			    */
"\000",		/*  uppercase pi			    */
"\000",		/*  uppercase sigma			    */
"\000",		/*  <unknown>				    */
"\000",		/*  uppercase upsilon			    */
"\000",		/*  uppercase phi			    */
"\000",		/*  uppercase psi			    */
"\000",		/*  uppercase omega			    */
"\000",		/*  square root				    */
"\000",		/*  lowercase terminal sigma		    */
"\000",		/*  rooten				    */
"\001>\b_",	/*  >=					    */
"\001<\b_",	/*  <=					    */
"\000",		/*  identically equal			    */
"\201-",	/*  equation minus			    */
"\000",		/*  approximately equal			    */
"\000",		/*  approximates			    */
"\001=\b|",	/*  not equal				    */
"\202->",	/*  right arrow				    */
"\202<-",	/*  left arrow				    */
"\201^\b|",	/*  up arrow				    */
"\201v\b|",	/*  down arrow				    */
"\001=",	/*  equation equal			    */
"\201*",	/*  multiply				    */
"\201/",	/*  divide				    */
"\001_\b+",	/*  plus minus				    */
"\000",		/*  union				    */
"\000",		/*  intersection			    */
"\000",		/*  proper subset of			    */
"\000",		/*  proper superset of			    */
"\000",		/*  improper subset of			    */
"\000",		/*  improper superset of		    */
"\202oo",	/*  infinity				    */
"\201o\b)",	/*  partial derivative			    */
"\000",		/*  gradient				    */
"\201\033O\"",	/*  logical not (decimal address 127)	    */
"\000",		/*  small integral sign			    */
"\000",		/*  proportional to			    */
"\000",		/*  empty set				    */
"\000",		/*  member of				    */
"\201+",	/*  equation plus			    */
"\000",		/*  registered				    */
"\201O/bc",	/*  copyright				    */
"\001|",	/*  box rule				    */
"\201\033O!",	/*  cent sign (decimal address 32)	    */
"\201|\b=",	/*  double dagger			    */
"\000",		/*  right hand				    */
"\000",		/*  left hand				    */
"\201*",	/*  mathematical star			    */
"\000",		/*  Bell System logo			    */
"\201|",	/*  logical or				    */
"\000",		/*  circle				    */
"\001|",	/*  left top curly brace		    */
"\001|",	/*  left bottom curly brace		    */
"\001|",	/*  right top curly brace		    */
"\001|",	/*  right bottom curly brace		    */
"\001|",	/*  left center curly brace		    */
"\001|",	/*  right center curly brace		    */
"\201|",	/*  bold vertical bar			    */
"\001+",	/*  left bottom square bracket		    */
"\001+",	/*  right bottom square bracket		    */
"\001+",	/*  left top square bracket		    */
"\001+"};	/*  right top square bracket		    */
