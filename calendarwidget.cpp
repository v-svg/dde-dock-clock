#include "calendarwidget.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QTextCharFormat>
#include <QButtonGroup>
#include <QStackedLayout>
#include <QGraphicsOpacityEffect>
#include <QDateTime>
#include <QPainter>

CalendarWidget::CalendarWidget(QWidget *parent) : QWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    QTimer *refreshDateTimer = new QTimer;
    refreshDateTimer->setInterval(1000);
    refreshDateTimer->start();

    setFixedWidth(319);
    setFixedHeight(468);

    QVBoxLayout *vbox = new QVBoxLayout;
    QStackedLayout *stack = new QStackedLayout;

    timeLabel = new QLabel;

    QFont font = timeLabel->font();
    font.setPixelSize(46);
    font.setWeight(QFont::Light);

    timeLabel->setFont(font);
    timeLabel->setFixedHeight(48);

    todayButton = new QPushButton;
    todayButton->setFixedHeight(26);

    vbox->addWidget(timeLabel, 0, Qt::AlignCenter);
    vbox->addWidget(todayButton, 1, Qt::AlignCenter | Qt::AlignTop);
    vbox->addSpacing(18);

    m_calendar = new Calendar;

    m_date = new DateWidget;
    m_date->setVisible(false);

    setMonthWidget();
    setYearsWidget();

    stack->setStackingMode(QStackedLayout::StackAll);
    stack->addWidget(m_calendar);
    stack->addWidget(m_date);
    stack->addWidget(m_months);
    stack->addWidget(m_years);

    vbox->addLayout(stack);
    setLayout(vbox);

    updateTime();
    updateStyle();
    selectDate();

    connect(refreshDateTimer, &QTimer::timeout, this, &CalendarWidget::updateTime);
    connect(m_date, &DateWidget::setPrevDay, this, &CalendarWidget::prevDay);
    connect(m_date, &DateWidget::setNextDay, this, &CalendarWidget::nextDay);
    connect(m_date, &DateWidget::hideDateInfo, [=] {
        m_calendar->setSelectedDate(m_date->sDate);
        setAnimation(m_date, m_calendar);
    });
    connect(m_calendar, &Calendar::paletteChange, this, &CalendarWidget::updateStyle);
    connect(m_calendar, &Calendar::activated, [=] {
        selectDate();
        setAnimation(m_calendar, m_date);
    });
    connect(m_calendar, &Calendar::monthButtonClicked, [=] {
        setAnimation(m_calendar, m_months);
    });
    connect(todayButton, &QPushButton::clicked, [=] {
        jumpToToday();
        if (m_months->isVisible())
           setAnimation(m_months, m_calendar);
        if (m_years->isVisible())
           setAnimation(m_years, m_calendar);
    });
}

CalendarWidget::~CalendarWidget()
{
}

void CalendarWidget::wheelEvent(QWheelEvent *e)
{
    if (m_years->isVisible() && m_years->underMouse()) {
        if (e->delta() > 0)
            emit prevYears();
        else
            emit nextYears();
    }
}

void CalendarWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::XButton1) {
        if (m_months->isVisible())
           setAnimation(m_months, m_calendar);
        if (m_years->isVisible())
           setAnimation(m_years, m_calendar);
    }
}

void CalendarWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QColor color = QColor(QApplication::palette().text().color());
    color.setAlpha(26);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRect(QRect(0, 105, 319, 1));
}

void CalendarWidget::jumpToToday()
{
    m_calendar->setSelectedDate(QDate::currentDate());

    if (!m_calendar->isVisible()) {
        selectDate();

        m_date->update();
    }
}

void CalendarWidget::updateTime()
{
    const QDateTime dateTime = QDateTime::currentDateTime();
    QString date = dateTime.date().toString(Qt::SystemLocaleLongDate);

    if (m_settings.value("ShowSeconds").toBool())
        timeLabel->setText(dateTime.toString("HH:mm:ss"));
    else
        timeLabel->setText(dateTime.toString("HH:mm"));
    todayButton->setText(date);
}

