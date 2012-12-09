# -------------------------------------------------
# Project created by QtCreator 2009-12-04T10:58:22
# -------------------------------------------------
QT += network \
    xml \
    dbus

QT -= gui
TARGET = q_whiteboard_node
TEMPLATE = lib
CONFIG += debug

CONFIG +=link_pkgconfig

PKGCONFIG = libwhiteboard-util \
            dbus-glib-1
#CFLAGS += -g \
#    -O0
INCLUDEPATH += ../include /usr/lib/dbus-1.0/include/dbus/ /usr/include/glib-2.0

DEFINES += LIBQ_WHITEBOARD_NODE
SOURCES += q_whiteboard_node.cpp \
    insertresponsehandler.cpp \
    subscriptionwqlbasebool.cpp \
    subscriptionwqlbaselist.cpp \
    subscriptionbase.cpp \
    triple.cpp \
    sparqlquery.cpp \
    wqlvaluessubscription.cpp \
    wqlissubtypequery.cpp \
    wqlrelatedsubscription.cpp \
    wqlvaluesquery.cpp \
    wqlrelatedquery.cpp \
    wqlistypequery.cpp \
    wqlnodetypesquery.cpp \
    templatequeryhandler.cpp \
    templatesubscription.cpp \
    templatequery.cpp \
    querywqlbaselisthandler.cpp \
    querywqlbaselist.cpp \
    querywqlbaseboolhandler.cpp \
    querywqlbasebool.cpp \
    querysparqlbasebindings.cpp \
    querysparqlbasebindingshandler.cpp \
    querybase.cpp \
    q_sib_info.cpp \
    q_sib_discovery.cpp 

HEADERS +=  ../include/q_whiteboard_node.h \
    ../include/templatequeryhandler.h \
    ../include/templatequery.h \
    ../include/templatesubscription.h \
    ../include/wqlvaluessubscription.h \
    ../include/wqlrelatedquery.h \
    ../include/wqlrelatedsubscription.h \
    ../include/wqlvaluesquery.h \
    ../include/wqlnodetypesquery.h \
    ../include/wqlistypequery.h \
    ../include/wqlissubtypequery.h \
    ../include/triple.h \
    ../include/subscriptionbase.h \
    ../include/subscriptionwqlbaselist.h \
    ../include/subscriptionwqlbasebool.h \
    ../include/querywqlbaseboolhandler.h \
    ../include/querywqlbasebool.h \
    ../include/querybase.h \
    ../include/querywqlbaselist.h \
    ../include/querywqlbaselisthandler.h \
    ../include/querysparqlbasebindings.h \
    ../include/querysparqlbasebindingshandler.h \
    ../include/sparqlquery.h \
    ../include/insertresponsehandler.h \
    ../include/q_sib_info.h \
    ../include/q_sib_discovery.h

#LIBRARY_INSTALL = /opt/qtsdk-2010.02/lib
#HEADER_INSTALL = /opt/qtsdk-2010.02/include/whiteboard/qt-core

LIBRARY_INSTALL = /usr/local/lib
HEADER_INSTALL = /usr/local/include/whiteboard/qt-core

target.path = $$LIBRARY_INSTALL
headers.files = $$HEADERS
headers.path = $$HEADER_INSTALL
INSTALLS += target headers
