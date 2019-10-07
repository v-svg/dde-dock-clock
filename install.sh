#!/bin/bash
sudo cp `dirname $0`/translations/dde-dock_es.qm /usr/share/dde-dock/translations/dde-dock_es.qm
sudo cp `dirname $0`/translations/dde-dock_ru.qm /usr/share/dde-dock/translations/dde-dock_ru.qm
sudo cp `dirname $0`/translations/dde-dock_uk.qm /usr/share/dde-dock/translations/dde-dock_uk.qm
sudo cp `dirname $0`/translations/dde-dock_zh_CN.qm /usr/share/dde-dock/translations/dde-dock_zh_CN.qm
sudo cp `dirname $0`/libclock.so /usr/lib/dde-dock/plugins/libclock.so
killall dde-dock