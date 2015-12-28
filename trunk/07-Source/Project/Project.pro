#-------------------------------------------------
#
# Project created by QtCreator 2012-11-10T14:55:04
#
#-------------------------------------------------

QT       += core gui

TARGET = Project
TEMPLATE = app


SOURCES += \
    about/about.cpp \
    license/license.cpp \
    presets/presets.cpp \
    presets/folders.cpp \
    util.cpp \
    tabmanager.cpp \
    tab.cpp \
    sortproxy.cpp \
    settingsglobal.cpp \
    settings.cpp \
    parms.cpp \
    mw_menuaction.cpp \
    matcher.cpp \
    mapped.cpp \
    mainwindow.cpp \
    main.cpp \
    highlighter.cpp \
    grep.cpp \
    find.cpp \
    encoding.cpp

HEADERS  += \
    about/about.h \
    license/license.h \
    presets/presets.h \
    presets/folders.h \
    util.h \
    types.h \
    tabmanager.h \
    tab.h \
    stabwidget.h \
    sortproxy.h \
    settingsglobal.h \
    settings.h \
    parms.h \
    matcher.h \
    mapped.h \
    mainwindow.h \
    highlighter.h \
    greprun.h \
    grep.h \
    findrun.h \
    find.h \
    debug.h \
    atomicqueue.h

FORMS    += \
    about/about.ui \
    license/license.ui \
    presets/presets.ui \
    presets/folders.ui \
    mainwindow.ui \
    settings.ui \
    settingsglobal.ui \
    tab.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    license/gpl-3.0.html
