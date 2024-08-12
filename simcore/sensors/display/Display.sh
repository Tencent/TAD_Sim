#!/bin/bash

# Display.sh -name=Display_clone -address=172.17.0.1:21302 -device=0 -graphicsadapter=0 -RenderOffscreen

UE4_TRUE_SCRIPT_NAME=$(echo \"$0\" | xargs readlink -f)
UE4_PROJECT_ROOT=$(dirname "$UE4_TRUE_SCRIPT_NAME")

UDirKey="UserDir="
UDIR="$HOME/.config/tadsim/data/service_data/display"
for i in $@
do
    if [[ $i == $UDirKey* ]]
    then
	UDIR=${i#*=}	
    fi
done
if [ ! -d $UDIR ]
then
    mkdir $UDIR
fi
chmod 777 -R $UDIR

DV="-device="
DID = "0"
for i in $@
do
    if [[ $i == $DV* ]]
    then
	DID=${i#*=}	
    fi
done


_term() { 
  echo "Caught SIGTERM signal!" 
  kill -TERM "$child" 2>/dev/null
}
trap _term TERM QUIT

#echo "$UE4_PROJECT_ROOT/Display/Binaries/Linux/Display" $@ $C -log LOG=Display${A}.${B}.log

export SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS=0

if [ -f /etc/os-release ]; then
    # freedesktop.org and systemd
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
elif type lsb_release >/dev/null 2>&1; then
    # linuxbase.org
    OS=$(lsb_release -si)
    VER=$(lsb_release -sr)
elif [ -f /etc/lsb-release ]; then
    # For some versions of Debian/Ubuntu without lsb_release command
    . /etc/lsb-release
    OS=$DISTRIB_ID
    VER=$DISTRIB_RELEASE
elif [ -f /etc/debian_version ]; then
    # Older Debian/Ubuntu/etc.
    OS=Debian
    VER=$(cat /etc/debian_version)
elif [ -f /etc/SuSe-release ]; then
    # Older SuSE/etc.
    ...
elif [ -f /etc/redhat-release ]; then
    # Older Red Hat, CentOS, etc.
    ...
else
    # Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
    OS=$(uname -s)
    VER=$(uname -r)
fi

echo $OS
echo $VER

if [[ $VER == 16.04 ]]
then
	export LD_LIBRARY_PATH="$UE4_PROJECT_ROOT/Display/Binaries/Linux/ubuntu16/:$LD_LIBRARY_PATH"
else
	export LD_LIBRARY_PATH="$UE4_PROJECT_ROOT/Display/Binaries/Linux/ubuntu18_20/:$LD_LIBRARY_PATH"
fi

exec "$UE4_PROJECT_ROOT/Display/Binaries/Linux/Display" $@ UserDir=$UDIR -LocalLogTimes -log LOG=Display${DID}.log

child=$! 
wait "$child"
