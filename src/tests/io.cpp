/*
 *  io.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "matrix/io.hpp"

#include <SDL_main.h>

using namespace oz;

int main( int, char** )
{
  Buffer buffer = Buffer( 1024 );
  OutputStream os = buffer.outputStream();
  InputStream is = buffer.inputStream();

  os.writeVec3( Vec3( 1.5f, 0.7f, 4.9f ) );
  os.writeInt( 42 );
  os.writeFloat( 1.7f );
  os.writeString( "drek" );
  os.writeInt( 43 );
  os.writeVec3( Vec3( 1.51f, 0.71f, 4.91f ) );
  os.writeFloat( 1.71f );

  assert( is.readVec3() == Vec3( 1.5f, 0.7f, 4.9f ) );
  assert( is.readInt() == 42 );
  assert( is.readFloat() == 1.7f );
  assert( is.readString().equals( "drek" ) );
  assert( is.readInt() == 43 );
  assert( is.readVec3() == Vec3( 1.51f, 0.71f, 4.91f ) );
  assert( is.readFloat() == 1.71f );

  printf( "OK\n" );
  return 0;
}
