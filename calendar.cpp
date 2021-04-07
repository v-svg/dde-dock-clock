#include "calendar.h"
#include "datetimeplugin.h"
#include <QPainter>
#include <QApplication>
#include <QHeaderView>

#define ITEM_SIZE 43

Calendar::Calendar(QWidget *parent) : QCalendarWidget(parent),
      m_settings("deepin", "dde-dock-clock")
{
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    setStyleSheet("#qt_calendar_navigationbar {"
                  "background-color: transparent;"
                  "}"
                  "QCalendarWidget {"
                  "background-color: transparent;"
                  "}"
                  "QToolButton::menu-indicator {"
                  "image: none;"
                  "}");

    m_tableView = findChild<QTableView *>("qt_calendar_calendarview");
    if (m_tableView) {
        m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_tableView->horizontalHeader()->setDefaultSectionSize(ITEM_SIZE);
        m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_tableView->verticalHeader()->setDefaultSectionSize(ITEM_SIZE);
        m_tableView->setStyleSheet("QTableView {"
                                   "selection-background-color: transparent;"
                                   "alternate-background-color: transparent;"
                                   "}"
                                   "QTableView::item:hover {"
                                   "background-color: transparent;"
                                   "}");
    }

    m_prevButton = findChild<QToolButton *>("qt_calendar_prevmonth");
    if (m_prevButton) {
        m_prevButton->disconnect();
        connect(m_prevButton, &QToolButton::clicked, [=] {
            setSelectedDate(selectedDate().addMonths(-1));
        });
    }

    m_nextButton = findChild<QToolButton *>("qt_calendar_nextmonth");
    if (m_nextButton) {
        m_nextButton->disconnect();
        connect(m_nextButton, &QToolButton::clicked, [=] {
            setSelectedDate(selectedDate().addMonths(1));
        });
    }

    m_monthButton = findChild<QToolButton *>("qt_calendar_monthbutton");
    if (m_monthButton) {
        m_monthButton->setCursor(Qt::PointingHandCursor);
        m_monthButton->disconnect();
        connect(m_monthButton, &QToolButton::clicked, this, &Calendar::monthButtonClicked);
        connect(this, &Calendar::currentPageChanged, this, &Calendar::updateMonthButton);

        updateMonthButton();
    }

    m_yearButton = findChild<QToolButton *>("qt_calendar_yearbutton");
    if (m_yearButton) {
        m_yearButton->setCursor(Qt::PointingHandCursor);
        m_yearButton->disconnect();
        connect(m_yearButton, &QToolButton::clicked, this, &Calendar::yearButtonClicked);
    }

    updateFont();
}

Calendar::~Calendar()
{
}

void Calendar::updateMonthButton()
{
    QString text = m_monthButton->text();
    m_monthButton->setText(text.at(0).toUpper() + text.mid(1));
}

bool Calendar::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationFontChange)
        updateFont();

    if (e->type() == QEvent::PaletteChange)
        emit paletteChange();

    return QWidget::event(e);
}

void Calendar::wheelEvent(QWheelEvent *e)
{
    int n;
    if (e->delta() > 0)
        n = -1;
    else
        n = 1;
        
    if (m_tableView->underMouse() || m_monthButton->underMouse()) {
        setSelectedDate(selectedDate().addMonths(n));
    }
    if (m_yearButton->underMouse()) {
        setSelectedDate(selectedDate().addYears(n));
    }
}

void Calendar::updateFont()
{
    QFont font = qApp->font();
    int size = font.pointSize();
    if (size > 13)
        font.setPointSize(13);
    setFont(font);
}

void Calendar::setButtonStyle(QString style)
{
    if (m_prevButton) {
        m_prevButton->setFixedSize(ITEM_SIZE, ITEM_SIZE);
        m_prevButton->setIcon(QIcon());
        m_prevButton->setText("ᐸ");
        m_prevButton->setStyleSheet(style);
        m_prevButton->setCursor(Qt::PointingHandCursor);
    }

    if (m_nextButton) {
        m_nextButton->setFixedSize(ITEM_SIZE, ITEM_SIZE);
        m_nextButton->setIcon(QIcon());
        m_nextButton->setText("ᐳ");
        m_nextButton->setStyleSheet(style);
        m_nextButton->setCursor(Qt::PointingHandCursor);
    }

    if (m_monthButton && m_yearButton) {
        style = style.replace(" font-size: 16px; font-weight: 600;", "");
        m_monthButton->setStyleSheet(style);
        m_yearButton->setStyleSheet(style);
    }
}

void Calendar::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QCalendarWidget::paintCell(painter, rect, date);

    QString dateDay = QString::number(date.day());
    QPoint cursorPos = mapFromGlobal(QCursor::pos()) - QPoint(0, ITEM_SIZE);

    bool selected = date == selectedDate();
    bool current = date == QDate::currentDate();

    bool rounded = m_settings.value("RoundForm", false).toBool();
    bool colored = m_settings.value("ColorWeekend", true).toBool();
    int alpha = m_settings.value("SetAlpha", 129).toInt();

    QColor selectColor = cellColor;
    selectColor.setAlpha(alpha);

    QColor textColor = QColor(QApplication::palette().text().color());
    textColor.setAlpha(alpha - 40);

    QFont font = qApp->font();
    font.setWeight(QFont::Black);
    font.setPointSize(font.pointSize() + 1);
    if (font.pointSize() > 14)
        font.setPointSize(14);
    painter->setFont(font);

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    if (selected && current) {
        painter->setBrush(textColor);
        if (rounded)
            painter->drawEllipse(rect.adjusted(5, 5, -5, -5));
        else
            painter->drawRoundedRect(rect.adjusted(5, 5, -5, -5), 3, 3);
        textColor.setAlpha(0);
    }
    if (rect.contains(cursorPos)) {
        textColor.setAlpha(26);
        painter->setBrush(textColor);
        if (rounded)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
    }
    if (current) {
        if (!selected)
            textColor.setAlpha(alpha - 45);
        painter->setBrush(textColor);
        if (rounded)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
        if (!selected) {
            painter->setPen(Qt::white);
            if (colored) {
                if ((date.dayOfWeek() == 6 || date.dayOfWeek() == 7)) {
                    painter->setPen(cellColor);
                }
            }
            painter->drawText(rect, Qt::AlignCenter, dateDay);
        }
    }
    if (selected) {
        painter->setBrush(selectColor);
        if (rounded)
            painter->drawEllipse(rect);
        else
            painter->drawRoundedRect(rect, 4, 4);
        painter->setPen(Qt::white);
        painter->drawText(rect, Qt::AlignCenter, dateDay);
    }
}