void CalendarWidget::updateStyle()
{
    int intColor = m_settings.value("SetColor", 7).toInt();
    QList<QColor> colorList = {QColor(239, 83, 80), QColor(236, 64, 122),
         QColor(171, 71, 188), QColor(140, 75, 255), QColor(92, 107, 192),
         QColor(25, 118, 210), QColor(44, 167, 248), QColor(0, 188, 212),
         QColor(77, 182, 172), QColor(102, 187, 106), QColor(124, 179, 66),
         QColor(220, 231, 117), QColor(255, 235, 59), QColor(255, 202, 40),
         QColor(255, 152, 0), QColor(255, 112, 67)};
    QColor themeColor = colorList[intColor - 1];
    QColor normalColor = QApplication::palette().color(QPalette::Text);
    QColor pressedColor = QApplication::palette().color(QPalette::Disabled, QPalette::Text);

    QString style = QString("QWidget {border: none; color: %1; font-size: 16px; font-weight: 600;}"
                            "QWidget:hover {color: %2;}"
                            "QWidget:pressed {color: %3;}"
                           ).arg(normalColor.name(QColor::HexRgb)).arg(themeColor.name(QColor::HexRgb)
                           ).arg(pressedColor.name(QColor::HexArgb));
    btnStyle = style;

    m_calendar->cellColor = themeColor;
    m_calendar->setButtonStyle(style);

    m_date->dateColor = themeColor;
    m_date->setButtonStyle(style);
    m_date->setMoonPhase();

    todayButton->setFont(m_calendar->font());
    todayButton->setStyleSheet(style.replace(" font-size: 16px; font-weight: 600;", ""));
    todayButton->setCursor(Qt::PointingHandCursor);
    
    QTextCharFormat format;
    QColor weekendColor;
    if (m_settings.value("ColorWeekend", true).toBool())
        weekendColor = themeColor;
    else
        weekendColor = normalColor;
    format.setForeground(QBrush(weekendColor, Qt::SolidPattern));
    m_calendar->setWeekdayTextFormat(Qt::Saturday, format);
    m_calendar->setWeekdayTextFormat(Qt::Sunday, format);

    emit styleChanged(style);
}

void CalendarWidget::setAnimation(QWidget *visibleWidget, QWidget *hiddenWidget)
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    visibleWidget->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    connect(animation, &QPropertyAnimation::finished, [=] {
        visibleWidget->setVisible(false);
    });

    effect = new QGraphicsOpacityEffect();
    hiddenWidget->setGraphicsEffect(effect);
    animation = new QPropertyAnimation(effect, "opacity");
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    hiddenWidget->setVisible(true);
}

void CalendarWidget::prevDay()
{
    QDate date = m_date->sDate;
    date = date.addDays(-1);
    setDate(date);
}

void CalendarWidget::nextDay()
{
    QDate date = m_date->sDate;
    date = date.addDays(1);
    setDate(date);
}

void CalendarWidget::selectDate()
{
    setDate(m_calendar->selectedDate());
}

void CalendarWidget::setDate(const QDate &date)
{
    m_date->sDate = date;
    if (date == QDate::currentDate())
        m_date->today = true;
    else
        m_date->today = false;
    m_date->setZodiacSign();
    m_date->setMoonPhase();
    m_date->weekLabel->setText(date.toString("dddd"));
    m_date->dateLabel->setText(date.toString("d"));
    m_date->monthLabel->setText(date.toString("MMMM yyyy"));
    m_date->update();

    m_calendar->setSelectedDate(date);
}

