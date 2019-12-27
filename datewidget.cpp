#include "datewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QPainter>
#include <QCursor>

DateWidget::DateWidget(QWidget *parent) : QWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;

    prevButton = new QPushButton;
    prevButton->setText("🡠");
    prevButton->setCursor(QCursor(Qt::PointingHandCursor));
    prevButton->setFixedSize(25, 25);
    prevButton->setAutoRepeat(true);

    nextButton = new QPushButton;
    nextButton->setText("🡢");
    nextButton->setCursor(QCursor(Qt::PointingHandCursor));
    nextButton->setFixedSize(25, 25);
    nextButton->setAutoRepeat(true);

    hbox->addWidget(prevButton, 0, Qt::AlignLeft | Qt::AlignTop);
    hbox->addWidget(nextButton, 0, Qt::AlignRight | Qt::AlignTop);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    weekLabel = new QLabel;
    weekLabel->setStyleSheet("margin: 18px 0px 0px 0px;");
    weekLabel->setFixedHeight(40);

    hbox->addWidget(weekLabel, 1, Qt::AlignCenter);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    dateLabel = new QLabel;
    dateLabel->setFixedHeight(64);
    dateLabel->setStyleSheet("font-size: 70px;");

    hbox->addWidget(dateLabel, 2, Qt::AlignCenter);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    monthLabel = new QLabel;
    monthLabel->setFixedHeight(28);

    hbox->addWidget(monthLabel, 3, Qt::AlignHCenter | Qt::AlignTop);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    phasesLabel = new QLabel;
    phasesLabel->setFixedHeight(190);
    zodiacLabel = new QLabel;
    zodiacLabel->setFixedHeight(190);

    hbox->addWidget(phasesLabel, 4, Qt::AlignRight);
    hbox->addSpacing(20);
    hbox->addWidget(zodiacLabel, 4, Qt::AlignLeft);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    closeButton = new QPushButton;
    closeButton->setText("🞨");
    closeButton->setCursor(QCursor(Qt::PointingHandCursor));
    closeButton->setFixedSize(25, 25);

    hbox->addWidget(closeButton, 5, Qt::AlignRight | Qt::AlignBottom);
    vbox->addLayout(hbox);
    setLayout(vbox);
}

DateWidget::~DateWidget()
{
}

void DateWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    int center = this->width() / 2;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(center, 120);
    painter.setPen(Qt::NoPen);

    int alfa = m_settings.value("SetAlfa", 110).toInt();
    bool form = m_settings.value("RoundForm", false).toBool();
    QColor color = QColor(255, 255, 255);
    color.setAlpha(alfa - 40);
    dateColor.setAlpha(alfa);
    if (form == true) {
        if (today == true) {
            painter.setBrush(color);        
            painter.drawEllipse(-90, -90, 180, 180);
        }
        painter.setBrush(dateColor);            
        painter.drawEllipse(-105, -105, 210, 210);
    } else {
        if (today == true) {
            painter.setBrush(color);        
            painter.drawRoundedRect(-80, -80, 160, 160, 14, 14);
        }
        painter.setBrush(dateColor);
        painter.drawRoundedRect(-95, -95, 190, 190, 22, 22);
    }
}

void DateWidget::wheelEvent(QWheelEvent *event)
{
    const int numDegrees = event->delta() / 8;
    const int numSteps = numDegrees / 15;
    sDate = sDate.addDays(-numSteps);
    if (sDate == QDate::currentDate())
        today = true;
    else
        today = false;
    weekLabel->setText(sDate.toString("dddd"));
    dateLabel->setText(sDate.toString("d"));
    monthLabel->setText(sDate.toString("MMMM yyyy"));
    getZodiac();
    moonPhase();
    update();
}

void DateWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    emit hideDate();
}

void DateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::XButton1)
        emit hideDate();
}

void DateWidget::getZodiac()
{
    int month = sDate.month();
    int day = sDate.day();
    int date = (month * 100) + day;
    QString icon;

    if (date >= 321 && date <= 419)
             icon = "aries";
    else if (date >= 420 && date <= 520)
             icon = "taurus";
    else if (date >= 521 && date <= 620)
             icon = "gemini";
    else if (date >= 621 && date <= 722)
             icon = "cancer";
    else if (date >= 723 && date <= 822)
             icon = "leo";
    else if (date >= 823 && date <= 922)
             icon = "virgo";
    else if (date >= 923 && date <= 1022)
             icon = "libra";
    else if (date >= 1023 && date <= 1121)
             icon = "scorpio";
    else if (date >= 1122 && date <= 1221)
             icon = "sagittarius";
    else if (date >= 1222 || date <= 119)
             icon = "capricorn";
    else if (date >= 120 && date <= 218)
             icon = "aquarius";
    else if (date >= 219 && date <= 320)
             icon = "pisces";

    const auto ratio = qApp->devicePixelRatio();
    const int iconSize = static_cast<int> (90 * ratio);
    QString iconPath = QString(":/icons/resources/icons/zodiac/%1.svg").arg(icon);
    QPixmap pixmap = QPixmap(iconPath).scaled(iconSize, iconSize,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);
    zodiacLabel->setPixmap(pixmap);
}

void DateWidget::moonPhase()
{
    int y = sDate.year();
    int m = sDate.month();
    int d = sDate.day();
    int c, e;
    double jd;
    int b;

    if (m < 3) {
        y--;
        m += 12;
    }
    ++m;
    c = 365.25 * y;
    e = 30.6 * m;
    jd = c + e + d - 694039.09;
    jd /= 29.5305882;
    b = jd;
    jd -= b;
    b = jd * 16 + 0.5;
    b = b & 15;

    QString 	phase =	QString::number(b);
    const auto ratio = qApp->devicePixelRatio();
    const int iconSize = static_cast<int> (90 * ratio);
    QString iconPath = QString(":/icons/resources/icons/moon/%1.svg").arg(phase);
    QPixmap pixmap = QPixmap(iconPath).scaled(iconSize, iconSize,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);
    QTransform trans;
    int angle = m_settings.value("Angle", 180).toInt();
    trans.rotate(angle);
    phasesLabel->setPixmap(pixmap.transformed(trans));
}