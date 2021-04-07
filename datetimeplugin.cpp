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
#include <DApplication>
#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QButtonGroup>
#include <QSlider>
#include <QDialogButtonBox>

DWIDGET_USE_NAMESPACE

DatetimePlugin::DatetimePlugin(QObject *parent)
    : QObject(parent),
      m_dateTipsLabel(new QLabel),
      m_refershTimer(new QTimer(this)),
      m_settings("deepin", "dde-dock-clock")
{
    QString applicationName = qApp->applicationName();
    qApp->setApplicationName("dde-dock-clock");
    qDebug() << qApp->loadTranslator();
    qApp->setApplicationName(applicationName);

    m_dateTipsLabel->setObjectName("clock");
    m_dateTipsLabel->setStyleSheet("padding:0px 3px;");

    m_refershTimer->setInterval(1000);
    m_refershTimer->start();

    m_centralWidget = new DatetimeWidget;
    m_calendarWidget = new CalendarWidget;

    connect(m_centralWidget, &DatetimeWidget::requestUpdateGeometry, [this] {
        m_proxyInter->itemUpdate(this, pluginName());
    });
    connect(m_centralWidget, &DatetimeWidget::mouseMidBtnClicked, this, &DatetimePlugin::openCalendar);
    connect(m_refershTimer, &QTimer::timeout, this, &DatetimePlugin::updateCurrentTimeString);
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

    return m_calendarWidget;
}

