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
CPDEST="../../examples/updateCellular/"

if [ -f $OAPATH ]; then
    if [ -d $CPDEST ]; then
        echo "Copying $OAPATH to fw updater (cellular) sketch folder ($CPDEST)"
        mv $OAPATH $CPDEST 
    fi

    echo "Moving $OAPATH to fw updater sketch folder ($DST)"
    mv $OAPATH $DST 
else
    echo "File $OAPATH not found"
fi

if [ -f $ODPATH ]; then

    if [ -d $CPDEST ]; then
        echo "Copying $ODPATH to fw updater (cellular) sketch folder ($CPDEST)"
        mv $ODPATH $CPDEST 
    fi

    echo "Moving $ODPATH to fw updater sketch folder ($DST)"
	 mv $ODPATH $DST 
else
    echo "File $ODPATH not found"
fi

if [ -f $OCPATH ]; then
    if [ -d $CPDEST ]; then
        echo "Copying $OCPATH to fw updater (cellular) sketch folder ($CPDEST)"
        mv $OCPATH $CPDEST
    fi 

    echo "Moving $OCPATH to fw updater sketch folder ($DST)"
     mv $OCPATH $DST 
else
    echo "File $OCPATH not found"
fi
