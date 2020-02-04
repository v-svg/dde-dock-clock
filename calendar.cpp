#include "calendar.h"
#include "datetimeplugin.h"
#include <QPainter>
#include <QApplication>
#include <QTableView>
#include <QHeaderView>
#include <QToolButton>

Calendar::Calendar(QWidget *parent) : QCalendarWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

	QTableView *table = findChild<QTableView *>("qt_calendar_calendarview");
    if (table) {
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->horizontalHeader()->setDefaultSectionSize(43);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->verticalHeader()->setDefaultSectionSize(43);
    table->setStyleSheet("QTableView {selection-background-color: transparent;}"
                         "QTableView {alternate-background-color: transparent;}");
    }

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
    prevmonth->setFixedSize(43, 43);
    prevmonth->setIcon(QIcon());
    prevmonth->setText("🡠");
    prevmonth->setStyleSheet(textStyle);
    prevmonth->setCursor(Qt::PointingHandCursor);
    }

    QToolButton *nextmonth = findChild<QToolButton *>("qt_calendar_nextmonth");
    if (nextmonth) {
    nextmonth->setFixedSize(43, 43);
    nextmonth->setIcon(QIcon());
    nextmonth->setText("🡢");
    nextmonth->setStyleSheet(textStyle);
    nextmonth->setCursor(Qt::PointingHandCursor);
    }
}

void Calendar::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QCalendarWidget::paintCell(painter, rect, date);
    QString dateDay = QString::number(date.day());

    bool form = m_settings.value("RoundForm", false).toBool();
    int alfa = m_settings.value("SetAlfa", 110).toInt();

    QColor selCellColor = cellColor;
    selCellColor.setAlpha(alfa);
    QColor color = QColor(255, 255, 255);
    color.setAlpha(alfa - 40);

    QFont font = qApp->font();
    font.setWeight(QFont::Black);
    font.setPointSize(font.pointSize() + 1);
    if (font.pointSize()  > 14)
        font.setPointSize(14);
    painter->setFont(font);

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    if (date == selectedDate() && date == QDate::currentDate()) {
        painter->setBrush(color);
        if (form == true)
            painter->drawEllipse(rect.adjusted(5.5, 5.5, -5.5, -5.5));
        else
            painter->drawRoundedRect(rect.adjusted(5.5, 5.5, -5.5, -5.5), 3, 3);
    color = Qt::transparent;
    }

    if (date == selectedDate()) {
        painter->setBrush(selCellColor);
        if (form == true)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
    if (date != QDate::currentDate())
        painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, dateDay);
    }
    
    if (date == QDate::currentDate()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        if (form == true)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
        painter->setPen(Qt::white);
        painter->drawText(rect, Qt::AlignCenter, dateDay);
    }
}
