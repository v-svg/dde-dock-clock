#include "calendarwidget.h"
#include "calendar.h"
#include "datetimeplugin.h"
#include "datewidget.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QApplication>
#include <QTextCharFormat>
#include <QStackedLayout>
#include <QGraphicsOpacityEffect>
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
    setFixedHeight(468);

    QVBoxLayout *vbox = new QVBoxLayout;
    QStackedLayout *slayout = new QStackedLayout;

    currentTime = new QLabel;
    currentTime->setStyleSheet("QLabel {font-size: 48px;}");
    currentTime->setFixedHeight(50);
    currentTime->setAlignment(Qt::AlignCenter);

    currentDate = new QPushButton;
    currentDate->setFixedHeight(22);

    vbox->addWidget(currentTime, 0, Qt::AlignCenter);
    vbox->addWidget(currentDate, 1, Qt::AlignHCenter | Qt::AlignTop);
    vbox->addSpacing(22);

    calendar = new Calendar;
    datewidget = new DateWidget;
    datewidget->setVisible(false);

    slayout->setStackingMode(QStackedLayout::StackAll);
    slayout->addWidget(calendar);
    slayout->addWidget(datewidget);

    vbox->addLayout(slayout);
    setLayout(vbox);

    updateTime();
    updateDateStyle();
    selDate();

    connect(refreshDateTimer, &QTimer::timeout,
            this, &CalendarWidget::updateTime);
    connect(currentDate, &QPushButton::clicked,
            this, &CalendarWidget::jumpToToday);
    connect(calendar, &Calendar::activated,
            this, &CalendarWidget::showDay);
    connect(datewidget, &DateWidget::hideDate,
            this, &CalendarWidget::showCal);
    connect(datewidget->closeButton, &QPushButton::clicked,
            this, &CalendarWidget::showCal);
    connect(datewidget->prevButton, &QPushButton::clicked,
            this, &CalendarWidget::prevDay);
    connect(datewidget->nextButton, &QPushButton::clicked,
            this, &CalendarWidget::nextDay);
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
    painter.drawRect(0, 105, 319, 1);
}

void CalendarWidget::jumpToToday()
{
    calendar->setSelectedDate(QDate::currentDate());

    if (calendar->isVisible() == false) {
        selDate();
        datewidget->update();
    }
}

void CalendarWidget::updateTime()
{
    const QDateTime dateTime = QDateTime::currentDateTime();
    QString date = dateTime.date().toString(Qt::SystemLocaleLongDate);
    QFont font = qApp->font();
    if (font.pointSize()  > 13)
        font.setPointSize(13);
    setFont(font);
    calendar->setFont(font);
    if (m_settings.value("ShowSeconds").toBool() == false)
        currentTime->setText(dateTime.toString("HH:mm"));
    else
        currentTime->setText(dateTime.toString("HH:mm:ss"));
    currentDate->setText(date);
}

void CalendarWidget::updateDateStyle()
{
    int intColor = m_settings.value("SetColor", 5).toInt();
    QList<QColor> colorList = {QColor(244, 67, 54), QColor(233, 30, 99),
         QColor(190, 63, 213), QColor(136, 96, 205), QColor(104, 119, 202),
         QColor(25, 138, 230), QColor(13, 148, 211), QColor(9, 147, 165),
         QColor(23, 140, 129), QColor(41, 142, 46), QColor(139, 195, 74),
         QColor(205, 220, 57), QColor(255, 235, 59), QColor(255, 193, 7),
         QColor(234, 165, 62), QColor(255, 87, 34)};
    QColor themeColor = colorList[intColor];
    QString textColor = themeColor.name(QColor::HexRgb);

    QString  styleText = QString(
        "QToolButton {border: none; font-size: 16px; font-weight: 600;}"
        "QWidget:hover {color: %1;}"
        "QWidget:pressed {color: lightGrey;}").arg(textColor);

    QString styleSheet = QString(
        "QPushButton {border: none; font-size: 16px; font-weight: 600;}"
        "QPushButton:!hover {color: white;}"
        "QPushButton:hover {color: %1;}"
        "QPushButton:pressed {color: lightGrey;}").arg(textColor);

    datewidget->prevButton->setStyleSheet(styleSheet);
    datewidget->nextButton->setStyleSheet(styleSheet);
    datewidget->closeButton->setStyleSheet(styleSheet.replace("16", "14"));
    datewidget->dateColor = themeColor;

    calendar->cellColor = themeColor;
    calendar->textStyle = styleText;
    calendar->updateButtonStyle();

    currentDate->setStyleSheet(styleSheet.replace(" font-size: 14px; font-weight: 600;", ""));
    currentDate->setCursor(QCursor(Qt::PointingHandCursor));
    
    QTextCharFormat format;
    QColor weekendColor;
    if (m_settings.value("ColorWeekend", true).toBool() == true)
        weekendColor = colorList[intColor];
    else
        weekendColor = QColor(Qt::white);
    format.setForeground(QBrush(weekendColor, Qt::SolidPattern));
    calendar->setWeekdayTextFormat(Qt::Saturday, format);
    calendar->setWeekdayTextFormat(Qt::Sunday, format);
}

void CalendarWidget::showDay()
{
    selDate();
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    calendar->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    connect(animation, &QPropertyAnimation::finished,
            calendar, &CalendarWidget::hide);

    effect = new QGraphicsOpacityEffect();
    datewidget->setGraphicsEffect(effect);
    animation = new QPropertyAnimation(effect, "opacity");
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    datewidget->setVisible(true);
}

void CalendarWidget::showCal()
{
    selectDate = datewidget->sDate;
    calendar->setSelectedDate(selectDate);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    calendar->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    calendar->setVisible(true);

    effect = new QGraphicsOpacityEffect();
    datewidget->setGraphicsEffect(effect);
    animation = new QPropertyAnimation(effect, "opacity");
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    connect(animation, &QPropertyAnimation::finished,
            datewidget, &DateWidget::hide);
}

void CalendarWidget::prevDay()
{
    selectDate = datewidget->sDate;
    selectDate = selectDate.addDays(-1);
    setDate();
}

void CalendarWidget::nextDay()
{
    selectDate = datewidget->sDate;
    selectDate = selectDate.addDays(1);
    setDate();
}

void CalendarWidget::selDate()
{
    selectDate = calendar->selectedDate();
    setDate();
}

void CalendarWidget::setDate()
{
    datewidget->sDate = selectDate;
    if (selectDate == QDate::currentDate())
        datewidget->today = true;
    else
        datewidget->today = false;
    datewidget->getZodiac();
    datewidget->moonPhase();
    datewidget->weekLabel->setText(selectDate.toString("dddd"));
    datewidget->dateLabel->setText(selectDate.toString("d"));
    datewidget->monthLabel->setText(selectDate.toString("MMMM yyyy"));
    datewidget->update();
    calendar->setSelectedDate(selectDate);
}

