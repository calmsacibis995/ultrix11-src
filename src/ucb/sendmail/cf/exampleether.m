############################################################
############################################################
#####
#####		SENDMAIL CONFIGURATION FILE
#####
#####		Generic Arpa Configuration	
#####
#####		@(#)exampleether.m4	3.0	(ULTRIX)	4/22/86
#####
############################################################
############################################################



############################################################
###	local info
############################################################

# internet hostname
DA$w

# domain
DDARPA
CDARPA

# official hostname
Dj$w.$D

include(base.m4)

include(zerobase.m4)

###############################################
###  Machine dependent part of rulset zero  ###
###############################################

# resolve names we can handle locally
R$*<@$*$-.ARPA>$*	$#tcp$@$3$:$1<@$2$3.ARPA>$4	user@tcphost.ARPA
R$*<@$*$->$*		$#tcp$@$3$:$1<@$2$3>$4		user@tcphost.ARPA

# everything else must be a local name
R$+			$#local$:$1			local names

include(localm.m4)
include(xm.m4)
include(tcpm.m4)
