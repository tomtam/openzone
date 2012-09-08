set( NACL ON )

set( PLATFORM_TRIPLET           "i686-nacl" )
set( PLATFORM_PREFIX            "/home/davorin/Projects/nacl_sdk/pepper_21/toolchain/linux_x86_newlib" )

set( CMAKE_SYSTEM_NAME          "Linux" CACHE STRING "Target system." )
set( CMAKE_SYSTEM_PROCESSOR     "i686" CACHE STRING "Target processor." )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PREFIX}/${PLATFORM_TRIPLET}" )
set( CMAKE_C_COMPILER           "${PLATFORM_PREFIX}/bin/${PLATFORM_TRIPLET}-gcc" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_PREFIX}/bin/${PLATFORM_TRIPLET}-g++" )
set( CMAKE_STRIP                "${PLATFORM_PREFIX}/bin/${PLATFORM_TRIPLET}-strip" )

set( CMAKE_C_FLAGS              "-m32 -march=i686 -msse3 -mfpmath=sse" CACHE STRING "" )
set( CMAKE_CXX_FLAGS            "-m32 -march=i686 -msse3 -mfpmath=sse" CACHE STRING "" )

# CMake only looks into lib but not lib32.
set( PEPPER_LIBRARY             "${PLATFORM_PREFIX}/x86_64-nacl/lib32/libppapi.a" CACHE PATH "" )
set( PEPPER_CXX_LIBRARY         "${PLATFORM_PREFIX}/x86_64-nacl/lib32/libppapi_cpp.a" CACHE PATH "" )
set( PEPPER_GLES2_LIBRARY       "${PLATFORM_PREFIX}/x86_64-nacl/lib32/libppapi_gles2.a" CACHE PATH "" )
set( LUA_MATH_LIBRARY           "${PLATFORM_PREFIX}/x86_64-nacl/lib32/libm.a" CACHE PATH "" )
