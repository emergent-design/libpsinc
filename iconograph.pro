QT += core gui widgets
QMAKE_MAKEFILE = iconograph.make
#QMAKE_CXXFLAGS += -std=c++17	# only required for 18.04 builds

TARGET = iconograph
TEMPLATE = app

INCLUDEPATH += include/iconograph include
SOURCES += src/iconograph/*.cpp
HEADERS += include/iconograph/*.h
RESOURCES += resources/*.qrc
FORMS   += ui/*.ui
LIBS    += -Llib -lpsinc -lfreeimage -lusb-1.0
CONFIG  += rtti c++17

DESTDIR = bin
OBJECTS_DIR = bin/.obj
MOC_DIR = bin/.moc
RCC_DIR = bin/.rcc
UI_DIR = bin/.ui
