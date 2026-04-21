QT       += core testlib network
QT       -= gui

CONFIG   += c++17 console
CONFIG   -= app_bundle

TARGET   = FilterDesignerTests
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    tst_FilterCalculator.cpp \
    tst_OllamaClient.cpp \
    tst_CodeExecutor.cpp \
    ../src/FilterCalculator.cpp \
    ../src/OllamaClient.cpp \
    ../src/CodeExecutor.cpp

HEADERS += \
    ../include/FilterTypes.h \
    ../include/FilterCalculator.h \
    ../include/OllamaClient.h \
    ../include/CodeExecutor.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
