TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    tank.cpp \
    projectile.cpp \
    trophy.cpp \
    game.cpp

LIBS += `pkg-config --libs allegro-5.0 allegro_dialog-5.0 allegro_ttf-5.0 allegro_font-5.0 allegro_primitives-5.0 allegro_image-5.0 allegro_audio-5.0 allegro_acodec-5.0`

QMAKE_CXXFLAGS += -std=gnu++14

HEADERS += \
    tank.h \
    projectile.h \
    trophy.h \
    defines.h \
    game.h
