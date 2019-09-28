#include "calendar.h"
#include "datetimeplugin.h"
#include <QPainter>
#include <QApplication>
#include <QTableView>
#include <QHeaderView>
#include <QToolButton>
#include <QCursor>

Calendar::Calendar(QWidget *parent)
   : QCalendarWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

	QTableView *table = findChild<QTableView *>();
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	table->horizontalHeader()->setDefaultSectionSize(40);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	table->verticalHeader()->setDefaultSectionSize(40);

    QToolButton *year = findChild<QToolButton *>("qt_calendar_yearbutton");
    if (year) {
    year->setEnabled(false);
    year->setStyleSheet("QToolButton {color: white}");
    }

    QToolButton *month = findChild<QToolButton *>("qt_calendar_monthbutton");
    if (month) {
    month->setEnabled(false);
    month->setStyleSheet("QToolButton {color: white}");
    }

    setStyleSheet("#qt_calendar_navigationbar {background-color: transparent;}"
                  "#qt_calendar_calendarview {selection-background-color: transparent;}"
                  "QTableView {alternate-background-color: transparent;}"
                  "QCalendarWidget {background-color: transparent;}"
                  "QToolButton::menu-indicator {image: none;}");
}

Calendar::~Calendar()
{
}

void Calendar::updateButtonStyle()
{   
    QToolButton *prevmonth = findChild<QToolButton *>("qt_calendar_prevmonth");
    if (prevmonth) {
    prevmonth->setFixedSize(40, 40);
    prevmonth->setIcon(QIcon());
    prevmonth->setText(QString("<<"));
    prevmonth->setStyleSheet(textStyle);
    prevmonth->setCursor(QCursor(Qt::PointingHandCursor));
    }

    QToolButton *nextmonth = findChild<QToolButton *>("qt_calendar_nextmonth");
    if (nextmonth) {
    nextmonth->setFixedSize(40, 40);
    nextmonth->setIcon(QIcon());
    nextmonth->setText(QString(">>"));
    nextmonth->setStyleSheet(textStyle);
    nextmonth->setCursor(QCursor(Qt::PointingHandCursor));
    }
}
void Calendar::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    bool form = m_settings.value("RoundForm", false).toBool();
    int alfa = m_settings.value("SetAlfa", 85).toInt();
    int intColor = m_settings.value("SetColor", 5).toInt();
    QList<QColor> colorList = {
    QColor(244, 67, 54), QColor(233, 30, 99), QColor(156, 39, 176), QColor(103, 58, 183),
    QColor(63, 81, 181), QColor(25, 138, 230), QColor(13, 148, 211), QColor(9, 147, 165),
    QColor(10, 158, 142), QColor(51, 117, 54), QColor(139, 195, 74), QColor(205, 220, 57),
    QColor(255, 235, 59), QColor(255, 193, 7), QColor(234, 165, 62), QColor(255, 87, 34)};
    QColor brushColor = colorList[intColor];
    brushColor.setAlpha(alfa);

    QCalendarWidget::paintCell(painter, rect, date);
    QColor color = QColor(255, 255, 255);
    color.setAlpha(alfa - 25);
    QString dateDay = QString::number(date.day());
    QFont font = qApp->font();
    font.setWeight(QFont::Black);
    painter->setFont(font);
    painter->setRenderHints(QPainter::Antialiasing);

    painter->setPen(Qt::NoPen);
    if (date == selectedDate() && date == QDate::currentDate()) {
        painter->setBrush(color);
        if (form == true)
            painter->drawEllipse(rect.adjusted(6, 6, -6, -6));
        else
            painter->drawRoundedRect(rect.adjusted(6, 6, -6, -6), 3, 3);
    color = Qt::transparent;
    }

    if(date == selectedDate()) {
    painter->setBrush(brushColor);
        if (form == true)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
    if (date != QDate::currentDate())
        painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, dateDay);
    }
    
    painter->setPen(Qt::NoPen);
    if (date == QDate::currentDate()) {
        painter->setBrush(color);
        if (form == true)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
        if (date.dayOfWeek() == 6 or date.dayOfWeek() == 7) {
            if (date != selectedDate())
                painter->setPen(QColor(230, 21, 15));
            else
                painter->setPen(Qt::white);
        } else
            painter->setPen(Qt::white);
        painter->drawText(rect, Qt::AlignCenter, dateDay);
    }
}
