/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dde-dock/constants.h"
#include "datetimewidget.h"
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

DatetimeWidget::DatetimeWidget(QWidget *parent) : QWidget(parent),
      m_settings("deepin", "dde-dock-clock"),
      m_24HourFormat(m_settings.value("24HourFormat", true).toBool())
{
}

bool DatetimeWidget::enabled()
{
    return m_settings.value(PLUGIN_STATE_KEY, true).toBool();
}

void DatetimeWidget::setEnabled(const bool b)
{
    m_settings.setValue(PLUGIN_STATE_KEY, b);
}

void DatetimeWidget::toggleClock()
{
    m_settings.setValue("ShowClock", true);
    update();
}

void DatetimeWidget::toggleHourFormat()
{
    m_24HourFormat = !m_24HourFormat;
    m_settings.setValue("24HourFormat", m_24HourFormat);
    m_cachedTime.clear();
    update();
    emit requestUpdateGeometry();
    m_settings.setValue("ShowClock", false);
}

QSize DatetimeWidget::sizeHint() const
{
    const QDateTime current = QDateTime::currentDateTime();
    QFontMetrics FM(qApp->font());
    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    if (m_24HourFormat) {
        QString format = m_settings.value("Format", "HH:mm\nM/d ddd").toString();
        QString text = current.toString(format);
        if (text.contains("\\n")) {
            QStringList SL = text.split("\\n");
            if (SL.at(0).length() > SL.at(1).length())
                return QSize(FM.boundingRect(SL.at(0)).width() + 12, FM.boundingRect("88:88").height() * 2);
            else
                return QSize(FM.boundingRect(SL.at(1)).width() + 12, FM.boundingRect("88:88").height() * 2);
        } else
            return QSize(FM.boundingRect(text).width() + 12, 26);
    } else
        if (position == Dock::Top || position == Dock::Bottom)
            return QSize(FM.boundingRect("88:88 AM").width() + 12, 26);
        else
            return QSize(FM.boundingRect("88:88").width() + 12, FM.boundingRect("AM").height() * 2);
}

void DatetimeWidget::resizeEvent(QResizeEvent *e)
{
    m_cachedTime.clear();
    QWidget::resizeEvent(e);
}

void DatetimeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton)
        emit mouseMidBtnClicked();

    QWidget::mouseReleaseEvent(e);
}

void DatetimeWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    const auto ratio = qApp->devicePixelRatio();
    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();
    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    const QDateTime current = QDateTime::currentDateTime();
    const QTime time = QTime::currentTime();
    const QRect rct(rect());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (displayMode == Dock::Efficient) {
        QString format;
        if (m_24HourFormat)
            if (position == Dock::Top || position == Dock::Bottom)
                format = m_settings.value("Format","HH:mm\nyyyy/M/d ddd").toString().replace("\\n","\n");
            else {
                QFontMetrics FM(qApp->font());
                format = m_settings.value("Format","HH:mm\nyyyy/M/d ddd").toString().replace("\\n","\n");
                QString text = current.toString(format);
                if (FM.boundingRect(rect(), Qt::AlignCenter, text).width() > width())
                    format.remove(" ddd").remove("ddd ").remove("yyyy/").remove(".yyyy").remove(":ss");
                text = current.toString(format);
                if (FM.boundingRect(rect(), Qt::AlignCenter, text).width() > width())
                    format = "HH:mm";
            }
        else {
            if (position == Dock::Top || position == Dock::Bottom)
                format = "hh:mm AP";
            else
                format = "hh:mm\nAP";
        }

        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, current.toString(format));
        return;
    }
    // position and size
    int side = qMin(width(), height());
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);
    // clock face shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 10));
    painter.drawEllipse(QPoint(0, 5), 76, 76);
    painter.setBrush(QColor(0, 0, 0, 30));
    painter.drawEllipse(QPoint(0, 5), 71, 71);
    // clock face
    painter.setBrush(QColor(249, 175, 21));
    painter.drawEllipse(-69.5, -69.5, 139, 139);
    painter.setBrush(Qt::white);
    painter.drawEllipse(-61.5, -61.5, 123, 123);
    // analog clock
    if (m_settings.value("ShowClock", false).toBool()) {
        // clock hands
        static const int hourHand[8] = { -3, 0, 3, 0, 3, -30, -3, -30 };
        static const int minuteHand[8] = { -2, 0, 2, 0, 2, -42, -2, -42 };
        static const int secondHand[12] = { -1, 0, -1, -55, 1, -55, 1, 0, 4, 20, -4, 20 };
        static const int hourHandShadow[8] = { -4, 0, 4, 0, 4, -30, -4, -30 };
        static const int minuteHandShadow[8] = { -3, 0, 3, 0, 3, -42, -3, -42 };
        static const int secondHandShadow[12] = { -2, 0, -2, -55, 2, -55, 2, 0, 4, 20, -4, 20 };
        // hour dots
        painter.setBrush(QColor(94, 94, 94));
        for (int i = 0; i < 12; ++i) {
            painter.drawEllipse(48, -4, 8, 8);
            painter.rotate(30.0);
        }
        // hour hand shadow
        painter.translate(QPoint(0, 5));
        painter.setBrush(QColor(0, 0, 0, 60));
        painter.save();
        painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
        painter.drawConvexPolygon(QPolygon(4, hourHandShadow));
        painter.restore();
        // hour hand
        painter.translate(QPoint(0, -5));
        painter.setBrush(QColor(16, 16, 16));
        painter.save();
        painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
        painter.drawConvexPolygon(QPolygon(4, hourHand));
        painter.restore();
        // minute hand shadow
        painter.translate(QPoint(0, 6));
        painter.setBrush(QColor(0, 0, 0, 60));
        painter.save();
        painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
        painter.drawConvexPolygon(QPolygon(4, minuteHandShadow));
        painter.restore();
        painter.drawEllipse(-8, -8, 16, 16);
        // minute hand
        painter.translate(QPoint(0, -6));
        painter.setBrush(QColor(36, 36, 36));
        painter.save();
        painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
        painter.drawConvexPolygon(QPolygon(4, minuteHand));
        painter.restore();
        painter.drawEllipse(-8, -8, 16, 16);
        // second hand shadow
        painter.translate(QPoint(0, 7));
        painter.setBrush(QColor(0, 0, 0, 60));
        painter.save();
        painter.rotate(6.0 * (time.second()));
        painter.drawConvexPolygon(QPolygon(6, secondHandShadow));
        painter.restore();
        // second hand
        painter.translate(QPoint(0, -7));
        painter.setBrush(Qt::red);
        painter.save();
        painter.rotate(6.0 * (time.second()));
        painter.drawConvexPolygon(QPolygon(6, secondHand));
        painter.restore();
        painter.drawEllipse(-6, -6, 12, 12);

        return;
    }
    const QString currentTimeString = current.toString(m_24HourFormat ? "hhmm" : "hhmma");
    painter.resetTransform();
    // check cache valid
    if (m_cachedTime != currentTimeString) {
        m_cachedTime = currentTimeString;
        // draw new pixmap
        m_cachedIcon = QPixmap(size() * ratio);
        m_cachedIcon.fill(Qt::transparent);
        m_cachedIcon.setDevicePixelRatio(ratio);
        QPainter p(&m_cachedIcon);
        // draw fashion mode datetime plugin
        const int bigNumHeight = side / 3.2;
        const int bigNumWidth = double(bigNumHeight) * 8 / 18;
        const int smallNumHeight = bigNumHeight / 2;
        const int smallNumWidth = double(smallNumHeight) * 5 / 9;
        // draw big num 1
        const QString bigNum1Path = QString(":/icons/resources/icons/big%1.svg").arg(currentTimeString[0]);
        const QPixmap bigNum1 = loadSvg(bigNum1Path, QSize(bigNumWidth, bigNumHeight));
        const QPoint bigNum1Offset = QPoint(width() / 2 - bigNumWidth * 2 + 1, height() / 2 - bigNumHeight / 2);
        p.drawPixmap(bigNum1Offset, bigNum1);
        // draw big num 2
        const QString bigNum2Path = QString(":/icons/resources/icons/big%1.svg").arg(currentTimeString[1]);
        const QPixmap bigNum2 = loadSvg(bigNum2Path, QSize(bigNumWidth, bigNumHeight));
        const QPoint bigNum2Offset = bigNum1Offset + QPoint(bigNumWidth + 1, 0);
        p.drawPixmap(bigNum2Offset, bigNum2);

        if (!m_24HourFormat) {
            // draw small num 1
            const QString smallNum1Path = QString(":/icons/resources/icons/small%1.svg").arg(currentTimeString[2]);
            const QPixmap smallNum1 = loadSvg(smallNum1Path, QSize(smallNumWidth, smallNumHeight));
            const QPoint smallNum1Offset = bigNum2Offset + QPoint(bigNumWidth + 2, 1);
            p.drawPixmap(smallNum1Offset, smallNum1);
            // draw small num 2
            const QString smallNum2Path = QString(":/icons/resources/icons/small%1.svg").arg(currentTimeString[3]);
            const QPixmap smallNum2 = loadSvg(smallNum2Path, QSize(smallNumWidth, smallNumHeight));
            const QPoint smallNum2Offset = smallNum1Offset + QPoint(smallNumWidth + 1, 0);
            p.drawPixmap(smallNum2Offset, smallNum2);
            // draw am/pm tips
            const int tips_width = (smallNumWidth * 2 + 2) & ~0x1;
            const int tips_height = tips_width / 2;

            QPixmap tips;
            if (current.time().hour() > 12)
                tips = loadSvg(":/icons/resources/icons/tips-pm.svg", QSize(tips_width, tips_height));
            else
                tips = loadSvg(":/icons/resources/icons/tips-am.svg", QSize(tips_width, tips_height));

            const QPoint tipsOffset = bigNum2Offset + QPoint(bigNumWidth + 2, bigNumHeight - tips_height);
            p.drawPixmap(tipsOffset, tips);
        } else {
            // draw small num 1
            const QString smallNum1Path = QString(":/icons/resources/icons/small%1.svg").arg(currentTimeString[2]);
            const QPixmap smallNum1 = loadSvg(smallNum1Path, QSize(smallNumWidth, smallNumHeight));
            const QPoint smallNum1Offset = bigNum2Offset + QPoint(bigNumWidth + 2, smallNumHeight);
            p.drawPixmap(smallNum1Offset, smallNum1);
            // draw small num 2
            const QString smallNum2Path = QString(":/icons/resources/icons/small%1.svg").arg(currentTimeString[3]);
            const QPixmap smallNum2 = loadSvg(smallNum2Path, QSize(smallNumWidth, smallNumHeight));
            const QPoint smallNum2Offset = smallNum1Offset + QPoint(smallNumWidth + 1, 0);
            p.drawPixmap(smallNum2Offset, smallNum2);
        }
    }
    // draw cached fashion mode time item
    painter.drawPixmap(rect().center() - m_cachedIcon.rect().center() / ratio, m_cachedIcon);
}

const QPixmap DatetimeWidget::loadSvg(const QString &fileName, const QSize size)
{
    const auto ratio = qApp->devicePixelRatio();

    QPixmap pixmap(size * ratio);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);

    return pixmap;
}
