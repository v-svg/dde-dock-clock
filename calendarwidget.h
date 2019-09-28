#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "dde-dock/pluginsiteminterface.h"
#include "calendar.h"
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

protected:
    void paintEvent(QPaintEvent *e);

public slots:
    void jumpToToday();

private:
    QSettings m_settings;
    void updateTime();
    QLabel *currentTimeLabel;
    QPushButton *currentDateLabel;
    Calendar *calendar;

};

#endif // CALENDARWIDGET_H
