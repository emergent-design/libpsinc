QT       += core gui
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iconograph-qt
TEMPLATE = app

INCLUDEPATH += include/iconograph-qt
SOURCES += src/iconograph-qt/*.cpp
HEADERS += include/iconograph-qt/*.h
FORMS   += ui/*.ui
LIBS    += -lemergent -lpsinc

DESTDIR = bin
OBJECTS_DIR = bin/.obj
MOC_DIR = bin/.moc
RCC_DIR = bin/.rcc
UI_DIR = bin/.ui
