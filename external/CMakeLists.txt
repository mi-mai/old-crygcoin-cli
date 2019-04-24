set(UPNPC_BUILD_STATIC ON CACHE BOOL "Build static library")
set(UPNPC_BUILD_SHARED OFF CACHE BOOL "Build shared library")
set(UPNPC_BUILD_TESTS OFF CACHE BOOL "Build test executables")
set(WITH_MD_LIBRARY OFF CACHE BOOL "Code Generation for static library")

add_subdirectory(miniupnpc)
add_subdirectory(cryptopp)
add_subdirectory(sqlite3)
add_subdirectory(lmdb)

if(MSVC)
    set_property(TARGET upnpc-static cryptopp-static sqlite3 lmdb PROPERTY FOLDER "external")
elseif(NOT MSVC)
    set_property(TARGET upnpc-static cryptopp-static sqlite3 lmdb PROPERTY FOLDER "external")
endif()

if(MSVC)
  set_property(TARGET upnpc-static APPEND_STRING PROPERTY COMPILE_FLAGS " -wd4244 -wd4267")
elseif(NOT MSVC)
  set_property(TARGET upnpc-static APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-undef -Wno-unused-result -Wno-unused-value")
endif()

