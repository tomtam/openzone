set( ANDROID ON )

set( PLATFORM_TRIPLET           "i686-linux-android" )
set( PLATFORM_PREFIX            "/opt/android-ndk/platforms/android-14/arch-x86" )
set( PLATFORM_TOOL_PREFIX       "/opt/android-ndk/toolchains/x86-4.6/prebuilt/linux-x86/bin" )
set( PLATFORM_SOURCE_PREFIX     "/opt/android-ndk/sources" )
if( NOT PLATFORM_PORTS_PREFIX )
  set( PLATFORM_PORTS_PREFIX    "${CMAKE_SOURCE_DIR}/ports/Android14-i686" )
endif()

set( CMAKE_SYSTEM_NAME          "Linux" CACHE STRING "Target system." )
set( CMAKE_SYSTEM_PROCESSOR     "i686" CACHE STRING "Target processor." )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PORTS_PREFIX};${PLATFORM_PREFIX}" )
set( CMAKE_C_COMPILER           "${PLATFORM_TOOL_PREFIX}/${PLATFORM_TRIPLET}-gcc" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_TOOL_PREFIX}/${PLATFORM_TRIPLET}-g++" )

set( PLATFORM_FLAGS             "-fPIC --sysroot=${PLATFORM_PREFIX}" )
set( PLATFORM_FLAGS             "${PLATFORM_FLAGS} -I${PLATFORM_SOURCE_PREFIX}/cxx-stl/gnu-libstdc++/4.6/include" )
set( PLATFORM_FLAGS             "${PLATFORM_FLAGS} -I${PLATFORM_SOURCE_PREFIX}/cxx-stl/gnu-libstdc++/4.6/libs/x86/include" )
set( PLATFORM_STLLIB            "${PLATFORM_SOURCE_PREFIX}/cxx-stl/gnu-libstdc++/4.6/libs/x86/libgnustl_shared.so" )

set( CMAKE_C_FLAGS              "${PLATFORM_FLAGS} -msse3 -mfpmath=sse" CACHE STRING "" )
set( CMAKE_CXX_FLAGS            "${PLATFORM_FLAGS} -msse3 -mfpmath=sse" CACHE STRING "" )
