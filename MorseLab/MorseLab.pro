QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = MorseLab
TEMPLATE  = app

# C++17 нужен ядру (структурные привязки). Для Qt 5 нужна версия 5.12+.
CONFIG   += c++17

# Исходники в UTF-8 (кириллица в строках и char32_t-литералах)
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    morse.cpp \
    distortion.cpp \
    pipeline.cpp

HEADERS += \
    mainwindow.h \
    morse.h \
    distortion.h \
    pipeline.h

FORMS += \
    mainwindow.ui
