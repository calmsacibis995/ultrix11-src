egrep "\*\*\*\*\*\*|^real|^user|^sys" | \
sed s/:/\	\	/g | \
awk '
	{ if (($1 == "******") && ($2 != "End"))  print $0 }

	{ if (($1 == "real") && ($3 == "")) {   flag++;
						r=r+$2;
						if(r>60) {
							r=r-60;
							rm=rm+1;
						}
						} }
	{ if (($1 == "real") && ($3 != "")) {   flag++;
						rm=rm+$2;
						r=r+$3;
						if(r>60) {
							r=r-60;
							rm=rm+1;
						}
						} }

	{ if (($1 == "user") && ($3 == "")) {   flag++;
						u=u+$2;
						if(u>60) {
							u=u-60;
							um=um+1;
						}
						} }
	{ if (($1 == "user") && ($3 != "")) {   flag++;
						um=um+$2;
						u=u+$3;
						if(u>60) {
							u=u-60;
							um=um+1;
						}
						} }
	{ if (($1 == "sys") && ($3 == "")) {    flag++;
						y=y+$2;
						if(y>60) {
							y=y-60;
							ym=ym+1;
						}
						} }
	{ if (($1 == "sys") && ($3 != "")) {    flag++;
						ym=ym+$2;
						y=y+$3;
						if(y>60) {
							y=y-60;
							ym=ym+1;
						}
						} }
	{ if (($2 == "End") && (flag > 0) && ($1=="******")) {
		printf("real:\t");
		if (rm <=0) printf("0:");
		if (rm >0)  printf(rm":");
		if (r<10)   printf("0");
		if (r<1)    printf("0");
		if (r==0)   printf(".");
		printf(r);

		printf("\nuser:\t");
		if (um <=0) printf("0:");
		if (um >0)  printf(um":");
		if (u<10)   printf("0");
		if (u<1)    printf("0");
		if (u==0)   printf(".");
		printf(u);

		printf("\nsys:\t");
		if (ym <=0) printf("0:");
		if (ym >0)  printf(ym":");
		if (y<10)   printf("0");
		if (y<1)    printf("0");
		if (y==0)   printf(".");
		printf(y);

		r=u=y=0;
		rm=um=ym=0; 
		flag=0;
		printf("\n"); } }
	{ if (($2 == "End")  && ($1 == "******")) { print $0; } }
		'	| \
sed s/\	\	/:/g	|   \
sed s/:/\	\	/g | \
awk '
	{ if ($1 != "******") print $0 }
	{ if ($1 == "real") r=$2; }
	{ if ($1 == "user") u=$2; }
	{ if ($1 == "sys") y=$2; }
	{ if ($1 == "******") {
		if ($2 != "End") {
			{ print $0 }
			{ if ($5 == "-") { ha=$9; ma=$10; sa=$11; } }
			{ if ($6 == "-") { ha=$10; ma=$11; sa=$12; } }
			{ if ($7 == "-") { ha=$11; ma=$12; sa=$13; } }
		}
		if ($2 == "End") {
			{ if ($5 == "-") { hb=$9; mb=$10; sb=$11; } }
			{ if ($6 == "-") { hb=$10; mb=$11; sb=$12; } }
			{ if ($7 == "-") { hb=$11; mb=$12; sb=$13; } }

			{ if(sb>=sa) s=sb-sa; }
			{ if(sb<sa) { mb=mb-1; s=sb+60-sa; } }
			{ if(mb>=ma) m=mb-ma; }
			{ if(mb<ma) { hb=hb-1; m=mb+60-ma; } }
			{ if(hb>=ha) h=hb-ha; }
			{ if(hb<ha) h=hb+24-ha; }

			{ print $0 }
			{ printf("\n\tTotal Elapsed Time: "); }
			{ if(h<10) printf("0"); }
			{ printf(h":"); }
			{ if(m<10) printf("0"); }
			{ printf(m":"); }
			{ if(s<10) printf("0"); }
			{ printf(s"\n") }
			{ h=m=s=0; }
			{ ha=ma=sa=0; }
			{ hb=mb=sb=0; }
		}
	} }
		'	| \
sed s/\	\	/:/g
