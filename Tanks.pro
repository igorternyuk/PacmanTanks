TEMPLATE = app
CONFIG += c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    tank.cpp \
    projectile.cpp \
    trophy.cpp \
    game.cpp

LIBS += `pkg-config --libs allegro-5.0 allegro_dialog-5.0 allegro_ttf-5.0 allegro_font-5.0 allegro_primitives-5.0 allegro_image-5.0 allegro_audio-5.0 allegro_acodec-5.0`
HEADERS += \
    game.hpp \
    projectile.hpp \
    tank.hpp \
    trophy.hpp \
    utils.hpp
