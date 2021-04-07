#ifndef CALENDAR_H
#define CALENDAR_H

#include "dde-dock/pluginsiteminterface.h"
#include <QCalendarWidget>
#include <QTableView>
#include <QToolButton>

class Calendar : public QCalendarWidget
{
    Q_OBJECT

public:
    Calendar(QWidget *parent = nullptr);
    ~Calendar();
    QColor cellColor;
    void setButtonStyle(QString style);

signals:
    void monthButtonClicked();
    void yearButtonClicked();
    void paletteChange();

protected:
    bool event(QEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;

private slots:
    void updateMonthButton();

private:
    QSettings m_settings;
    QTableView *m_tableView;
    QToolButton *m_prevButton;
    QToolButton *m_nextButton;
    QToolButton *m_monthButton;
    QToolButton *m_yearButton;
    void updateFont();
};

#endif // CALENDAR_H
