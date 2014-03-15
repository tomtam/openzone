/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file client/openzone.cc
 */

#include <client/Client.hh>

#include <SDL.h>

#if defined( __native_client__ )
# include <SDL_nacl.h>
#elif defined( _WIN32 )
extern "C"
int SDL_main( int argc, char **argv );
#endif

using namespace oz;

static void crashHandler()
{
#if SDL_MAJOR_VERSION < 2
  SDL_WM_GrabInput( SDL_GRAB_OFF );
#endif
  SDL_Quit();
}

#if defined( __ANDROID__ )
int javaMain( int argc, char** argv )
#elif defined( __native_client__ )
int naclMain( int argc, char** argv )
#elif defined( _WIN32 )
int SDL_main( int argc, char** argv )
#else
int main( int argc, char** argv )
#endif
{
  System::init( System::DEFAULT_MASK, &crashHandler );

  int exitCode = EXIT_FAILURE;

  Log::printRaw( "OpenZone " OZ_VERSION "\n"
                 "Copyright © 2002-2014 Davorin Učakar\n"
                 "This program comes with ABSOLUTELY NO WARRANTY.\n"
                 "This is free software, and you are welcome to redistribute it\n"
                 "under certain conditions; See COPYING file for details.\n\n" );

#ifdef __native_client__
  Pepper::post( "init:" );
#endif

  exitCode = client::client.init( argc, argv );

  if( exitCode == EXIT_SUCCESS ) {
    exitCode = client::client.main();
  }

  client::client.shutdown();

  if( Alloc::count != 0 ) {
    Log::verboseMode = true;
    bool isOutput = Log::printMemoryLeaks();
    Log::verboseMode = false;

    if( isOutput ) {
      Log::println( "There are some memory leaks. See '%s' for details.", Log::logFile() );
    }
  }

#ifdef __native_client__
  Pepper::post( "quit:" );
#endif

  return exitCode;
}

OZ_JAVA_ENTRY_POINT( Java_com_github_ducakar_openzone_SDLActivity_nativeInit )
OZ_NACL_ENTRY_POINT()
