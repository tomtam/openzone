set( MINGW32 ON )

set( PLATFORM_TRIPLET           "i486-mingw32" )
set( PLATFORM_PREFIX            "/usr/${PLATFORM_TRIPLET}" )
set( PLATFORM_TOOL_PREFIX       "${PLATFORM_PREFIX}" )

set( CMAKE_SYSTEM_NAME          Windows )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PREFIX}" )
set( CMAKE_C_COMPILER           "${PLATFORM_TRIPLET}-gcc" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_TRIPLET}-g++" )
set( CMAKE_RC_COMPILER          "${PLATFORM_TRIPLET}-windres" )
set( CMAKE_STRIP                "${PLATFORM_TRIPLET}-strip" )
set( CMAKE_EXE_LINKER_FLAGS     "-Wl,--enable-auto-import" )

set( SDL_INCLUDE_DIR            "${CMAKE_FIND_ROOT_PATH}/include/SDL" CACHE PATH "" )
set( SDLTTF_INCLUDE_DIR         "${CMAKE_FIND_ROOT_PATH}/include/SDL" CACHE PATH "" )
set( OPENGL_INCLUDE_DIR         "./include" CACHE PATH "" )
set( OPENAL_INCLUDE_DIR         "${CMAKE_FIND_ROOT_PATH}/include" CACHE PATH "" )
set( FREEIMAGE_LIBRARY          "${CMAKE_FIND_ROOT_PATH}/lib/FreeImage.lib" CACHE PATH "" )

mark_as_advanced( SDL_INCLUDE_DIR )
mark_as_advanced( SDLTTF_INCLUDE_DIR )
mark_as_advanced( OPENGL_INCLUDE_DIR )
mark_as_advanced( OPENAL_INCLUDE_DIR )
mark_as_advanced( FREEIMAGE_LIBRARY )
