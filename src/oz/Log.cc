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
 * @file oz/Log.cc
 */

#include "Log.hh"

#include "arrays.hh"
#include "Time.hh"
#include "Exception.hh"

#include "windefs.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __native_client__

# include "System.hh"
# include "Semaphore.hh"

# include <ppapi/cpp/completion_callback.h>
# include <ppapi/cpp/instance.h>
# include <ppapi/cpp/core.h>
# include <ppapi/cpp/var.h>

# define CHECK_SEMAPHORE() \
  if( !isSemaphoreValid ) { \
    pthread_mutex_init( &semaphoreMutex, null ); \
    pthread_cond_init( &semaphoreCond, null ); \
    isSemaphoreValid = true; \
  }

# define CONSOLE_PUTS( s ) \
  if( System::core->IsMainThread() ) { \
    System::instance->PostMessage( pp::Var( s ) ); \
  } \
  else { \
    struct _Callback \
    { \
      static void _main( void* data, int ) \
      { \
        System::instance->PostMessage( pp::Var( static_cast<const char*>( data ) ) ); \
        pthread_mutex_lock( &semaphoreMutex ); \
        ++semaphoreCounter; \
        pthread_mutex_unlock( &semaphoreMutex ); \
        pthread_cond_signal( &semaphoreCond ); \
      } \
    }; \
    System::core->CallOnMainThread( 0, pp::CompletionCallback( _Callback::_main, \
                                                               const_cast<char*>( s ) ) ); \
    pthread_mutex_lock( &semaphoreMutex ); \
    while( semaphoreCounter == 0 ) { \
      pthread_cond_wait( &semaphoreCond, &semaphoreMutex ); \
    } \
    --semaphoreCounter; \
    pthread_mutex_unlock( &semaphoreMutex ); \
  }

#endif

