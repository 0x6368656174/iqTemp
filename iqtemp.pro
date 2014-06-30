QT       += core gui quick multimedia network

include ('iqlibs/iqlibs.pri')

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iqTemp
TEMPLATE = app

HEADERS += \
    iqtempsensor.h \
    iqtempmainwindow.h \
    iqobjectbasedtablemodel.h \
    iqtempsensoreditdialog.h \
    iqtempproxymodel.h

SOURCES += \
   main.cpp \
   iqtempsensor.cpp \
    iqtempmainwindow.cpp \
    iqobjectbasedtablemodel.cpp \
    iqtempsensoreditdialog.cpp \
    iqtempproxymodel.cpp

RESOURCES += \
    qml.qrc \
    icons.qrc \
    sounds.qrc

OTHER_FILES += \
   qml/iqtemp/FlowListView.qml \
    qml/iqtemp/MimicPanel.qml

FORMS += \
    iqtempmainwindow.ui \
    iqtempsensoreditdialog.ui

TRANSLATIONS += \
    iqtemp_ru.ts

RC_FILE = iqtemp.rc
