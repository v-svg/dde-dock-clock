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

#include "datetimeplugin.h"
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QFormLayout>
#include <QFontMetrics>

DatetimePlugin::DatetimePlugin(QObject *parent)
    : QObject(parent),
      m_dateTipsLabel(new QLabel),
      m_refershTimer(new QTimer(this)),
      m_settings("deepin", "dde-dock-clock")
{
    m_dateTipsLabel->setObjectName("clock");
    m_dateTipsLabel->setStyleSheet("color:white; padding:0px 3px;");

    m_refershTimer->setInterval(1000);
    m_refershTimer->start();

    m_centralWidget = new DatetimeWidget;
    m_calendarWidget = new CalendarWidget;

    connect(m_centralWidget, &DatetimeWidget::requestUpdateGeometry,
           [this] { m_proxyInter->itemUpdate(this, pluginName()); });
    connect(m_refershTimer, &QTimer::timeout,
            this, &DatetimePlugin::updateCurrentTimeString);
}

const QString DatetimePlugin::pluginName() const
{
    return "clock";
}

const QString DatetimePlugin::pluginDisplayName() const
{
    return tr("Clock");
}

void DatetimePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    if (m_centralWidget->enabled())
        m_proxyInter->itemAdded(this, pluginName());
}

void DatetimePlugin::pluginStateSwitched()
{
    m_centralWidget->setEnabled(!m_centralWidget->enabled());

    if (m_centralWidget->enabled())
        m_proxyInter->itemAdded(this, pluginName());
    else
        m_proxyInter->itemRemoved(this, pluginName());
}

bool DatetimePlugin::pluginIsDisable()
{
    return !m_centralWidget->enabled();
}

int DatetimePlugin::itemSortKey(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1").arg(displayMode());
    return m_settings.value(key, 0).toInt();
}

void DatetimePlugin::setSortKey(const QString &itemKey, const int order)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1").arg(displayMode());
    m_settings.setValue(key, order);
}

QWidget *DatetimePlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_centralWidget;
}

QWidget *DatetimePlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_dateTipsLabel;
}

QWidget *DatetimePlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    m_calendarWidget->update();
    return m_calendarWidget;
}

const QString DatetimePlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();

    QList<QVariant> items;
    items.reserve(4);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = tr("Open Calendar");
    open["isActive"] = true;
    items.push_back(open);

    if (displayMode == Dock::Fashion && m_settings.value("ShowClock").toBool() == false) {
        QMap<QString, QVariant> clock;
        clock["itemId"] = "clock";
        clock["itemText"] = tr("Analogue Clock");
        clock["isActive"] = true;
        items.push_back(clock);
    }

    QMap<QString, QVariant> settings;
    settings["itemId"] = "settings";
    if (m_centralWidget->is24HourFormat())
        settings["itemText"] = tr("12 Hour Time");
    else
        settings["itemText"] = tr("24 Hour Time");
    settings["isActive"] = true;
    items.push_back(settings);

    QMap<QString, QVariant> set;
    set["itemId"] = "set";
    set["itemText"] = tr("Plugin Settings");
    set["isActive"] = true;
    items.push_back(set);

    QMap<QString, QVariant> tset;
    tset["itemId"] = "tset";
    tset["itemText"] = tr("Time Settings");
    tset["isActive"] = true;
    items.push_back(tset);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DatetimePlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    if (menuId == "clock")
        m_centralWidget->toggleClock();
    if (menuId == "set")
        set();
    if (menuId == "settings")
        m_centralWidget->toggleHourFormat();
    if (menuId == "tset")
        QProcess::startDetached("dbus-send --print-reply --dest=com.deepin.dde.ControlCenter /com/deepin/dde/ControlCenter com.deepin.dde.ControlCenter.ShowModule \"string:datetime\"");
    if (menuId == "open")
        openCalendar();
}

void DatetimePlugin::updateCurrentTimeString()
{
    const QDateTime currentDateTime = QDateTime::currentDateTime();

    if (m_centralWidget->is24HourFormat())
        m_dateTipsLabel->setText(currentDateTime.date().toString(Qt::SystemLocaleLongDate)
                         + currentDateTime.toString(" HH:mm:ss"));
    else
        m_dateTipsLabel->setText(currentDateTime.date().toString(Qt::SystemLocaleLongDate)
                         + currentDateTime.toString(" hh:mm:ss A"));

    m_centralWidget->update();
}

