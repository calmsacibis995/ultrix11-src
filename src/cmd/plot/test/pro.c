
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)pro.c	3.0	4/22/86 */
main()
{
int x,y;
openpl();
erase();
line( 20, 30, 20, 60);
line( 20, 60, 40, 60);
line( 40, 60, 40, 45);
line( 40, 45, 20, 45);
line( 50, 30, 50, 60);
line( 50, 60, 70, 60);
line( 70, 60, 70, 45);
line( 70, 45, 50, 45);
line( 60, 45, 70, 30);
line( 80, 30, 80, 60);
line( 80, 60,100, 60);
line(100, 60,100, 30);
line(100, 30, 80, 30);
line(110, 30,130, 60);
line(140, 60,150, 30);
line(150, 30,160, 60);
line(170, 60,190, 60);
line(190, 60,170, 30);
line(200, 30,200, 60);
line(200, 60,210, 45);
line(210, 45,220, 60);
line(220,60,220,30);
closepl();
x=90;y=130;
openpl();
erase();
line(x+19,y+5,x+105,y+5);     /* base line */
line(x+105,y+5,x+105,y+10);   /* right	  */
line(x+19,y+5,x+19,y+10);     /* left  */
line(x+10,y+12,x+111,y+12);   /* top base */

line(x+10,y+12,x+8,y+17);
line(x+8,y+17,x+10,y+38);
line(x+10,y+38,x+110,y+38);
line(x+111,y+38,x+113,y+19);	 /* right side */
line(x+113,y+19,x+111,y+12);	 /* right side */
x=80; y= 60;
     /* screen */
line(x+15,y+10,x+18,y+5);
line(x+18,y+5,x+40,y+2);
line(x+40,y+2,x+88,y+12);
line(x+88,y+12,x+93,y+30);
line(x+93,y+30,x+62,y+53);
line(x+63,y+53,x+61,y+59);
line(x+61,y+59,x+60,y+57);
line(x+60,y+59,x+55,y+62);
line(x+55,y+62,x+39,y+10);
line(x+39,y+10,x+37,y+8);
line(x+37,y+8,x+2,y+14);
line(x+2,y+14,x+0,y+15);
line(x+0,y+15,x+15,y+60);
line(x+15,y+60,x+16,y+61);
line(x+16,y+61,x+20,y+62);
line(x+20,y+62,x+45,y+64);
line(x+45,y+64,x+46,y+63);
line(x+45,y+64,x+55,y+62);
line(x+46,y+63,x+31,y+13);
line(x+31,y+13,x+1,y+18);

line(x+6,y+25,x+17,y+58);
line(x+17,y+58,x+18,y+59);
line(x+18,y+59,x+19,y+60);
line(x+19,y+60,x+40,y+61);
line(x+40,y+61,x+41,y+60);
line(x+41,y+60,x+42,y+59);
line(x+42,y+59,x+30,y+20);
line(x+30,y+20,x+29,y+19);
line(x+29,y+19,x+28,y+18);
line(x+28,y+18,x+5,y+21);
line(x+5,y+21,x+5,y+23);
line(x+5,y+24,x+6,y+25);

/*key board*/
line(5,48,5,50);
line(5,50,24,65);
line(24,65,120,29);
line(120,29,120,21);
line(120,21,95,1);
line(95,1,5,48);
line(120,29,95,4);
line(95,4,5,50);
closepl();
}
