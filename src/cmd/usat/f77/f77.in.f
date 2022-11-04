C #
C ######################################################################
C #   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
C #   All Rights Reserved. 					       #
C #   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
C ######################################################################
C #

	parameter (i = 100)
	print 6
6	format(' ', "\n  Celsius\tFahrenheit\t   Kelvin")
	xk=0
	do 99 j=0, i
	xc = xk - 273.16
	xf = ((9.0/5.0)*(xc+32.0))
	print 22, xc, xf, xk
22	format(' ', 3(f8.2"\t"))
	xk=xk+5
99	continue
	end
