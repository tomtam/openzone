/*
 * liboz - OpenZone core library.
 *
 * Copyright © 2002-2012 Davorin Učakar
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file oz/Thread.cc
 */

#include "Thread.hh"

#include "Exception.hh"

#ifdef _WIN32
# include "windefs.h"
# include <cstdlib>
# include <windows.h>
#else
# include <cstdlib>
# include <pthread.h>
#endif

namespace oz
{

#ifdef _WIN32

struct ThreadDesc
{
  HANDLE thread;
};

static DWORD WINAPI winMain( void* data )
{
  Thread::Main* main = *reinterpret_cast<Thread::Main**>( &data );
  main();
  return 0;
}

#else

struct ThreadDesc
{
  pthread_t thread;
};

static void* pthreadMain( void* data )
{
  Thread::Main* main = *reinterpret_cast<Thread::Main**>( &data );
  main();
  return null;
}

#endif

void Thread::start( Main* main )
{
  hard_assert( descriptor == null );

  descriptor = static_cast<ThreadDesc*>( malloc( sizeof( ThreadDesc ) ) );
  if( descriptor == null ) {
    throw Exception( "Thread resource allocation failed" );
  }

#ifdef _WIN32
  descriptor->thread = CreateThread( null, 0, winMain, *reinterpret_cast<void**>( &main ), 0,
                                     null );
  if( descriptor->thread == null ) {
    throw Exception( "Thread creation failed" );
  }
#else
  if( pthread_create( &descriptor->thread, null, pthreadMain,
                      *reinterpret_cast<void**>( &main ) ) != 0 )
  {
    throw Exception( "Thread creation failed" );
  }
#endif
}

void Thread::join()
{
  hard_assert( descriptor != null );

#ifdef _WIN32
  WaitForSingleObject( descriptor->thread, INFINITE );
  CloseHandle( descriptor->thread );
#else
  if( pthread_join( descriptor->thread, null ) != 0 ) {
    free( descriptor );
    descriptor = null;
    throw Exception( "Thread join failed" );
  }
#endif

  free( descriptor );
  descriptor = null;
}

}