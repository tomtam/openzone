/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 * @file tests/engine.cc
 */

#include <ozCore/ozCore.hh>
#include <ozEngine/ozEngine.hh>
#include <ozFactory/ozFactory.hh>
#include <SDL.h>
#include <AL/alc.h>

using namespace oz;

int main( int argc, char** argv )
{
  System::init();
  SDL_Init( SDL_INIT_VIDEO );
  Window::create( "Test", 100, 100, false );

  ALCdevice*  device  = alcOpenDevice( nullptr );
  ALCcontext* context = alcCreateContext( device, nullptr );
  alcMakeContextCurrent( context );

//   ALBuffer buffer( "/usr/share/sounds/Kopete_Received.ogg" );
  ALStreamingBuffer buffer( "/home/davorin/Glasba/Whatever.ogg" );
  ALSource source = buffer.createSource();

  hard_assert( source.id() != 0 );
//   alSourcePlay( source.id() );
  OZ_AL_CHECK_ERROR();

//   File dds( argc < 2 ? "mail.dds" : argv[1] );
//   GLTexture texture( dds );

  File file( argc < 2 ? "/usr/share/icons/OpenZone_Fire_Slim/cursors/half-busy" : argv[1] );
  Cursor cursor( file );

  if( !cursor.isLoaded() ) {
    return EXIT_FAILURE;
  }

  OutputStream ostream( 0 );
  Builder::buildDDS( File( "/home/davorin/Slike/users/jojo-64x64.png" ),
                     Builder::MIPMAPS_BIT | Builder::COMPRESSION_BIT, &ostream );
  File( "drek.dds" ).write( ostream.begin(), ostream.tell() );

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  bool isAlive = true;
  while( isAlive ) {
    SDL_Event event;
    SDL_PollEvent( &event );

    if( event.type == SDL_QUIT || event.type == SDL_KEYDOWN ) {
      isAlive = false;
    }

    glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, cursor.textureId() );

    glBegin( GL_QUADS );
      glTexCoord2i( 0, 1 ); glVertex2d( -1, +1 - 0.02 * cursor.height() );
      glTexCoord2i( 1, 1 ); glVertex2d( -1 + 0.02 * cursor.width(), +1 - 0.02 * cursor.height() );
      glTexCoord2i( 1, 0 ); glVertex2d( -1 + 0.02 * cursor.width(), +1 );
      glTexCoord2i( 0, 0 ); glVertex2d( -1, +1 );
    glEnd();

    Window::swapBuffers();
    cursor.advance( 15 );

//     isAlive &= buffer.update();
    Time::sleep( 10 );
  }

  source.destroy();
  buffer.destroy();

  OZ_AL_CHECK_ERROR();
  alcDestroyContext( context );
  alcCloseDevice( device );
  Window::destroy();
  SDL_Quit();

  List<String> fileNames;
  List<Buffer> fileContents;

//   Shader::readFile( "data/oz_base/glsl/mesh.vert", &fileNames, &fileContents );

  for( const Buffer& b : fileContents ) {
    Log() << "---------------\n";
    Log() << b.toString();
  }
  return 0;
}
