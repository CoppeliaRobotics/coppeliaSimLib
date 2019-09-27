include(v_rep.pri)
include(config.pri)

INCLUDEPATH += $$BOOST_INCLUDEPATH
INCLUDEPATH += $$LUA_INCLUDEPATH
INCLUDEPATH += $$PWD/"sourceCode/verSpec"

LIBS += $$LUA_LIBS
