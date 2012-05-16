set( NACL ON )

set( PLATFORM_TRIPLET           "x86_64-nacl" )
set( PLATFORM_PREFIX            "/home/davorin/Projects/nacl_sdk/pepper_19/toolchain/linux_x86_glibc" )

set( CMAKE_SYSTEM_NAME          "Linux" )
set( CMAKE_SYSTEM_PROCESSOR     "x86_64" )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PREFIX}/${PLATFORM_TRIPLET}" )
set( CMAKE_C_COMPILER           "${PLATFORM_PREFIX}/bin/${PLATFORM_TRIPLET}-gcc" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_PREFIX}/bin/${PLATFORM_TRIPLET}-g++" )
set( CMAKE_STRIP                "${PLATFORM_PREFIX}/bin/${PLATFORM_TRIPLET}-strip" )
