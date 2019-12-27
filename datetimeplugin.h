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

#include "dde-dock/pluginsiteminterface.h"
#include "datetimewidget.h"
#include "calendarwidget.h"
#include <QTimer>
#include <QLabel>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QGroupBox>

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
    void alfaValue();

private:
    QPointer<DatetimeWidget> m_centralWidget;
    QPointer<CalendarWidget> m_calendarWidget;
    QPointer<QLabel> m_dateTipsLabel;
    QTimer *m_refershTimer;
    QSettings m_settings;
    void openCalendar();
    QVariant colorInt;
    int alfaInt;
    QPointer<QGroupBox> group1;
    QPointer<QGroupBox> group2;
    QPointer<QGroupBox> group3;
    QPointer<QCheckBox> clock;
    QPointer<QRadioButton> angle180;
    QPointer<QRadioButton> angle90;
    QPointer<QRadioButton> angle0;
    QPointer<QCheckBox> seconds;
    QPointer<QCheckBox> form;
    QPointer<QCheckBox> weekend;
    QPointer<QRadioButton> color0;
    QPointer<QRadioButton> color1;
    QPointer<QRadioButton> color2;
    QPointer<QRadioButton> color3;
    QPointer<QRadioButton> color4;
    QPointer<QRadioButton> color5;
    QPointer<QRadioButton> color6;
    QPointer<QRadioButton> color7;
    QPointer<QRadioButton> color8;
    QPointer<QRadioButton> color9;
    QPointer<QRadioButton> color10;
    QPointer<QRadioButton> color11;
    QPointer<QRadioButton> color12;
    QPointer<QRadioButton> color13;
    QPointer<QRadioButton> color14;
    QPointer<QRadioButton> color15;
    QPointer<QSlider> slider;
};

#endif // DATETIMEPLUGIN_H