const QString DatetimePlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();

    QList<QVariant> items;
    items.reserve(5);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = tr("Open Calendar");
    open["isActive"] = true;
    items.push_back(open);

    if (displayMode == Dock::Fashion && !m_settings.value("ShowClock").toBool()) {
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
    dialog->setWindowIcon(QIcon::fromTheme("gnome-schedule"));

    QFormLayout *layout = new QFormLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSizeConstraint(QLayout::SetFixedSize);

    QGroupBox *group = new QGroupBox(tr("Plugin options"));

    QCheckBox *clock = new QCheckBox(tr("Analogue Clock"));
    clock->setChecked(m_settings.value("ShowClock").toBool());
    layout->addRow(clock);

    QLineEdit *lineEditFormat = new QLineEdit;
    lineEditFormat->setPlaceholderText(tr("Default if empty"));
    lineEditFormat->setText(m_settings.value("Format", tr("HH:mm\\nMM/dd ddd")).toString());
    layout->addRow(tr("Format"), lineEditFormat);

    QLabel *label = new QLabel("HH:mm:ss\\nyyyy/MM/dd ddd");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(tr("Such as"), label);

    group->setLayout(layout);
    vbox->addWidget(group);

    group = new QGroupBox(tr("Calendar options"));

    layout = new QFormLayout;
    QCheckBox *seconds = new QCheckBox(tr("Show seconds"));
    seconds->setChecked(m_settings.value("ShowSeconds").toBool());
    layout->addRow(seconds);

    QLineEdit *lineEditApp = new QLineEdit;
    lineEditApp->setPlaceholderText(tr("Deepin Calendar if empty"));
    lineEditApp->setText(m_settings.value("Calendar", "dde-calendar").toString());
    layout->addRow(tr("Command"), lineEditApp);
    label = new QLabel(tr("Launching this application when selecting the menu item \"Open Calendar\""));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    layout->addRow(label);

    QRadioButton *angle180 = new QRadioButton(tr("Northern"));
    QRadioButton *angle90 = new QRadioButton(tr("Equator"));
    QRadioButton *angle0 = new QRadioButton(tr("Southern"));
    int angle = m_settings.value("Angle", 180).toInt();
    if (angle == 180) {
        angle180->setChecked(true);
        angle90->setChecked(false);
        angle0->setChecked(false);
    } if (angle == 90) {
        angle180->setChecked(false);
        angle90->setChecked(true);
        angle0->setChecked(false);
    } if (angle == 0) {
        angle180->setChecked(false);
        angle90->setChecked(false);
        angle0->setChecked(true);
    }

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(angle180);
    hbox->addWidget(angle90);
    hbox->addWidget(angle0);
    layout->addRow(tr("Location"), hbox);

    group->setLayout(layout);
    vbox->addWidget(group);

    group = new QGroupBox(tr("Calendar theme"));

    layout = new QFormLayout;
    QCheckBox *roundForm = new QCheckBox(tr("Round selection"));
    roundForm->setChecked(m_settings.value("RoundForm", true).toBool());
    layout->addRow(roundForm);

    QCheckBox *weekend = new QCheckBox(tr("Color weekend"));
    weekend->setChecked(m_settings.value("ColorWeekend", true).toBool());
    layout->addRow(weekend);

    colorButton1 = new QRadioButton;
    colorButton2 = new QRadioButton;
    colorButton3 = new QRadioButton;
    colorButton4 = new QRadioButton;
    colorButton5 = new QRadioButton;
    colorButton6 = new QRadioButton;
    colorButton7 = new QRadioButton;
    colorButton8 = new QRadioButton;
    colorButton9 = new QRadioButton;
    colorButton10 = new QRadioButton;
    colorButton11 = new QRadioButton;
    colorButton12 = new QRadioButton;
    colorButton13 = new QRadioButton;
    colorButton14 = new QRadioButton;
    colorButton15 = new QRadioButton;
    colorButton16 = new QRadioButton;

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(colorButton1, 1);
    buttonGroup->addButton(colorButton2, 2);
    buttonGroup->addButton(colorButton3, 3);
    buttonGroup->addButton(colorButton4, 4);
    buttonGroup->addButton(colorButton5, 5);
    buttonGroup->addButton(colorButton6, 6);
    buttonGroup->addButton(colorButton7, 7);
    buttonGroup->addButton(colorButton8, 8);
    buttonGroup->addButton(colorButton9, 9);
    buttonGroup->addButton(colorButton10, 10);
    buttonGroup->addButton(colorButton11, 11);
    buttonGroup->addButton(colorButton12, 12);
    buttonGroup->addButton(colorButton13, 13);
    buttonGroup->addButton(colorButton14, 14);
    buttonGroup->addButton(colorButton15, 15);
    buttonGroup->addButton(colorButton16, 16);

    int colorInt = m_settings.value("SetColor", 7).toInt();
    foreach (QAbstractButton* button, buttonGroup->buttons()) {
        if (buttonGroup->id(button) == colorInt) {
            button->setChecked(true);
            break;
        }
    }

    hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->addWidget(colorButton1);
    hbox->addWidget(colorButton2);
    hbox->addWidget(colorButton3);
    hbox->addWidget(colorButton4);
    hbox->addWidget(colorButton5);
    hbox->addWidget(colorButton6);
    hbox->addWidget(colorButton7);
    hbox->addWidget(colorButton8);
    layout->addRow(tr("Accent color"), hbox);

    hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->addWidget(colorButton9);
    hbox->addWidget(colorButton10);
    hbox->addWidget(colorButton11);
    hbox->addWidget(colorButton12);
    hbox->addWidget(colorButton13);
    hbox->addWidget(colorButton14);
    hbox->addWidget(colorButton15);
    hbox->addWidget(colorButton16);
    layout->addRow("", hbox);

    colorAlfa = m_settings.value("SetAlfa", 129).toInt();
    label = new QLabel(tr("Transparency") + " " + QString::number(colorAlfa * 100 / 255) + "%");

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(79, 154);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(5);
    slider->setSingleStep(5);
    slider->setPageStep(5);
    slider->setValue(colorAlfa);
    layout->addRow(label, slider);

    group->setLayout(layout);
    vbox->addWidget(group);

    btnRadius = roundForm->isChecked() ? 15 : 3;
    setButtonStyle();

    connect(roundForm, &QCheckBox::toggled, this, [=] {
        btnRadius = roundForm->isChecked() ? 15 : 3;
        setButtonStyle();
    });

    connect(slider, &QSlider::valueChanged, this, [=] {
        colorAlfa = slider->value();
        label->setText(tr("Transparency") + " " + QString::number(colorAlfa * 100 / 255) + "%");
        setButtonStyle();
    });

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel |
                                                     QDialogButtonBox::Ok, dialog);
    buttons->setStyleSheet("button-layout: 3;");
    buttons->setContentsMargins(0, 5, 0, 0);
    vbox->addWidget(buttons, 1, Qt::AlignCenter);

    dialog->setLayout(vbox);

    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);

    if (dialog->exec() == QDialog::Accepted) {
        QString timeFormat = lineEditFormat->text();
        if (timeFormat.isEmpty())
            timeFormat = QString(tr("HH:mm\\nMM/dd ddd"));

        QString calendarApp = lineEditApp->text();
        if (calendarApp.isEmpty())
            calendarApp = QString("dde-calendar");

        if (angle180->isChecked())
            m_settings.setValue("Angle", 180);
        if (angle90->isChecked())
            m_settings.setValue("Angle", 90);
        if (angle0->isChecked())
            m_settings.setValue("Angle", 0);

        m_settings.setValue("ShowClock", clock->isChecked());
        m_settings.setValue("Format", timeFormat);
        m_settings.setValue("ShowSeconds", seconds->isChecked());
        m_settings.setValue("Calendar", calendarApp);
        m_settings.setValue("RoundForm", roundForm->isChecked());
        m_settings.setValue("ColorWeekend", weekend->isChecked());        
        m_settings.setValue("SetColor", buttonGroup->checkedId());
        m_settings.setValue("SetAlfa", colorAlfa);

        m_centralWidget->requestUpdateGeometry();

        m_calendarWidget->updateStyle();

        dialog->close();
    }
}

