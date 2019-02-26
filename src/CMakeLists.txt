# Build statically
add_definitions(-DSTATICLIB)

# Add external libraries as include dirs, so we can do #include "httplib" for example
include_directories(${CMAKE_SOURCE_DIR}/external/rocksdb/include)
include_directories(${CMAKE_SOURCE_DIR}/external/cpp-linenoise)
include_directories(${CMAKE_SOURCE_DIR}/external/cpp-httplib)
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann-json)
include_directories(${CMAKE_SOURCE_DIR}/external/rapidjson)
include_directories(${CMAKE_SOURCE_DIR}/external/cxxopts)
include_directories(${CMAKE_SOURCE_DIR}/external/cryptopp)

# Show cmake where the source files are
# Note, if you add remove a source file, you will need to re-run cmake so it
# can find the new file
file(GLOB_RECURSE blockchainExplorer blockchainExplorer/*)
file(GLOB_RECURSE common common/*)
file(GLOB_RECURSE crypto crypto/*)
file(GLOB_RECURSE cryptoNoteCore cryptoNoteCore/* CryptoNoteConfig.h)
file(GLOB_RECURSE cryptoNoteProtocol cryptoNoteProtocol/*)
file(GLOB_RECURSE cryptoTest cryptoTest/*)
file(GLOB_RECURSE errors errors/*)
file(GLOB_RECURSE http http/*)
file(GLOB_RECURSE jsonRpcServer jsonRpcServer/*)
file(GLOB_RECURSE logging logging/*)
file(GLOB_RECURSE miner miner/*)
file(GLOB_RECURSE mnemonics mnemonics/*)
file(GLOB_RECURSE nigel nigel/*)
file(GLOB_RECURSE nodeRpcProxy nodeRpcProxy/*)
file(GLOB_RECURSE p2p p2p/*)
file(GLOB_RECURSE rpc rpc/*)
file(GLOB_RECURSE serialization serialization/*)
file(GLOB_RECURSE service walletService/*)
file(GLOB_RECURSE subWallets subWallets/*)
file(GLOB_RECURSE transfers transfers/*)
file(GLOB_RECURSE TurtleCoind daemon/*)
file(GLOB_RECURSE utilities utilities/*)
file(GLOB_RECURSE wallet wallet/*)
file(GLOB_RECURSE walletApi walletApi/*)
file(GLOB_RECURSE walletBackend walletBackend/*)
file(GLOB_RECURSE zedwallet zedwallet/*)
file(GLOB_RECURSE zedwallet++ zedwallet++/*)

if(MSVC)
    file(GLOB_RECURSE system system/* platform/Windows/System/*)
elseif(APPLE)
    file(GLOB_RECURSE system system/* platform/OSX/System/* platform/Posix/System/*)
else()
    file(GLOB_RECURSE system system/* platform/Linux/System/* platform/Posix/System/*)
endif()

# Group the files together in IDEs
source_group("" FILES $${common} ${crypto} ${cryptoNoteCore} ${cryptoNoteProtocol} ${TurtleCoind} ${jsonRpcServer} ${http} ${logging} ${miner} ${mnemonics} ${nigel} ${nodeRpcProxy} ${p2p} ${rpc} ${serialization} ${system} ${transfers} ${wallet} ${walletApi} ${walletBackend} ${zedwallet} ${zedwallet++} ${cryptoTest} ${errors} ${utilities} ${subWallets})

# Define a group of files as a library to link against
add_library(blockchainExplorer STATIC ${blockchainExplorer})
add_library(common STATIC ${common})
add_library(crypto STATIC ${crypto})
add_library(cryptoNoteCore STATIC ${cryptoNoteCore})
add_library(errors STATIC ${errors})
add_library(http STATIC ${http})
add_library(jsonRpcServer STATIC ${jsonRpcServer})
add_library(logging STATIC ${logging})
add_library(mnemonics STATIC ${mnemonics})
add_library(nigel STATIC ${nigel})
add_library(nodeRpcProxy STATIC ${nodeRpcProxy})
add_library(p2p STATIC ${CryptoNoteProtocol} ${p2p})
add_library(rpc STATIC ${rpc})
add_library(serialization STATIC ${serialization})
add_library(subWallets STATIC ${subWallets})
add_library(system STATIC ${system})
add_library(transfers STATIC ${transfers})
add_library(utilities STATIC ${utilities})
add_library(wallet STATIC ${wallet})
add_library(walletBackend STATIC ${walletBackend})
add_library(walletService STATIC ${service})

if(MSVC)
  set(DAEMON_SOURCES_OS
    binaryInfo/daemon.rc
  )
  set(ZED_WALLET_SOURCES_OS
    binaryInfo/zedwallet.rc
  )
  set(MINER_SOURCES_OS
    binaryInfo/miner.rc
  )
  set(PG_SOURCES_OS
    binaryInfo/service.rc
  )
  set(CT_SOURCES_OS
    binaryInfo/cryptotest.rc
  )
  set(WALLET_API_SOURCES_OS
    binaryInfo/walletapi.rc
  )
endif()

add_executable(cryptotest ${cryptoTest} ${CT_SOURCES_OS})
add_executable(miner ${miner} ${MINER_SOURCES_OS})
add_executable(service ${service} ${PG_SOURCES_OS})
add_executable(TurtleCoind ${TurtleCoind} ${DAEMON_SOURCES_OS})
add_executable(walletApi ${walletApi} ${WALLET_API_SOURCES_OS})
add_executable(zedwallet ${zedwallet} ${ZED_WALLET_SOURCES_OS})
add_executable(zedwallet++ ${zedwallet++} ${ZED_WALLET_SOURCES_OS})

if(MSVC)
    target_link_libraries(System ws2_32)
    target_link_libraries(TurtleCoind Rpcrt4)
    target_link_libraries(service Rpcrt4)
endif ()

# A bit of hackery so we don't have to do the if/else/ for every target that
# wants to use filesystem
add_library(__filesystem INTERFACE)

# Windows works out of the box
if (APPLE)
    target_link_libraries(__filesystem INTERFACE /usr/local/opt/llvm/lib/libc++fs.a)
elseif (UNIX)
    target_link_libraries(__filesystem INTERFACE stdc++fs)
endif()

if(MSVC)
	target_link_libraries(TurtleCoind system cryptoNoteCore rocksdb ${Boost_LIBRARIES})
else()
	target_link_libraries(TurtleCoind system cryptoNoteCore rocksdblib ${Boost_LIBRARIES})
endif()

# Add the dependencies we need
target_link_libraries(common __filesystem)
target_link_libraries(cryptoNoteCore common logging crypto p2p rpc http serialization system ${Boost_LIBRARIES})
target_link_libraries(cryptotest crypto common)
target_link_libraries(errors subWallets)
target_link_libraries(logging common)
target_link_libraries(miner cryptoNoteCore rpc system http crypto errors utilities)
target_link_libraries(nigel errors)
target_link_libraries(p2p cryptoNoteCore upnpc-static)
target_link_libraries(rpc p2p utilities)
target_link_libraries(service jsonRpcServer wallet mnemonics errors)
target_link_libraries(subWallets utilities cryptoNoteCore)
target_link_libraries(wallet nodeRpcProxy transfers cryptoNoteCore common ${Boost_LIBRARIES})
target_link_libraries(walletApi walletBackend)
target_link_libraries(walletBackend mnemonics cryptoNoteCore nigel cryptopp-static __filesystem Utilities subWallets)
target_link_libraries(walletService mnemonics)
target_link_libraries(zedwallet mnemonics wallet errors utilities)
target_link_libraries(zedwallet++ walletBackend)

# Add dependencies means we have to build the latter before we build the former
# In this case it's because we need to have the current version name rather
# than a cached one
add_dependencies(cryptotest version)
add_dependencies(miner version)
add_dependencies(jsonRpcServer version)
add_dependencies(p2p version)
add_dependencies(rpc version)
add_dependencies(service version)
add_dependencies(TurtleCoind version)
add_dependencies(walletApi version)
add_dependencies(walletService version)
add_dependencies(zedwallet version)

# Finally build the binaries
set_property(TARGET TurtleCoind PROPERTY OUTPUT_NAME "TurtleCoind")
set_property(TARGET zedwallet PROPERTY OUTPUT_NAME "zedwallet")
set_property(TARGET zedwallet++ PROPERTY OUTPUT_NAME "zedwallet-beta")
set_property(TARGET service PROPERTY OUTPUT_NAME "turtle-service")
set_property(TARGET miner PROPERTY OUTPUT_NAME "miner")
set_property(TARGET cryptotest PROPERTY OUTPUT_NAME "cryptotest")
set_property(TARGET walletApi PROPERTY OUTPUT_NAME "wallet-api")

# Additional make targets, can be used to build a subset of the targets
# e.g. make pool will build only TurtleCoind and service
add_custom_target(pool DEPENDS TurtleCoind service)
add_custom_target(solominer DEPENDS TurtleCoind zedwallet miner)
add_custom_target(cli DEPENDS TurtleCoind zedwallet)
