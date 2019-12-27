#ifndef DATEWIDGET_H
#define DATEWIDGET_H

#include <QDate>
#include <QWidget>
#include <QSettings>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QWheelEvent>

class DateWidget : public QWidget
{
    Q_OBJECT

public:
    DateWidget(QWidget *parent = nullptr);
    ~DateWidget();
    QColor dateColor;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QPushButton *closeButton;
    QLabel *weekLabel;
    QLabel *dateLabel;
    QLabel *monthLabel;
    QDate sDate;
    bool today;
    void getZodiac();
    void moonPhase();

signals:
    void hideDate();

protected:
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event);

private:
    QSettings m_settings;
    QLabel *zodiacLabel;
    QLabel *phasesLabel;

};

#endif // DATEWIDGET_H
