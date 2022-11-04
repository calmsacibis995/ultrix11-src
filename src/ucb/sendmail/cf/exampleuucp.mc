############################################################
############################################################
#####
#####		SENDMAIL CONFIGURATION FILE
#####
#####		Generic Uucp Configuration
#####
#####		@(#)exampleuucp.mc	3.0.1.1	(ULTRIX)	4/22/86
#####
############################################################
############################################################



############################################################
###	local info
############################################################

# domain
DDUUCP
CDUUCP

# official hostname
Dj$w.$D

# UUCP name
DU$w

include(base.m4)

include(zerobase.m4)

################################################
###  Machine dependent part of ruleset zero  ###
################################################

# resolve names we can handle locally
R<@$+.UUCP>:$+		$1!$2				to old format
R$+<@$+.UUCP>		$2!$1				to old format
R$-!$+			$#uucp$@$1$:$2			host!user

# everything else must be a local name
R$+			$#local$:$1			local names

include(localm.m4)
include(xm.m4)
include(uucpm.m4)
