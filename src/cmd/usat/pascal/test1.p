
{ SCCSID: @(#)test1.p	3.1	9/17/87 }
{USAT TEST FOR PASCAL}
 
{: This program generates a table
   of temperature conversions for
   Celsius, Fahrenheit, and Kelvin. }
 
program test1(output);
const
   banner = 'Celsius	Fahrenheit	Kelvin';
var
   i: 1 .. 101;
   xk, xc, xf: real;
begin
   writeln;
   writeln(banner);
   xk := 0;
   for i := 1 to 101 do
      begin
          xc := xk - 273.16;
          xf := ((9.0/5.0)*(xc+32.0));
          writeln(xc:2, xf:2, xk:2);
	  xk := xk+5;
      end
end.
