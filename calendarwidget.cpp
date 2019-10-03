#include "calendarwidget.h"
#include "calendar.h"
#include "datetimeplugin.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDateTime>
#include <QPainter>
#include <QCursor>


CalendarWidget::CalendarWidget(QWidget *parent) : QWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    QTimer *refreshDateTimer = new QTimer;
    refreshDateTimer->setInterval(1000);
    refreshDateTimer->start();

    calendar = new Calendar;

    setFixedWidth(298);
    setFixedHeight(440);

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

    hbox->addStretch();
    hbox->addLayout(layout);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addWidget(currentDateLabel, 0, Qt::AlignHCenter);
    vbox->addSpacing(24);

    hbox = new QHBoxLayout;
    hbox->addWidget(calendar);

    vbox->addLayout(hbox);
    setLayout(vbox);

    updateTime();
    updateDateStyle();

    connect(refreshDateTimer, &QTimer::timeout, this, &CalendarWidget::updateTime);
    connect(currentDateLabel, SIGNAL(pressed()), this, SLOT(jumpToToday()));
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
    painter.drawRect(0, 97, 298, 1);
}

void CalendarWidget::jumpToToday()
{
    calendar->setSelectedDate(QDate::currentDate());
}

void CalendarWidget::updateTime()
{
    const QDateTime dateTime = QDateTime::currentDateTime();
    QString currentDate = dateTime.date().toString(Qt::SystemLocaleLongDate);
    QFont fontLabel = currentDateLabel->font();
    QFont fontApp = qApp->font();
    QFontMetrics FM(fontLabel);
    int widthFM = FM.width(currentDate);
    if (widthFM  > 280)
        fontLabel.setPointSize(13);
    else {
        if (fontApp.pointSize() < 13)
            fontLabel.setPointSize(fontApp.pointSize());
        else
            fontLabel.setPointSize(13);
    }
    currentDateLabel->setFont(fontLabel);
    if (m_settings.value("ShowSeconds").toBool() == false)
        currentTimeLabel->setText(dateTime.toString("HH:mm"));
    else
        currentTimeLabel->setText(dateTime.toString("HH:mm:ss"));
    currentDateLabel->setText(currentDate);

}

void CalendarWidget::updateDateStyle()
{
    int intColor = m_settings.value("SetColor", 5).toInt();
    QStringList brushColor = {
        "244, 67, 54", "233, 30, 99", "156, 39, 176", "103, 58, 183",
        "63, 81, 181", "25, 138, 230", "13, 148, 211", "9, 147, 165",
        "10, 158, 142", "51, 117, 54", "139, 195, 74", "205, 220, 57",
        "255, 235, 59", "255, 193, 7", "234, 165, 62", "255, 87, 34"};
    QString styleSheet = QString(
        "QPushButton {border: none;}"
        "QPushButton:!hover {color: white;}"
        "QPushButton:hover {color: rgb(%1);}"
        "QPushButton:pressed {color: lightGrey;}").arg(brushColor[intColor]);
    currentDateLabel->setStyleSheet(styleSheet);
    currentDateLabel->setCursor(QCursor(Qt::PointingHandCursor));
    QString  styleText = QString(
        "QToolButton {border: none;}"
        "QWidget:hover {color: rgb(%1);}"
        "QWidget:pressed {color: lightGrey;}").arg(brushColor[intColor]);
    calendar->textStyle = styleText;
    calendar->updateButtonStyle();
}