void CalendarWidget::setMonthWidget()
{
    m_months = new QWidget;
    m_months->setVisible(false);

    QPushButton *monthButton1 = new QPushButton;
    QPushButton *monthButton2 = new QPushButton;
    QPushButton *monthButton3 = new QPushButton;
    QPushButton *monthButton4 = new QPushButton;
    QPushButton *monthButton5 = new QPushButton;
    QPushButton *monthButton6 = new QPushButton;
    QPushButton *monthButton7 = new QPushButton;
    QPushButton *monthButton8 = new QPushButton;
    QPushButton *monthButton9 = new QPushButton;
    QPushButton *monthButton10 = new QPushButton;
    QPushButton *monthButton11 = new QPushButton;
    QPushButton *monthButton12 = new QPushButton;

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(monthButton1, 1);
    buttonGroup->addButton(monthButton2, 2);
    buttonGroup->addButton(monthButton3, 3);
    buttonGroup->addButton(monthButton4, 4);
    buttonGroup->addButton(monthButton5, 5);
    buttonGroup->addButton(monthButton6, 6);
    buttonGroup->addButton(monthButton7, 7);
    buttonGroup->addButton(monthButton8, 8);
    buttonGroup->addButton(monthButton9, 9);
    buttonGroup->addButton(monthButton10, 10);
    buttonGroup->addButton(monthButton11, 11);
    buttonGroup->addButton(monthButton12, 12);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    grid->setSpacing(0);
    grid->addWidget(monthButton1, 1, 1, Qt::AlignCenter);
    grid->addWidget(monthButton2, 1, 2, Qt::AlignCenter);
    grid->addWidget(monthButton3, 2, 1, Qt::AlignCenter);
    grid->addWidget(monthButton4, 2, 2, Qt::AlignCenter);
    grid->addWidget(monthButton5, 3, 1, Qt::AlignCenter);
    grid->addWidget(monthButton6, 3, 2, Qt::AlignCenter);
    grid->addWidget(monthButton7, 4, 1, Qt::AlignCenter);
    grid->addWidget(monthButton8, 4, 2, Qt::AlignCenter);
    grid->addWidget(monthButton9, 5, 1, Qt::AlignCenter);
    grid->addWidget(monthButton10, 5, 2, Qt::AlignCenter);
    grid->addWidget(monthButton11, 6, 1, Qt::AlignCenter);
    grid->addWidget(monthButton12, 6, 2, Qt::AlignCenter);

    m_months->setLayout(grid);

    foreach (QAbstractButton* button, buttonGroup->buttons()) {
        QString text = locale().standaloneMonthName(buttonGroup->id(button));
        button->setText(text.at(0).toUpper() + text.mid(1));
        button->setFixedSize(130, 37);
        button->setCursor(Qt::PointingHandCursor);
    }

    connect(buttonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [=](int id) {
        QDate currentDate = m_calendar->selectedDate();
        QDate newDate = currentDate.addMonths(id - currentDate.month());
        m_calendar->setSelectedDate(newDate);
        setAnimation(m_months, m_calendar);
    });
    connect(this, &CalendarWidget::styleChanged, [=](QString style) {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            button->setStyleSheet(style);
        }
    });
}

