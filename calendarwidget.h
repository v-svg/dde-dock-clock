#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "dde-dock/pluginsiteminterface.h"
#include "calendar.h"
#include "datewidget.h"
#include <QWidget>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLabel>

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    ~CalendarWidget();
    void updateDateStyle();
    DateWidget *datewidget;

protected:
    void paintEvent(QPaintEvent *e);

public slots:
    void jumpToToday();

private:
    QSettings m_settings;
    void updateTime();
    void showDay();
    void showCal();
    void prevDay();
    void nextDay();
    void selDate();
    void setDate();
    QLabel *currentTime;
    QPushButton *currentDate;
    Calendar *calendar;
    QDate selectDate;
};

#endif // CALENDARWIDGET_H
