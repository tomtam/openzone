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
 * @file oz/Semaphore.cc
 */

#include "Semaphore.hh"

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

struct SemaphoreDesc
{
  HANDLE        semaphore;
  volatile long counter;
};

#else

struct SemaphoreDesc
{
  pthread_mutex_t mutex;
  pthread_cond_t  cond;
  volatile int    counter;
};

#endif

int Semaphore::counter() const
{
  hard_assert( descriptor != null );

  return descriptor->counter;
}

void Semaphore::post() const
{
  hard_assert( descriptor != null );

#ifdef _WIN32

  InterlockedIncrement( &descriptor->counter );
  ReleaseSemaphore( descriptor->semaphore, 1, null );

#else

  pthread_mutex_lock( &descriptor->mutex );
  ++descriptor->counter;
  pthread_mutex_unlock( &descriptor->mutex );
  pthread_cond_signal( &descriptor->cond );

#endif
}

void Semaphore::wait() const
{
  hard_assert( descriptor != null );

#ifdef _WIN32

  WaitForSingleObject( descriptor->semaphore, INFINITE );
  InterlockedDecrement( &descriptor->counter );

#else

  pthread_mutex_lock( &descriptor->mutex );
  while( descriptor->counter == 0 ) {
    pthread_cond_wait( &descriptor->cond, &descriptor->mutex );
  }
  --descriptor->counter;
  pthread_mutex_unlock( &descriptor->mutex );

#endif
}

bool Semaphore::tryWait() const
{
  hard_assert( descriptor != null );

#ifdef _WIN32

  int ret = WaitForSingleObject( descriptor->semaphore, 0 );
  if( ret == WAIT_TIMEOUT ) {
    return false;
  }

  InterlockedDecrement( &descriptor->counter );
  return true;

#else

  bool hasSucceeded = false;

  pthread_mutex_lock( &descriptor->mutex );
  if( descriptor->counter != 0 ) {
    --descriptor->counter;
    hasSucceeded = true;
  }
  pthread_mutex_unlock( &descriptor->mutex );

  return hasSucceeded;

#endif
}

void Semaphore::init( int counter )
{
  hard_assert( descriptor == null && counter >= 0 );

  descriptor = static_cast<SemaphoreDesc*>( malloc( sizeof( SemaphoreDesc ) ) );
  if( descriptor == null ) {
    throw Exception( "Semaphore resource allocation failed" );
  }

  descriptor->counter = counter;

#ifdef _WIN32

  descriptor->semaphore = CreateSemaphore( null, counter, 0x7fffffff, null );
  if( descriptor->semaphore == null ) {
    free( descriptor );
    throw Exception( "Semaphore semaphore creation failed" );
  }

#else

  if( pthread_mutex_init( &descriptor->mutex, null ) != 0 ) {
    free( descriptor );
    throw Exception( "Semaphore mutex creation failed" );
  }
  if( pthread_cond_init( &descriptor->cond, null ) != 0 ) {
    pthread_mutex_destroy( &descriptor->mutex );
    free( descriptor );
    throw Exception( "Semaphore condition variable creation failed" );
  }

#endif
}

void Semaphore::destroy()
{
  hard_assert( descriptor != null );

#ifdef _WIN32
  CloseHandle( &descriptor->semaphore );
#else
  pthread_cond_destroy( &descriptor->cond );
  pthread_mutex_destroy( &descriptor->mutex );
#endif

  free( descriptor );
  descriptor = null;
}

}