#Common settings for Qmmp build

#Extra clean target

unix: QMAKE_DISTCLEAN += -r .build

#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui/
MOC_DIR=./.build/moc/
OBJECTS_DIR=./.build/obj
RCC_DIR=./.build/rcc

#Defines

DEFINES += QT_NO_CAST_FROM_BYTEARRAY QT_STRICT_ITERATORS QT_NO_FOREACH
unix:DEFINES += QMMP_WS_X11
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060200 QT_WARN_DEPRECATED_UP_TO=0x060200 QT_DEPRECATED_WARNINGS

#Configuration

CONFIG -= depend_includepath
CONFIG += hide_symbols
CONFIG += c++17
QT += widgets

#Version

QMMP_VERSION = 2.1.9

#Install paths

VERSIONS = $$split(QMMP_VERSION, ".")

QMMP_VERSION_MAJOR = $$member(VERSIONS, 0)
QMMP_VERSION_MINOR = $$member(VERSIONS, 1)

#Include and link paths
win32 {
EXTRA_INCDIR = C:/devel/mingw32-libs/include
QMAKE_CXXFLAGS += "-isystem $${EXTRA_INCDIR}"
QMAKE_CFLAGS += "-isystem $${EXTRA_INCDIR}"
QMAKE_LIBDIR +=  C:/devel/mingw32-libs/lib
}

#APP_NAME_SUFFIX = "-1"

unix {
  isEmpty(PREFIX): PREFIX=/usr
  isEmpty(BIN_DIR): BIN_DIR=$$PREFIX/bin
  isEmpty(DATA_DIR): DATA_DIR=$$PREFIX/share
  isEmpty(LIB_DIR): LIB_DIR=$$PREFIX/lib
  isEmpty(PLUGIN_DIR): PLUGIN_DIR=$$LIB_DIR/qmmp-$${QMMP_VERSION_MAJOR}.$${QMMP_VERSION_MINOR}
}

#QMAKE_LFLAGS_DEBUG += "-Wl,--as-needed -Wl,--no-undefined -Wl,-z,relro -Wl,--build-id -Wl,--enable-new-dtags"
#QMAKE_LFLAGS += "-Wl,--as-needed -Wl,--no-undefined -Wl,-z,relro -Wl,--build-id -Wl,--enable-new-dtags"

#*-g++ {
#  QMAKE_CXXFLAGS += -Werror=zero-as-null-pointer-constant
#  QMAKE_CXXFLAGS += -Werror=suggest-override
#}

#Comment/uncomment this if you want to change plugins list

CONFIG += ARCHIVE_PLUGIN
CONFIG += JACK_PLUGIN
CONFIG += FLAC_PLUGIN
CONFIG += MUSEPACK_PLUGIN
CONFIG += FFMPEG_PLUGIN
CONFIG += XMP_PLUGIN
CONFIG += WILDMIDI_PLUGIN
CONFIG += GME_PLUGIN
CONFIG += OPUS_PLUGIN
CONFIG += OSS_PLUGIN #deprecated
CONFIG += PULSE_AUDIO_PLUGIN
CONFIG += ALSA_PLUGIN
CONFIG += OSS4_PLUGIN
#CONFIG += PIPEWIRE_PLUGIN
CONFIG += AAC_PLUGIN
CONFIG += MPLAYER_PLUGIN
CONFIG += CDAUDIO_PLUGIN
CONFIG += BS2B_PLUGIN
CONFIG += SOXR_PLUGIN
CONFIG += LADSPA_PLUGIN
CONFIG += FILEWRITER_PLUGIN
CONFIG += PROJECTM_PLUGIN
CONFIG += UDISKS_PLUGIN
CONFIG += SID_PLUGIN
CONFIG += QTMULTIMEDIA_PLUGIN
CONFIG += SHOUT_PLUGIN
CONFIG += HISTORY_PLUGIN
CONFIG += SLEEPINHIBITOR_PLUGIN

#additional features

CONFIG += WITH_MPG123
CONFIG += WITH_MAD
CONFIG += WITH_LIBRCD
CONFIG += WITH_ENCA
CONFIG += WITH_SKINNED
CONFIG += WITH_QSUI

CONFIG -= $$DISABLED_PLUGINS
