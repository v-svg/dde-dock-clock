#include "calendarwidget.h"
#include "calendar.h"
#include "datetimeplugin.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QTextCharFormat>
#include <QDateTime>
#include <QPainter>
#include <QCursor>


CalendarWidget::CalendarWidget(QWidget *parent) : QWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    QTimer *refreshDateTimer = new QTimer;
    refreshDateTimer->setInterval(1000);
    refreshDateTimer->start();

    setFixedWidth(319);
    setFixedHeight(464);

    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    QVBoxLayout *layout = new QVBoxLayout;

    currentTimeLabel = new QLabel;
    currentTimeLabel->setStyleSheet("QLabel {font-size: 48px;}");
    currentTimeLabel->setFixedHeight(44);
    currentTimeLabel->setAlignment(Qt::AlignCenter);

    currentDateLabel = new QPushButton;
    currentDateLabel->setFixedHeight(22);

    layout->addWidget(currentTimeLabel);
    layout->setMargin(0);

    hbox->addStretch();
    hbox->addLayout(layout);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addWidget(currentDateLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    vbox->addSpacing(24);

    hbox = new QHBoxLayout;
    calendar = new Calendar;
    hbox->addWidget(calendar);

    vbox->addLayout(hbox);
    setLayout(vbox);

    updateTime();
    updateDateStyle();

    connect(refreshDateTimer, &QTimer::timeout, this, &CalendarWidget::updateTime);
    connect(currentDateLabel, &QPushButton::clicked, this, &CalendarWidget::jumpToToday);
}

CalendarWidget::~CalendarWidget()
{
}

void CalendarWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 0.12 * 255));
    painter.drawRect(0, 97, 319, 1);
}

void CalendarWidget::jumpToToday()
{
    calendar->setSelectedDate(QDate::currentDate());
}

void CalendarWidget::updateTime()
{
    const QDateTime dateTime = QDateTime::currentDateTime();
    QString currentDate = dateTime.date().toString(Qt::SystemLocaleLongDate);
    QFont font = qApp->font();
    if (font.pointSize()  > 13)
        font.setPointSize(13);
    setFont(font);
    calendar->setFont(font);
    if (m_settings.value("ShowSeconds").toBool() == false)
        currentTimeLabel->setText(dateTime.toString("HH:mm"));
    else
        currentTimeLabel->setText(dateTime.toString("HH:mm:ss"));
    currentDateLabel->setText(currentDate);

}

void CalendarWidget::updateDateStyle()
{
    int intColor = m_settings.value("SetColor", 5).toInt();
    QList<QColor> colorList = {
    QColor(244, 67, 54), QColor(233, 30, 99), QColor(190, 63, 213), QColor(136, 96, 205),
    QColor(104, 119, 202), QColor(25, 138, 230), QColor(13, 148, 211), QColor(9, 147, 165),
    QColor(23, 140, 129), QColor(41, 142, 46), QColor(139, 195, 74), QColor(205, 220, 57),
    QColor(255, 235, 59), QColor(255, 193, 7), QColor(234, 165, 62), QColor(255, 87, 34)};
    QColor themeColor = colorList[intColor];
    QString brushColor = themeColor.name(QColor::HexRgb);
    QString styleSheet = QString(
        "QPushButton {border: none;}"
        "QPushButton:!hover {color: white;}"
        "QPushButton:hover {color: %1;}"
        "QPushButton:pressed {color: lightGrey;}").arg(brushColor);
    currentDateLabel->setStyleSheet(styleSheet);
    currentDateLabel->setCursor(QCursor(Qt::PointingHandCursor));
    QString  styleText = QString(
        "QToolButton {border: none;}"
        "QWidget:hover {color: %1;}"
        "QWidget:pressed {color: lightGrey;}").arg(brushColor);
    calendar->textStyle = styleText;
    calendar->cellColor = themeColor;
    calendar->updateButtonStyle();
    QTextCharFormat format = calendar->weekdayTextFormat(Qt::Saturday);
    QColor weekendColor;
    if (m_settings.value("ColorWeekend", true).toBool() == true)
        weekendColor = colorList[intColor];
    else
        weekendColor = QColor(Qt::white);
    format.setForeground(QBrush(weekendColor, Qt::SolidPattern));
    calendar->setWeekdayTextFormat(Qt::Saturday, format);
    calendar->setWeekdayTextFormat(Qt::Sunday, format);
}
