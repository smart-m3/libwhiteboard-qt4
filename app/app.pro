# -------------------------------------------------
# Project created by QtCreator 2009-12-08T10:06:28
# -------------------------------------------------
QT += network \
    xml
TARGET = q_testapp
CONFIG += gui
CONFIG += link_pkgconfig
PKGCONFIG = libwhiteboard-util \
    dbus-glib-1
TEMPLATE = app

# INCLUDEPATH += /opt/qtsdk-2010.02/include/whiteboard/qt-core
# INCLUDEPATH += /usr/local/include/whiteboard/qt-core
INCLUDEPATH += ../include

# LIBS += -L/opt/qtsdk-2010.02/lib  \
# -lq_whiteboard_node
# LIBS += -L/usr/local/lib  \
# -lq_whiteboard_node
LIBS += -L../src \
    -lq_whiteboard_node
SOURCES += main.cpp \
    q_testapp.cpp
HEADERS += q_testapp.h
FORMS += qtestapp.ui
