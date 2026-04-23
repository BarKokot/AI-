QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = FilterDesigner
TEMPLATE = app

INCLUDEPATH += include

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/FilterCalculator.cpp \
    src/OllamaClient.cpp \
    src/OllamaWorker.cpp \
    src/CodeExecutor.cpp \
    src/ParameterWidget.cpp \
    src/ResultWidget.cpp \
    src/SchematicRenderer.cpp

HEADERS += \
    include/MainWindow.h \
    include/FilterCalculator.h \
    include/OllamaClient.h \
    include/OllamaWorker.h \
    include/CodeExecutor.h \
    include/ParameterWidget.h \
    include/ResultWidget.h \
    include/SchematicRenderer.h \
    include/FilterTypes.h

RESOURCES += resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
