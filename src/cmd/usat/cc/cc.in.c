
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *	SCCSID: @(#)cc.in.c	3.0	4/22/86
 *
 * This program prints a conversion table
 * of temperatures for Celsius, Fahrenheit,
 * and Kelvin.
 */

#define	CTOK	273.16
#define	MAXK	500	
#define	MINK	0
#define	INC	5

char	heading[] =	"\nCelsius\t\tFahrenheit\tKelvin";
char	fmt2[] =	"%6.2f\t\t";

main()
{
	int	i;
	float	c,f,k,r;

	printf(heading);
	for(k=(MINK); k<=MAXK; k=k+INC) {
		printf("\n");
		c = k - CTOK;
		printf(fmt2,c);
		f = ctof(c);
		printf(fmt2,k);
	}
	printf("\n");
	exit(0);
}
/*
 * convert f to c
 */
ctof(c)
float c;
{
	float	f;

	f = ((9.0/5.0) * (c + 32.));
	printf(fmt2,f);
}
