/*
 * ozCore - OpenZone Core Library.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 * @file ozCore/common.hh
 *
 * Essential includes, macros, types and templates.
 *
 * You may want to add `soft_assert`, `hard_assert`, `byte`, `ubyte`, `ushort`, `ulong`, `long64`,
 * `ulong64`, `foreach`, `float4` and `uint4` to your `~/.kde/share/apps/katepart/syntax/cpp.xml` or
 * global `/usr/share/apps/katepart/syntax/cpp.xml` to look like reserved words in KatePart editors.
 */

#pragma once

#include <ozCore/config.hh>

#include <cstdarg>
#include <cstddef>
#include <new>

/**
 * @def OZ_ALIGNED
 * Compiler-specific attribute for data type alignment.
 */
#define OZ_ALIGNED( n ) __attribute__(( aligned( n ) ))

/**
 * @def OZ_ALWAYS_INLINE
 * Compiler-specific attribute that suggests function inlining even in debug mode.
 */
#define OZ_ALWAYS_INLINE __attribute__(( always_inline ))

/**
 * @def OZ_BYTE_ORDER
 * Byte order inside a 32-bit integer. 1234 for little endian and 4321 for big endian machines.
 */
#define OZ_BYTE_ORDER __BYTE_ORDER__

/**
 * @def OZ_HIDDEN
 * Compiler-specific attribute that prevents symbol exporting when building a shared library.
 */
#ifdef _WIN32
# define OZ_HIDDEN
#else
# define OZ_HIDDEN __attribute__(( visibility( "hidden" ) ))
#endif

/**
 * @def OZ_NORETURN
 * Compiler-specific attribute that marks a function as no-return.
 */
#define OZ_NORETURN __attribute__(( noreturn ))

/**
 * @def OZ_PRINTF_FORMAT
 * Compiler-specific attribute that specifies checking of printf-like arguments.
 */
#define OZ_PRINTF_FORMAT( s, first ) __attribute__(( format( printf, s, first ) ))

/**
 * @def OZ_WEAK
 * Compiler-specific attribute specifying a weak symbol.
 */
#ifdef _WIN32
# define OZ_WEAK
#else
# define OZ_WEAK __attribute(( weak ))
#endif

/**
 * @def soft_assert
 * If condition fails, raise SIGTRAP and print error using global log.
 */
#ifdef NDEBUG
# define soft_assert( cond ) void( 0 )
#else
# define soft_assert( cond ) \
  ( ( cond ) ? \
    void( 0 ) : oz::detail::softAssertHelper( __PRETTY_FUNCTION__, __FILE__, __LINE__, #cond ) )
#endif

/**
 * @def hard_assert
 * If condition fails, raise SIGTRAP, print error using global log and abort program.
 */
#ifdef NDEBUG
# define hard_assert( cond ) void( 0 )
#else
# define hard_assert( cond ) \
  ( ( cond ) ? \
    void( 0 ) : oz::detail::hardAssertHelper( __PRETTY_FUNCTION__, __FILE__, __LINE__, #cond ) )
#endif

/**
 * Top-level OpenZone namespace.
 */
namespace oz
{

// These three types are also defined afterwards (which is completely unnecessary) as a workaround
// for a QtCreator bug and to provide Doxygen documentation.
using std::nullptr_t;
using std::size_t;
using std::ptrdiff_t;

/**
 * Null pointer type.
 */
typedef std::nullptr_t nullptr_t;

/**
 * Platform-dependent unsigned integer type for memory offsets and sizes.
 */
typedef std::size_t size_t;

/**
 * Platform-dependent signed integer type for memory offsets and pointer differences.
 */
typedef std::ptrdiff_t ptrdiff_t;

/**
 * Signed byte.
 */
typedef signed char byte;

/**
 * Unsigned byte.
 */
typedef unsigned char ubyte;

/**
 * Unsigned short integer.
 */
typedef unsigned short ushort;

/**
 * Unsigned integer.
 */
typedef unsigned int uint;

/**
 * Unsigned long integer.
 */
typedef unsigned long ulong;

/**
 * Signed 64-bit integer.
 */
typedef long long long64;

/**
 * Unsigned 64-bit integer.
 */
typedef unsigned long long ulong64;

static_assert( sizeof( char   ) == 1, "sizeof( char ) should be 1" );
static_assert( sizeof( short  ) == 2, "sizeof( short ) should be 2" );
static_assert( sizeof( int    ) == 4, "sizeof( int ) should be 4" );
static_assert( sizeof( long64 ) == 8, "sizeof( long64 ) should be 8" );
static_assert( sizeof( float  ) == 4, "sizeof( float ) should be 4" );
static_assert( sizeof( double ) == 8, "sizeof( double ) should be 8" );

/**
 * Internal helpers.
 */
namespace detail
{

/**
 * Helper method for `soft_assert` macro.
 */
void softAssertHelper( const char* function, const char* file, int line, const char* message );

/**
 * Helper method for `hard_assert` macro.
 */
OZ_NORETURN
void hardAssertHelper( const char* function, const char* file, int line, const char* message );

}

/**
 * Swap values of variables.
 */
template <typename Value>
OZ_ALWAYS_INLINE
inline void swap( Value& a, Value& b )
{
  Value t = static_cast<Value&&>( a );
  a = static_cast<Value&&>( b );
  b = static_cast<Value&&>( t );
}

/**
 * Absolute value.
 */
template <typename Value>
OZ_ALWAYS_INLINE
inline Value abs( const Value& a )
{
  return a < 0 ? -a : a;
}

/**
 * `a` if `a <= b`, `b` otherwise.
 */
template <typename Value>
OZ_ALWAYS_INLINE
inline const Value& min( const Value& a, const Value& b )
{
  return b < a ? b : a;
}

/**
 * `a` if `a >= b`, `b` otherwise.
 */
template <typename Value>
OZ_ALWAYS_INLINE
inline const Value& max( const Value& a, const Value& b )
{
  return a < b ? b : a;
}

/**
 * `c` if `a <= c <= b`, respective boundary otherwise.
 */
template <typename Value>
OZ_ALWAYS_INLINE
inline const Value& clamp( const Value& c, const Value& a, const Value& b )
{
  hard_assert( !( b < a ) );

  return c < a ? a : ( b < c ? b : c );
}

/**
 * Hash function for integers, identity.
 */
OZ_ALWAYS_INLINE
inline int hash( int value )
{
  return value;
}

/**
 * Hash function for strings, implements Bernstein's hash.
 */
inline int hash( const char* s )
{
  int value = 5381;

  while( *s != '\0' ) {
    value = value * 33 + *s;
    ++s;
  }
  return value;
}

}