void DatetimePlugin::set()
{
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Clock"));
    QFormLayout *layout = new QFormLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->setSizeConstraint(QLayout::SetFixedSize);

    group1 = new QGroupBox(tr("Plugin options"));

    clock = new QCheckBox(tr("Analogue Clock"));
    clock->setChecked(m_settings.value("ShowClock").toBool());
    layout->addRow(clock);

    QLineEdit *lineEdit_format = new QLineEdit;
    lineEdit_format->setPlaceholderText(tr("Default if empty"));
    lineEdit_format->setText(m_settings.value("Format", tr("HH:mm\\nMM/dd ddd")).toString());
    layout->addRow(tr("Format"), lineEdit_format);

    QLabel *label = new QLabel("HH:mm:ss\\nyyyy/MM/dd ddd");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(tr("Such as"), label);

    group1->setLayout(layout);
    vbox->addWidget(group1);

    group2 = new QGroupBox(tr("Calendar options"));

    layout = new QFormLayout;
    seconds = new QCheckBox(tr("Show seconds"));
    seconds->setChecked(m_settings.value("ShowSeconds").toBool());
    layout->addRow(seconds);

    QLineEdit *lineEdit_app = new QLineEdit;
    lineEdit_app->setPlaceholderText(tr("Deepin Calendar if empty"));
    lineEdit_app->setText(m_settings.value("Calendar", "dbus-send --print-reply --dest=com.deepin.Calendar /com/deepin/Calendar com.deepin.Calendar.RaiseWindow").toString());
    layout->addRow(tr("Command"), lineEdit_app);
    label = new QLabel(tr("Launching this application when selecting the menu item \"Open Calendar\""));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    layout->addRow(label);

    hbox = new QHBoxLayout;
    angle180 = new QRadioButton(tr("Northern"));
    angle90 = new QRadioButton(tr("Equator"));
    angle0 = new QRadioButton(tr("Southern"));
    int angle = m_settings.value("Angle", 180).toInt();
    if (angle == 180) {
        angle180->setChecked(true);
        angle90->setChecked(false);
        angle0->setChecked(false);
    }
    if (angle == 90) {
        angle180->setChecked(false);
        angle90->setChecked(true);
        angle0->setChecked(false);
    }
    if (angle == 0) {
        angle180->setChecked(false);
        angle90->setChecked(false);
        angle0->setChecked(true);
    }

    hbox->addWidget(angle180);
    hbox->addWidget(angle90);
    hbox->addWidget(angle0);
    layout->addRow(tr("Location"), hbox);

    group2->setLayout(layout);
    vbox->addWidget(group2);

    group3 = new QGroupBox(tr("Calendar Theme"));

    layout = new QFormLayout;
    form = new QCheckBox(tr("Round selection"));
    form->setChecked(m_settings.value("RoundForm").toBool());
    layout->addRow(form);

    weekend = new QCheckBox(tr("Color weekend"));
    weekend->setChecked(m_settings.value("ColorWeekend", true).toBool());
    layout->addRow(weekend);

    hbox = new QHBoxLayout;
    label = new QLabel(tr("Color"));
    color0 = new QRadioButton();
    color1 = new QRadioButton();
    color2 = new QRadioButton();
    color3 = new QRadioButton();
    color4 = new QRadioButton();
    color5 = new QRadioButton();
    color6 = new QRadioButton();
    color7 = new QRadioButton();
    color8 = new QRadioButton();
    color9 = new QRadioButton();
    color10 = new QRadioButton();
    color11 = new QRadioButton();
    color12 = new QRadioButton();
    color13 = new QRadioButton();
    color14 = new QRadioButton();
    color15 = new QRadioButton();
    color0->setStyleSheet("background-color: rgb(244, 67, 54, 120);");
    color1->setStyleSheet("background-color: rgb(233, 30, 99, 120);");
    color2->setStyleSheet("background-color: rgb(190, 63, 213, 120);");
    color3->setStyleSheet("background-color: rgb(136, 96, 205, 120);");
    color4->setStyleSheet("background-color: rgb(104, 119, 202, 120);");
    color5->setStyleSheet("background-color: rgb(25, 138, 230, 120);");
    color6->setStyleSheet("background-color: rgb(13, 148, 211, 120);");
    color7->setStyleSheet("background-color: rgb(9, 147, 165, 120);");
    color8->setStyleSheet("background-color: rgb(10, 158, 142, 120);");
    color9->setStyleSheet("background-color: rgb(51, 117, 54, 120);");
    color10->setStyleSheet("background-color: rgb(41, 142, 46, 120);");
    color11->setStyleSheet("background-color: rgb(205, 220, 57, 120);");
    color12->setStyleSheet("background-color: rgb(255, 235, 59, 120);");
    color13->setStyleSheet("background-color: rgb(255, 193, 7, 120);");
    color14->setStyleSheet("background-color: rgb(234, 165, 62, 120);");
    color15->setStyleSheet("background-color: rgb(255, 87, 34, 120);");

    colorInt = m_settings.value("SetColor", 5);
    if (colorInt == 0) {
        color0->setChecked(true);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 1) {
        color0->setChecked(false);
        color1->setChecked(true);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 2) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(true);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 3) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(true);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 4) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(true);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 5) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(true);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 6) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(true);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 7) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(true);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 8) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(true);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 9) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(true);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 10) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(true);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 11) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(true);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 12) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(true);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 13) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color2->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(true);
        color14->setChecked(false);
        color15->setChecked(false);
    }
    if (colorInt == 14) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(true);
        color15->setChecked(false);
    }
    if (colorInt == 15) {
        color0->setChecked(false);
        color1->setChecked(false);
        color2->setChecked(false);
        color3->setChecked(false);
        color4->setChecked(false);
        color5->setChecked(false);
        color6->setChecked(false);
        color7->setChecked(false);
        color8->setChecked(false);
        color9->setChecked(false);
        color10->setChecked(false);
        color11->setChecked(false);
        color12->setChecked(false);
        color13->setChecked(false);
        color14->setChecked(false);
        color15->setChecked(true);
    }

    hbox->addStretch();
    hbox->addWidget(color0);
    hbox->addWidget(color1);
    hbox->addWidget(color2);
    hbox->addWidget(color3);
    hbox->addWidget(color4);
    hbox->addWidget(color5);
    hbox->addWidget(color6);
    hbox->addWidget(color7);
    hbox->addStretch();
    layout->addRow(label, hbox);
    hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(color8);
    hbox->addWidget(color9);
    hbox->addWidget(color10);
    hbox->addWidget(color11);
    hbox->addWidget(color12);
    hbox->addWidget(color13);
    hbox->addWidget(color14);
    hbox->addWidget(color15);
    hbox->addStretch();
    layout->addRow("", hbox);

    label = new QLabel(tr("Transparency"));
    slider = new QSlider(Qt::Horizontal);
    slider->setRange(70, 150);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(5);
    slider->setSingleStep(1);
    alfaInt = m_settings.value("SetAlfa", 110).toInt();
    slider->setValue(alfaInt);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(alfaValue()));
    layout->addRow(label, slider);

    group3->setLayout(layout);
    vbox->addWidget(group3);

    hbox = new QHBoxLayout;
    QPushButton *cancelButton = new QPushButton (tr("Cancel"));
    QPushButton *confirmButton = new QPushButton ("OK");
    confirmButton->setDefault(true);
    connect(cancelButton, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect(confirmButton, SIGNAL(clicked()), dialog, SLOT(accept()));
    hbox->addStretch();
    hbox->addWidget(cancelButton);
    hbox->addWidget(confirmButton);
    hbox->setContentsMargins(0, 10, 0, 0);
    vbox->addLayout(hbox);

    dialog->setLayout(vbox);

    if (dialog->exec() == QDialog::Accepted) {
        if (clock->isChecked())
            m_settings.setValue("ShowClock", true);
        else
            m_settings.setValue("ShowClock", false);

        if (seconds->isChecked())
            m_settings.setValue("ShowSeconds", true);
        else
            m_settings.setValue("ShowSeconds", false);

        if (angle180->isChecked())
            m_settings.setValue("Angle", 180);
        if (angle90->isChecked())
            m_settings.setValue("Angle", 90);
        if (angle0->isChecked())
            m_settings.setValue("Angle", 0);

        if (form->isChecked())
            m_settings.setValue("RoundForm", true);
        else
            m_settings.setValue("RoundForm", false);

        if (weekend->isChecked())
            m_settings.setValue("ColorWeekend", true);
        else
            m_settings.setValue("ColorWeekend", false);

        if (color0->isChecked())
            m_settings.setValue("SetColor", 0);
        if (color1->isChecked())
            m_settings.setValue("SetColor", 1);
        if (color2->isChecked())
            m_settings.setValue("SetColor", 2);
        if (color3->isChecked())
            m_settings.setValue("SetColor", 3);
        if (color4->isChecked())
            m_settings.setValue("SetColor", 4);
        if (color5->isChecked())
            m_settings.setValue("SetColor", 5);
        if (color6->isChecked())
            m_settings.setValue("SetColor", 6);
        if (color7->isChecked())
            m_settings.setValue("SetColor", 7);
        if (color8->isChecked())
            m_settings.setValue("SetColor", 8);
        if (color9->isChecked())
            m_settings.setValue("SetColor", 9);
        if (color10->isChecked())
            m_settings.setValue("SetColor", 10);
        if (color11->isChecked())
            m_settings.setValue("SetColor", 11);
        if (color12->isChecked())
            m_settings.setValue("SetColor", 12);
        if (color13->isChecked())
            m_settings.setValue("SetColor", 13);
        if (color14->isChecked())
            m_settings.setValue("SetColor", 14);
        if (color15->isChecked())
            m_settings.setValue("SetColor", 15);

        QString line_format = lineEdit_format->text();
        if (line_format.isEmpty() == true) // empty line protection
            line_format = QString(tr("HH:mm\\nMM/dd ddd"));
        m_settings.setValue("Format", line_format);
        QString line_app = lineEdit_app->text();
        if (line_app.isEmpty() == true) // empty line protection
            line_app = QString("dbus-send --print-reply --dest=com.deepin.Calendar /com/deepin/Calendar com.deepin.Calendar.RaiseWindow");
        m_settings.setValue("Calendar", line_app);
        m_settings.setValue("SetAlfa", alfaInt);
        m_centralWidget->update();
        m_calendarWidget->updateDateStyle();
        m_calendarWidget->datewidget->moonPhase();
    }
    dialog->close();
}

void DatetimePlugin::openCalendar()
{
    QProcess::startDetached(m_settings.value("Calendar", "dbus-send --print-reply --dest=com.deepin.Calendar /com/deepin/Calendar com.deepin.Calendar.RaiseWindow").toString());
}

void DatetimePlugin::alfaValue()
{
    alfaInt = slider->value();
}