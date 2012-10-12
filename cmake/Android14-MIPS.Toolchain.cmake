set( ANDROID ON )

set( PLATFORM_TRIPLET           "mipsel-linux-android" )
set( PLATFORM_PREFIX            "/opt/android-ndk/platforms/android-14/arch-mips/usr" )
set( PLATFORM_TOOL_PREFIX       "/opt/android-ndk/toolchains/mipsel-linux-android-4.6/prebuilt/linux-x86/bin" )
set( PLATFORM_SOURCE_PREFIX     "/opt/android-ndk/sources" )

set( CMAKE_SYSTEM_NAME          "Linux" CACHE STRING "Target system." )
set( CMAKE_SYSTEM_PROCESSOR     "MIPS" CACHE STRING "Target processor." )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PREFIX}" )
set( CMAKE_C_COMPILER           "${PLATFORM_TOOL_PREFIX}/${PLATFORM_TRIPLET}-gcc" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_TOOL_PREFIX}/${PLATFORM_TRIPLET}-g++" )

set( PLATFORM_FLAGS             "--sysroot=${PLATFORM_PREFIX} -std=c++0x" )
set( PLATFORM_FLAGS             "${PLATFORM_FLAGS} -I${PLATFORM_SOURCE_PREFIX}/cxx-stl/gnu-libstdc++/4.6/include" )
set( PLATFORM_FLAGS             "${PLATFORM_FLAGS} -I${PLATFORM_SOURCE_PREFIX}/cxx-stl/gnu-libstdc++/4.6/libs/x86/include" )

set( CMAKE_C_FLAGS              "${PLATFORM_FLAGS}" CACHE STRING "" )
set( CMAKE_CXX_FLAGS            "${PLATFORM_FLAGS}" CACHE STRING "" )
