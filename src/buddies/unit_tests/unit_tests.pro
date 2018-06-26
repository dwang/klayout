
DESTDIR_UT = $$OUT_PWD/../..
DESTDIR = $$OUT_PWD/..

TARGET = bd_tests

include($$PWD/../../klayout.pri)
include($$PWD/../../lib_ut.pri)

SOURCES = \
  bdBasicTests.cc \
  bdConverterTests.cc \
  bdStrm2txtTests.cc \
  bdStrmclipTests.cc \
  bdStrmcmpTests.cc \
  bdStrmxorTests.cc \
  bdStrmrunTests.cc \


INCLUDEPATH += $$BD_INC $$DB_INC $$TL_INC $$GSI_INC
DEPENDPATH += $$BD_INC $$DB_INC $$TL_INC $$GSI_INC

LIBS += -L$$DESTDIR_UT -lklayout_bd -lklayout_db -lklayout_tl -lklayout_gsi

PLUGINPATH += \
  $$PWD/../../plugins/common \
  $$PWD/../../plugins/streamers/gds2/db_plugin \
  $$PWD/../../plugins/streamers/cif/db_plugin \
  $$PWD/../../plugins/streamers/oasis/db_plugin \
  $$PWD/../../plugins/streamers/dxf/db_plugin \

INCLUDEPATH += $$PLUGINPATH
DEPENDPATH += $$PLUGINPATH
