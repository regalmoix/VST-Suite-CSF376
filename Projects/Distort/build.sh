#!/bin/bash

# Credits : https://aplicacionesysistemas.com/en/dialog-crear-menus-tus-scripts/

clear 

                
buildmode=Release
buildtarget=all

HEIGHT=15
WIDTH=40
CHOICE_HEIGHT=4


BACKTITLE="Build Plugin"
TITLE="Select Targets"
MENU="Choose one of the following options:"

OPTIONS=(1 "VST3 + Standalone"
         2 "Clean Existing Build"
         3 "VST3"
         4 "Standalone")

CHOICE=$(dialog --clear \
                --backtitle "$BACKTITLE" \
                --title "$TITLE" \
                --menu "$MENU" \
                $HEIGHT $WIDTH $CHOICE_HEIGHT \
                "${OPTIONS[@]}" \
                2>&1 >/dev/tty)

case $CHOICE in
        1)
            buildtarget=all
            ;;
        2)
            buildtarget=clean
            ;;
        3)
            buildtarget=VST3
            ;;
        4)
            buildtarget=Standalone
            ;;
esac




if [[ "$buildtarget" != "clean" ]]
then
    BACKTITLE="Build Plugin"
    TITLE="Build Options"
    MENU="Choose one of the following options:"

    OPTIONS=(1 "Debug" 2 "Release")

    CHOICE=$(dialog --clear \
                    --backtitle "$BACKTITLE" \
                    --title "$TITLE" \
                    --menu "$MENU" \
                    $HEIGHT $WIDTH $CHOICE_HEIGHT \
                    "${OPTIONS[@]}" \
                    2>&1 >/dev/tty)
                    
    case $CHOICE in
            1)
                buildmode=Debug
                ;;
            2)
                buildmode=Release
                ;;
    esac
fi
clear

echo "Selected Targets    : $buildtarget"

if [[ "$buildtarget" != "clean" ]]
then
    echo "Selected Build Mode : $buildmode"
fi

#echo "Running Command : "
#echo make $buildtarget CONFIG=$buildmode -C Builds/LinuxMakefile/
make -j8  $buildtarget CONFIG=$buildmode -C Builds/LinuxMakefile/
#dialog --checklist "Choose the options you want:" 0 0 0  1 cheese on 2 "Mustard" on  3 anchovies off
