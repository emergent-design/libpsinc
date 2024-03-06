QT += core gui widgets
QMAKE_CXXFLAGS += -std=c++17	# only required for 18.04 builds

TARGET = iconograph
TEMPLATE = app

INCLUDEPATH += include ../include
SOURCES += src/*.cpp
HEADERS += include/*.h
RESOURCES += resources/*.qrc
FORMS   += ui/*.ui
LIBS    += -L../lib -lpsinc -lfreeimage -lusb-1.0
CONFIG  += rtti c++17

DESTDIR = bin
OBJECTS_DIR = bin/.obj
MOC_DIR = bin/.moc
RCC_DIR = bin/.rcc
UI_DIR = bin/.ui

# Parallelisation now supported on windows builds yet
#linux {
#    LIBS += -ltbb
#}
