# ---------------------------------------------------------------------------- #
#  FILE NAME:   cpFw.sh
#  AUTHOR:      Daniele Aimo
#  EMAIL:       maidnl74@gmail.com
#  DATE:        20240123
#  DESCRIPTION: Copy the FW into the fw updater sketch 
#  ARGUMENTS:   
#  NOTES:       
# ---------------------------------------------------------------------------- #

OAPATH="./fwUpdateAnalog.h"
ODPATH="./fwUpdateDigital.h"
OCPATH="./fwUpdateCellular.h"
DST="../../examples/updateExpansionFw/"

if [ -f $OAPATH ]; then
    echo "Moving $OAPATH to fw updater sketch folder ($DST)"
	 mv $OAPATH $DST 
else
    echo "File $OAPATH not found"
fi

if [ -f $ODPATH ]; then
    echo "Moving $ODPATH to fw updater sketch folder ($DST)"
	 mv $ODPATH $DST 
else
    echo "File $ODPATH not found"
fi

if [ -f $OCPATH ]; then
    echo "Moving $OCPATH to fw updater sketch folder ($DST)"
     mv $OCPATH $DST 
else
    echo "File $OCPATH not found"
fi
