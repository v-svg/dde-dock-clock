#ifndef CALENDAR_H
#define CALENDAR_H

#include "dde-dock/pluginsiteminterface.h"
#include <QCalendarWidget>
#include <QColor>
#include <QDate>
#include <QPen>
#include <QBrush>

class Calendar : public QCalendarWidget
{
    Q_OBJECT

public:
    Calendar(QWidget *parent = nullptr);
    ~Calendar();
    QString textStyle;
    QColor cellColor;
    void updateButtonStyle();

protected:
    virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;

private:
    QSettings m_settings;

};

#endif // CALENDAR_H
