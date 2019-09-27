# location of boost headers:
    BOOST_INCLUDEPATH = "c:/local/boost_1_62_0"    # (e.g. Windows)
    #BOOST_INCLUDEPATH = "/usr/local/include"    # (e.g. MacOS)

# location of lua headers:
    LUA_INCLUDEPATH = "d:/lua-5.1.5/src"    # (e.g. Windows)
    #LUA_INCLUDEPATH = "../../../../../mingw64/include/lua5.1"    # (e.g. Windows-MSYS2)
    #LUA_INCLUDEPATH = "../../lua5_1_4_Linux26g4_64_lib/include"    # (e.g. Ubuntu)
    #LUA_INCLUDEPATH = "/usr/local/include/lua5.1"    # (e.g. MacOS)

# lua libraries to link:
    LUA_LIBS = "d:/lua-5.1.5/src/lua51.lib"    # (e.g. Windows)
    #LUA_LIBS = -llua5.1    # (e.g. Windows-MSYS2)
    #LUA_LIBS = -L"../../lua5_1_4_Linux26g4_64_lib/" -llua5.1    # (e.g. Ubuntu)
    #LUA_LIBS = "/usr/local/lib/liblua5.1.dylib"    # (e.g. MacOS)

# Make sure if a config.pri is found one level above, that it will be used instead of this one:
    exists(../config.pri) { include(../config.pri) }
