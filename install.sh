#!/bin/bash
BGreen='\033[1;32m'
BBlue='\033[1;34m'
NC='\033[0m'

ts_list=(`ls translations/*.ts`)
for ts in "${ts_list[@]}"
do
    printf "\nProcess ${ts}\n"
    lrelease "${ts}"
done
strip libclock.so
sudo cp libclock.so /usr/lib/dde-dock/plugins/libclock.so
echo -e "Copy ${BGreen}libclock.so${NC} to ${BBlue}/usr/lib/dde-dock/plugins/${NC}"
sudo mkdir -p /usr/share/dde-dock-clock/translations
sudo cp translations/*.qm /usr/share/dde-dock-clock/translations/
echo -e "Copy ${BGreen}*.qm${NC} to ${BBlue}/usr/share/dde-dock-clock/translations/${NC}"
killall dde-dock