void DatetimePlugin::setButtonStyle()
{
    QString style = "QRadioButton::indicator::unchecked {"
                    "border: 3px solid transparent;"
                    "border-radius: %1px;"
                    "width: 24px;"
                    "height: 24px;"
                    "background-color: rgb(%2, %3);"
                    "}"
                    "QRadioButton::indicator::checked {"
                    "border: 3px solid rgb(%2);"
                    "border-radius: %1px;"
                    "width: 24px;"
                    "height: 24px;"
                    "background-color: rgb(%2, %3);"
                    "image: url(:/icons/resources/icons/check/check%4.svg);"
                    "}"
                    "QRadioButton::indicator::unchecked:hover {"
                    "border: 3px solid rgb(%2);"
                    "border-radius: %1px;"
                    "width: 24px;"
                    "height: 24px;"
                    "background-color: rgb(%2, %3);"
                    "}";
    QString colorRgb = "239, 83, 80";
    QString iconName = "1";
    colorButton1->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "236, 64, 122";
    iconName = "2";
    colorButton2->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "171, 71, 188";
    iconName = "3";
    colorButton3->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "140, 75, 255";
    iconName = "4";
    colorButton4->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "92, 107, 192";
    iconName = "5";
    colorButton5->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "25, 118, 210";
    iconName = "6";
    colorButton6->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "44, 167, 248";
    iconName = "7";
    colorButton7->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "0, 188, 212";
    iconName = "8";
    colorButton8->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "77, 182, 172";
    iconName = "9";
    colorButton9->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "102, 187, 106";
    iconName = "10";
    colorButton10->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "124, 179, 66";
    iconName = "11";
    colorButton11->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "185, 204, 0";
    iconName = "12";
    colorButton12->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "243, 218, 0";
    iconName = "13";
    colorButton13->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "255, 191, 0";
    iconName = "14";
    colorButton14->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "255, 152, 0";
    iconName = "15";
    colorButton15->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
    colorRgb = "255, 112, 67";
    iconName = "16";
    colorButton16->setStyleSheet(style.arg(btnRadius).arg(colorRgb).arg(colorAlfa).arg(iconName));
}

void DatetimePlugin::openCalendar()
{
    QString process = m_settings.value("Calendar", "dde-calendar").toString();
    QProcess::startDetached(process);
}
