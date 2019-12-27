#!/bin/bash
sudo cp `dirname $0`/libclock.so /usr/lib/dde-dock/plugins/libclock.so
killall dde-dock