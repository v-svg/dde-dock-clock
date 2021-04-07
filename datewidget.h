#ifndef DATEWIDGET_H
#define DATEWIDGET_H

#include <QDate>
#include <QWidget>
#include <QSettings>
#include <QPushButton>
#include <QLabel>
#include <QWheelEvent>

class DateWidget : public QWidget
{
    Q_OBJECT

public:
    DateWidget(QWidget *parent = nullptr);
    ~DateWidget();
    QColor dateColor;
    QLabel *weekLabel;
    QLabel *dateLabel;
    QLabel *monthLabel;
    QDate sDate;
    bool today;
    void setButtonStyle(QString style);
    void setZodiacSign();
    void setMoonPhase();

signals:
    void setPrevDay();
    void setNextDay();
    void hideDateInfo();

protected:
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event);

private:
    QSettings m_settings;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QPushButton *closeButton;
    QLabel *zodiacLabel;
    QLabel *phasesLabel;

};

#endif // DATEWIDGET_H
