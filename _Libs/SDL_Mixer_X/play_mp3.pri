include($$PWD/libid3tag/libid3tag.pri)

HEADERS += \
    $$PWD/load_mp3.h \
    $$PWD/dynamic_mp3.h \
    $$PWD/music_mad.h

SOURCES += \
    $$PWD/dynamic_mp3.c \
    $$PWD/load_mp3.c \
    $$PWD/music_mad.c

