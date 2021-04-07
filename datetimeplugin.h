/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DATETIMEPLUGIN_H
#define DATETIMEPLUGIN_H

#include <QLabel>
#include <QTimer>
#include <QSettings>
#include <QRadioButton>

#include "dde-dock/pluginsiteminterface.h"
#include "datetimewidget.h"
#include "calendarwidget.h"

class DatetimePlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "datetime.json")

public:
    explicit DatetimePlugin(QObject *parent = nullptr);

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;

    int itemSortKey(const QString &itemKey);
    void setSortKey(const QString &itemKey, const int order);
    void set();

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey);

    const QString itemContextMenu(const QString &itemKey) override;

    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

private slots:
    void updateCurrentTimeString();

private:
    DatetimeWidget *m_centralWidget;
    CalendarWidget *m_calendarWidget;
    QLabel *m_dateTipsLabel;
    QTimer *m_refershTimer;
    QSettings m_settings;
    QRadioButton *colorButton1;
    QRadioButton *colorButton2;
    QRadioButton *colorButton3;
    QRadioButton *colorButton4;
    QRadioButton *colorButton5;
    QRadioButton *colorButton6;
    QRadioButton *colorButton7;
    QRadioButton *colorButton8;
    QRadioButton *colorButton9;
    QRadioButton *colorButton10;
    QRadioButton *colorButton11;
    QRadioButton *colorButton12;
    QRadioButton *colorButton13;
    QRadioButton *colorButton14;
    QRadioButton *colorButton15;
    QRadioButton *colorButton16;
    int colorAlfa = 129;
    int btnRadius = 15;
    void setButtonStyle();
    void openCalendar();
};

#endif // DATETIMEPLUGIN_H
