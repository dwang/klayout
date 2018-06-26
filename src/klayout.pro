
include(klayout.pri)

TEMPLATE = subdirs

SUBDIRS = \
  klayout_main \
  unit_tests \
  tl \
  gsi \
  db \
  rdb \
  lym \
  laybasic \
  lay \
  ant \
  img \
  edt \
  lib \
  plugins \
  buddies \
  fontgen \

LANG_DEPENDS =
MAIN_DEPENDS =

equals(HAVE_RUBY, "1") {
  SUBDIRS += rba
  LANG_DEPENDS += rba
  rba.depends += gsi db
} else {
  SUBDIRS += rbastub
  rbastub.depends += gsi
  LANG_DEPENDS += rbastub
}

equals(HAVE_PYTHON, "1") {
  SUBDIRS += pya
  LANG_DEPENDS += pya
  pya.depends += gsi db
  SUBDIRS += pymod
  pymod.depends += pya lay
} else {
  SUBDIRS += pyastub
  pyastub.depends += gsi
  LANG_DEPENDS += pyastub
}

gsi.depends += tl
db.depends += gsi
rdb.depends += db
laybasic.depends += rdb
ant.depends += laybasic
img.depends += laybasic
edt.depends += laybasic

equals(HAVE_RUBY, "1") {
  SUBDIRS += drc
  MAIN_DEPENDS += drc
  drc.depends += rdb lym
}

lym.depends += gsi $$LANG_DEPENDS
lay.depends += laybasic ant img edt lym
lib.depends += db

equals(HAVE_QTBINDINGS, "1") {
  SUBDIRS += gsiqt
  gsiqt.depends += gsi db
  laybasic.depends += gsiqt
  pymod.depends += gsiqt
}

plugins.depends += lay lib rdb ant

buddies.depends += plugins $$LANG_DEPENDS
klayout_main.depends += plugins $$MAIN_DEPENDS
unit_tests.depends += plugins $$MAIN_DEPENDS
