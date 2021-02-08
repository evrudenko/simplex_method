######################################################################
# Automatically generated by qmake (3.1) Thu Dec 5 16:01:20 2019
######################################################################

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += sources/main.cpp \
           sources/MainWindow.cpp \
           sources/SetupWindow.cpp \
           sources/Optimizer.cpp \
           sources/Controller.cpp \
           sources/TableSaver.cpp \
           sources/SystemSaver.cpp \
           sources/ABMSystemSaver.cpp \
           sources/Fraction.cpp

HEADERS += headers/MainWindow.hpp \
           headers/SetupWindow.hpp \
           headers/Optimizer.hpp \
           headers/Controller.hpp \
           headers/TableSaver.hpp \
           headers/SystemSaver.hpp \
           headers/ABMSystemSaver.hpp \
           headers/Fraction.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target