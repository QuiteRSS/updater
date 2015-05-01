QT       += core gui

TARGET = Updater
TEMPLATE = app

SOURCES += \
    src/mainwindow.cpp \
    src/main.cpp \
    src/logfile.cpp

HEADERS  += \
    src/version.h \
    src/mainwindow.h \
    src/logfile.h

CONFIG(debug, debug|release) {
  BUILD_DIR = debug
} else {
  BUILD_DIR = release
}

DESTDIR = $${BUILD_DIR}/target/
OBJECTS_DIR = $${BUILD_DIR}/obj/
MOC_DIR = $${BUILD_DIR}/moc/
RCC_DIR = $${BUILD_DIR}/rcc/

include(3rdparty/qtsingleapplication/qtsingleapplication.pri)

CODECFORTR  = UTF-8
CODECFORSRC = UTF-8

RESOURCES += \
    updater.qrc

RC_FILE = updaterapp.rc

DISTFILES += \
    README.md
