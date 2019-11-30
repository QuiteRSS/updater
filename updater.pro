# VCS revision info
REVFILE = src/version_rev.h
QMAKE_DISTCLEAN += $$REVFILE
exists(.git) {
  VERSION_REV = $$system(git rev-list --count master)
  !build_pass:message(VCS revision: $$VERSION_REV)

  system(echo $${LITERAL_HASH}define VCS_REVISION $$VERSION_REV > $$REVFILE)
} else:!exists($$REVFILE) {
  VERSION_REV = 0
  !build_pass:message(VCS revision: $$VERSION_REV)

  system(echo $${LITERAL_HASH}define VCS_REVISION $$VERSION_REV > $$REVFILE)
}

isEqual(QT_MAJOR_VERSION, 5) {
  QT += widgets
  DEFINES += HAVE_QT5
} else {
  QT += core gui
}

TARGET = Updater
TEMPLATE = app

SOURCES += \
    src/mainwindow.cpp \
    src/main.cpp \
    src/logfile.cpp

HEADERS  += \
    src/version.h \
    src/version_rev.h \
    src/mainwindow.h \
    src/logfile.h

INCLUDEPATH +=  $$PWD/src \

CONFIG(debug, debug|release) {
  BUILD_DIR = debug
} else {
  BUILD_DIR = release
}

CONFIG += embed_manifest_exe
QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:"level='requireAdministrator'"

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
