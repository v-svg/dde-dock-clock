#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "dde-dock/pluginsiteminterface.h"
#include "calendar.h"
#include "datewidget.h"

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    ~CalendarWidget();

signals:
    void styleChanged(const QString style);
    void prevYears();
    void nextYears();

public slots:
    void jumpToToday();
    void updateStyle();

protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e);

private slots:
    void updateTime();
    void prevDay();
    void nextDay();

private:
    void selectDate();
    void setDate(const QDate &date);
    void setMonthWidget();
    void setYearsWidget();
    void setAnimation(QWidget *visibleWidget, QWidget *hiddenWidget);
    void setWeekendColor();
    QSettings m_settings;
    QLabel *timeLabel;
    QPushButton *todayButton;
    Calendar *m_calendar;
    DateWidget *m_date;
    QWidget *m_months;
    QWidget *m_years;
    QColor m_color;
    QButtonGroup *buttonGroup;
    QString btnStyle;
};

#endif // CALENDARWIDGET_H
