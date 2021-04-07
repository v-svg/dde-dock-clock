#!/bin/bash
BRed='\033[1;31m'
BBlue='\033[1;34m'
NC='\033[0m'
 
sudo rm /usr/lib/dde-dock/plugins/libclock.so
echo -e "Remove ${BRed}libclock.so${NC} from ${BBlue}/usr/lib/dde-dock/plugins/${NC}"
sudo rm -r /usr/share/dde-dock-clock
echo -e "Remove ${BRed}/usr/share/dde-dock-clock/${NC}"
killall dde-dock