namespace oz
{

static const char* const SIGNALS[][2] =
{
  { "SIG???",    "[invalid signal number]"    },
  { "SIGHUP",    "Hangup"                     }, //  1
  { "SIGINT",    "Interrupt"                  }, //  2
  { "SIGQUIT",   "Quit"                       }, //  3
  { "SIGILL",    "Illegal instruction"        }, //  4
  { "SIGTRAP",   "Trace trap"                 }, //  5
  { "SIGABRT",   "Abort"                      }, //  6
  { "SIGBUS",    "BUS error"                  }, //  7
  { "SIGFPE",    "Floating-point exception"   }, //  8
  { "SIGKILL",   "Kill, unblockable"          }, //  9
  { "SIGUSR1",   "User-defined signal 1"      }, // 10
  { "SIGSEGV",   "Segmentation violation"     }, // 11
  { "SIGUSR2",   "User-defined signal 2"      }, // 12
  { "SIGPIPE",   "Broken pipe"                }, // 13
  { "SIGALRM",   "Alarm clock"                }, // 14
  { "SIGTERM",   "Termination"                }, // 15
  { "SIGSTKFLT", "Stack fault"                }, // 16
  { "SIGCHLD",   "Child status has changed"   }, // 17
  { "SIGCONT",   "Continue"                   }, // 18
  { "SIGSTOP",   "Stop, unblockable"          }, // 19
  { "SIGTSTP",   "Keyboard stop"              }, // 20
  { "SIGTTIN",   "Background read from tty"   }, // 21
  { "SIGTTOU",   "Background write to tty"    }, // 22
  { "SIGURG",    "Urgent condition on socket" }, // 23
  { "SIGXCPU",   "CPU limit exceeded"         }, // 24
  { "SIGXFSZ",   "File size limit exceeded"   }, // 25
  { "SIGVTALRM", "Virtual alarm clock"        }, // 26
  { "SIGPROF",   "Profiling alarm clock"      }, // 27
  { "SIGWINCH",  "Window size change"         }, // 28
  { "SIGIO",     "I/O now possible"           }, // 29
  { "SIGPWR",    "Power failure restart"      }, // 30
  { "SIGSYS",    "Bad system call"            }  // 31
};

static char  filePath[256]; // = ""
static FILE* file;          // = null
static int   tabs;          // = 0

#ifdef __native_client__
static bool            isSemaphoreValid;
static pthread_mutex_t semaphoreMutex;
static pthread_cond_t  semaphoreCond;
static volatile int    semaphoreCounter;
#endif

bool Log::showVerbose; // = false
bool Log::verboseMode; // = false

const char* Log::logFile()
{
  return filePath;
}

void Log::resetIndent()
{
  tabs = 0;
}

void Log::indent()
{
  ++tabs;
}

void Log::unindent()
{
  if( tabs > 0 ) {
    --tabs;
  }
}

void Log::vprintRaw( const char* s, va_list ap )
{
  char buffer[OUT_BUFFER_SIZE];

  vsnprintf( buffer, OUT_BUFFER_SIZE, s, ap );

  if( !verboseMode || showVerbose || file == null ) {
    fputs( buffer, stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( buffer );
#else
  if( file != null ) {
    fputs( buffer, file );
    fflush( file );
  }
#endif
}

void Log::printRaw( const char* s, ... )
{
  char buffer[OUT_BUFFER_SIZE];

  va_list ap;
  va_start( ap, s );
  vsnprintf( buffer, OUT_BUFFER_SIZE, s, ap );
  va_end( ap );

  if( !verboseMode || showVerbose || file == null ) {
    fputs( buffer, stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( buffer );
#else
  if( file != null ) {
    fputs( buffer, file );
    fflush( file );
  }
#endif
}

void Log::print( const char* s, ... )
{
  char buffer[OUT_BUFFER_SIZE];

  va_list ap;
  va_start( ap, s );
  vsnprintf( buffer, OUT_BUFFER_SIZE, s, ap );
  va_end( ap );

  if( !verboseMode || showVerbose || file == null ) {
    for( int i = 0; i < tabs; ++i ) {
      fputs( "  ", stdout );
    }
    fputs( buffer, stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  for( int i = 0; i < tabs; ++i ) {
    CONSOLE_PUTS( "  " );
  }
  CONSOLE_PUTS( buffer );
#else
  if( file != null ) {
    for( int i = 0; i < tabs; ++i ) {
      fputs( "  ", file );
    }
    fputs( buffer, file );
    fflush( file );
  }
#endif
}

void Log::printEnd( const char* s, ... )
{
  char buffer[OUT_BUFFER_SIZE];

  va_list ap;
  va_start( ap, s );
  vsnprintf( buffer, OUT_BUFFER_SIZE, s, ap );
  va_end( ap );

  if( !verboseMode || showVerbose || file == null ) {
    fputs( buffer, stdout );
    fputc( '\n', stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( buffer );
  CONSOLE_PUTS( "\n" );
#else
  if( file != null ) {
    fputs( buffer, file );
    fputc( '\n', file );
    fflush( file );
  }
#endif
}

void Log::printEnd()
{
  if( !verboseMode || showVerbose || file == null ) {
    fputc( '\n', stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( "\n" );
#else
  if( file != null ) {
    fputc( '\n', file );
    fflush( file );
  }
#endif
}

void Log::println( const char* s, ... )
{
  char buffer[OUT_BUFFER_SIZE];

  va_list ap;
  va_start( ap, s );
  vsnprintf( buffer, OUT_BUFFER_SIZE, s, ap );
  va_end( ap );

  if( !verboseMode || showVerbose || file == null ) {
    for( int i = 0; i < tabs; ++i ) {
      fputs( "  ", stdout );
    }
    fputs( buffer, stdout );
    fputc( '\n', stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  for( int i = 0; i < tabs; ++i ) {
    CONSOLE_PUTS( "  " );
  }
  CONSOLE_PUTS( buffer );
  CONSOLE_PUTS( "\n" );
#else
  if( file != null ) {
    for( int i = 0; i < tabs; ++i ) {
      fputs( "  ", file );
    }
    fputs( buffer, file );
    fputc( '\n', file );
    fflush( file );
  }
#endif
}

void Log::println()
{
  if( !verboseMode || showVerbose || file == null ) {
    fputc( '\n', stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( "\n" );
#else
  if( file != null ) {
    fputc( '\n', file );
    fflush( file );
  }
#endif
}

void Log::printTime( const Time& time )
{
  char buffer[OUT_BUFFER_SIZE];

  snprintf( buffer, OUT_BUFFER_SIZE, "%04d-%02d-%02d %02d:%02d:%02d",
            time.year, time.month, time.day, time.hour, time.minute, time.second );

  if( !verboseMode || showVerbose || file == null ) {
    fputs( buffer, stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( buffer );
#else
  if( file != null ) {
    fputs( buffer, file );
    fflush( file );
  }
#endif
}

void Log::printTrace( const StackTrace& st )
{
  if( st.nFrames == 0 ) {
    if( !verboseMode || showVerbose || file == null ) {
      fputs( "    [no stack trace]\n", stdout );
    }
#ifdef __native_client__
    CHECK_SEMAPHORE();

    CONSOLE_PUTS( "    [no stack trace]\n" );
#else
    if( file != null ) {
      fputs( "    [no stack trace]\n", file );
    }
#endif
  }
  else {
    char** entries = st.symbols();

#ifdef __native_client__
    CHECK_SEMAPHORE();
#endif

    for( int i = 0; i < st.nFrames; ++i ) {
      if( !verboseMode || showVerbose || file == null ) {
        fputs( "    ", stdout );
        fputs( entries[i], stdout );
        fputc( '\n', stdout );
      }

#ifdef __native_client__
      CONSOLE_PUTS( "    " );
      CONSOLE_PUTS( entries[i] );
      CONSOLE_PUTS( "\n" );
#else
      if( file != null ) {
        fputs( "    ", file );
        fputs( entries[i], file );
        fputc( '\n', file );
      }
#endif
    }

    ::free( entries );
  }

  if( file != null ) {
    fflush( file );
  }
}

void Log::printException( const std::exception& e )
{
  char buffer[OUT_BUFFER_SIZE];

  const Exception* oe = dynamic_cast<const Exception*>( &e );

  if( oe == null ) {
    snprintf( buffer, OUT_BUFFER_SIZE, "\n\nEXCEPTION: %s\n", e.what() );

    if( !verboseMode || showVerbose || file == null ) {
      fputs( buffer, stdout );
    }
#ifdef __native_client__
    CHECK_SEMAPHORE();

    CONSOLE_PUTS( buffer );
#else
    if( file != null ) {
      fputs( buffer, file );
      fflush( file );
    }
#endif
  }
  else {
    snprintf( buffer, OUT_BUFFER_SIZE, "\n\nEXCEPTION: %s\n  in %s\n  at %s:%d\n  stack trace:\n",
              oe->message, oe->function, oe->file, oe->line );

    if( !verboseMode || showVerbose || file == null ) {
      fputs( buffer, stdout );
    }
#ifdef __native_client__
    CHECK_SEMAPHORE();

    CONSOLE_PUTS( buffer );
#else
    if( file != null ) {
      fputs( buffer, file );
    }
#endif

    printTrace( oe->stackTrace );
  }
}

void Log::printSignal( int sigNum )
{
  char buffer[OUT_BUFFER_SIZE];

  int index = uint( sigNum ) >= uint( aLength( SIGNALS ) ) ? 0 : sigNum;

  snprintf( buffer, OUT_BUFFER_SIZE, "\n\nCaught signal %d %s (%s)\n",
            sigNum, SIGNALS[index][0], SIGNALS[index][1] );

  if( !verboseMode || showVerbose || file == null ) {
    fputs( buffer, stdout );
  }

#ifdef __native_client__
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( buffer );
#else
  if( file != null ) {
    fputs( buffer, file );
    fflush( file );
  }
#endif
}

void Log::printHalt()
{
#if defined( __native_client__ ) || defined( __ANDROID__ )
  const char* message = "Program halted, debugger can be attached ...\n";
#else
  const char* message = "Attach a debugger or send a fatal signal (e.g. CTRL-C) to kill ...\n";
#endif

  fputs( message, stdout );

#if defined( __native_client__ )
  CHECK_SEMAPHORE();

  CONSOLE_PUTS( message );
#else
  if( file != null ) {
    fputs( message, file );
    fflush( file );
  }
#endif
}

bool Log::init( const char* filePath_, bool clearFile )
{
  tabs = 0;

#ifdef __native_client__

  static_cast<void>( filePath_ );
  static_cast<void>( clearFile );

  strcpy( filePath, "[JavaScript Messages]" );

  return System::instance != null && System::core != null;

#else

  if( filePath_ == null ) {
    filePath[0] = '\0';
  }
  else {
    strncpy( filePath, filePath_, 256 );
    filePath[255] = '\0';
  }

  if( file != null ) {
    fclose( file );
    file = null;
  }
  if( filePath[0] != '\0' ) {
    file = fopen( filePath, clearFile ? "w" : "a" );
  }

  return file != null;

#endif
}

void Log::free()
{
#ifndef __native_client__
  if( file != null ) {
    fclose( file );
    file = null;
  }
#endif
}

}