void CalendarWidget::setYearsWidget()
{
    m_years = new QWidget;
    m_years->setVisible(false);

    QPushButton *yearButton1 = new QPushButton;
    QPushButton *yearButton2 = new QPushButton;
    QPushButton *yearButton3 = new QPushButton;
    QPushButton *yearButton4 = new QPushButton;
    QPushButton *yearButton5 = new QPushButton;
    QPushButton *yearButton6 = new QPushButton;
    QPushButton *yearButton7 = new QPushButton;
    QPushButton *yearButton8 = new QPushButton;
    QPushButton *yearButton9 = new QPushButton;
    QPushButton *yearButton10 = new QPushButton;

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(yearButton1, 1);
    buttonGroup->addButton(yearButton2, 2);
    buttonGroup->addButton(yearButton3, 3);
    buttonGroup->addButton(yearButton4, 4);
    buttonGroup->addButton(yearButton5, 5);
    buttonGroup->addButton(yearButton6, 6);
    buttonGroup->addButton(yearButton7, 7);
    buttonGroup->addButton(yearButton8, 8);
    buttonGroup->addButton(yearButton9, 9);
    buttonGroup->addButton(yearButton10, 10);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    QHBoxLayout *hbox = new QHBoxLayout;

    QPushButton *prevButton = new QPushButton;
    prevButton->setText("ᐸ");
    prevButton->setCursor(Qt::PointingHandCursor);
    prevButton->setFixedSize(43, 43);
    prevButton->setAutoRepeat(true);

    QLabel *periodYearsLabel = new QLabel;

    QPushButton *nextButton = new QPushButton;
    nextButton->setText("ᐳ");
    nextButton->setCursor(Qt::PointingHandCursor);
    nextButton->setFixedSize(43, 43);
    nextButton->setAutoRepeat(true);

    hbox->addWidget(prevButton);
    hbox->addStretch();
    hbox->addWidget(periodYearsLabel);
    hbox->addStretch();
    hbox->addWidget(nextButton);
    vbox->addLayout(hbox);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    grid->setSpacing(0);
    grid->addWidget(yearButton1, 1, 1, Qt::AlignCenter);
    grid->addWidget(yearButton2, 1, 2, Qt::AlignCenter);
    grid->addWidget(yearButton3, 2, 1, Qt::AlignCenter);
    grid->addWidget(yearButton4, 2, 2, Qt::AlignCenter);
    grid->addWidget(yearButton5, 3, 1, Qt::AlignCenter);
    grid->addWidget(yearButton6, 3, 2, Qt::AlignCenter);
    grid->addWidget(yearButton7, 4, 1, Qt::AlignCenter);
    grid->addWidget(yearButton8, 4, 2, Qt::AlignCenter);
    grid->addWidget(yearButton9, 5, 1, Qt::AlignCenter);
    grid->addWidget(yearButton10, 5, 2, Qt::AlignCenter);

    vbox->addLayout(grid);
    m_years->setLayout(vbox);

    foreach (QAbstractButton* button, buttonGroup->buttons()) {
        int selectedYear = m_calendar->selectedDate().year();
        int lastNumber = QString::number(selectedYear).right(1).toInt();
        if (lastNumber > 0)
             selectedYear = selectedYear - lastNumber;
        button->setText(QString::number(selectedYear + buttonGroup->id(button) - 1));
        button->setCursor(Qt::PointingHandCursor);
        button->setFixedSize(130, 37);
    }  
    periodYearsLabel->setText(yearButton1->text() + " – " + yearButton10->text());

    connect(prevButton, &QPushButton::clicked, [=] {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            button->setText(QString::number(button->text().toInt() - 10));
        }
        periodYearsLabel->setText(yearButton1->text() + " – " + yearButton10->text());
    });
    connect(nextButton, &QPushButton::clicked, [=] {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            button->setText(QString::number(button->text().toInt() + 10));
        }
        periodYearsLabel->setText(yearButton1->text() + " – " + yearButton10->text());
    });
    connect(this, &CalendarWidget::prevYears, [=] {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            button->setText(QString::number(button->text().toInt() - 10));
        }
        periodYearsLabel->setText(yearButton1->text() + " – " + yearButton10->text());
    });
    connect(this, &CalendarWidget::nextYears, [=] {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            button->setText(QString::number(button->text().toInt() + 10));
        }
        periodYearsLabel->setText(yearButton1->text() + " – " + yearButton10->text());
    });
    connect(buttonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [=](int id) {
        QDate currentDate = m_calendar->selectedDate();
        int buttonYear = buttonGroup->button(id)->text().toInt();
        QDate newDate = currentDate.addYears(buttonYear - currentDate.year());
        m_calendar->setSelectedDate(newDate);
        setAnimation(m_years, m_calendar);
    });
    connect(this, &CalendarWidget::styleChanged, [=](QString style) {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            button->setStyleSheet(style);
        }
        prevButton->setStyleSheet(btnStyle);
        nextButton->setStyleSheet(btnStyle);
    });
    connect(m_calendar, &Calendar::yearButtonClicked, [=] {
        foreach (QAbstractButton* button, buttonGroup->buttons()) {
            int selectedYear = m_calendar->selectedDate().year();
            int lastNumber = QString::number(selectedYear).right(1).toInt();
            if (lastNumber > 0)
                 selectedYear = selectedYear - lastNumber;
            button->setText(QString::number(selectedYear + buttonGroup->id(button) - 1));
        }  
        periodYearsLabel->setText(yearButton1->text() + " – " + yearButton10->text());
        setAnimation(m_calendar, m_years);
    });
}
