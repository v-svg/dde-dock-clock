QT              += widgets svg
TEMPLATE         = lib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkwidget

TARGET           = clock
DESTDIR          = $$_PRO_FILE_PWD_
DISTFILES       += datetime.json

HEADERS += \
    calendar.h \
    calendarwidget.h \
    datetimeplugin.h \
    datetimewidget.h \
    datewidget.h

SOURCES += \
    calendar.cpp \
    calendarwidget.cpp \
    datetimeplugin.cpp \
    datetimewidget.cpp \
    datewidget.cpp

RESOURCES += resources.qrc
