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
 * @file ozCore/Buffer.cc
 */

#include "Buffer.hh"

#include <zlib.h>

namespace oz
{

Buffer::Buffer( int size_ ) :
  data( size_ == 0 ? nullptr : new char[size_] ), size( size_ )
{}

Buffer::Buffer( const char* data_, int size_ ) :
  data( size_ == 0 ? nullptr : new char[size_] ), size( size_ )
{
  mCopy( data, data_, size_t( size ) );
}

Buffer::Buffer( const String& s ) :
  data( s.length() == 0 ? nullptr : new char[ s.length() ] ), size( s.length() )
{
  mCopy( data, s.cstr(), size_t( size ) );
}

Buffer::~Buffer()
{
  deallocate();
}

Buffer::Buffer( const Buffer& b ) :
  data( nullptr ), size( b.size )
{
  if( b.size != 0 ) {
    data = new char[size];
    mCopy( data, b.data, size_t( size ) );
  }
}

Buffer::Buffer( Buffer&& b ) :
  data( b.data ), size( b.size )
{
  b.data = nullptr;
  b.size = 0;
}

Buffer& Buffer::operator = ( const Buffer& b )
{
  if( &b == this ) {
    return *this;
  }

  if( size < b.size ) {
    deallocate();
    allocate( b.size );
  }

  mCopy( data, b.data, size_t( b.size ) );
  return *this;
}

Buffer& Buffer::operator = ( Buffer&& b )
{
  if( &b == this ) {
    return *this;
  }

  delete[] data;

  data = b.data;
  size = b.size;

  b.data = nullptr;
  b.size = 0;

  return *this;
}

String Buffer::toString() const
{
  char*  buffer;
  String s = String::create( size, &buffer );

  mCopy( buffer, data, size_t( size ) );
  return s;
}

Buffer Buffer::deflate( int level ) const
{
  Buffer buffer;

  z_stream zstream;
  zstream.zalloc = nullptr;
  zstream.zfree  = nullptr;
  zstream.opaque = nullptr;

  if( deflateInit( &zstream, level ) != Z_OK ) {
    return buffer;
  }

  // Upper bound for deflated data plus sizeof( int ) to write down size of the uncompressed data.
  int newSize = int( deflateBound( &zstream, ulong( size ) ) ) + 4;
  buffer.allocate( newSize );

  zstream.next_in   = reinterpret_cast<ubyte*>( const_cast<char*>( data ) );
  zstream.avail_in  = uint( size );
  zstream.next_out  = reinterpret_cast<ubyte*>( buffer.begin() );
  zstream.avail_out = uint( newSize );

  int ret = ::deflate( &zstream, Z_FINISH );
  deflateEnd( &zstream );

  if( ret != Z_STREAM_END ) {
    buffer.deallocate();
  }
  else if( zstream.total_out + 4 != uint( newSize ) ) {
    buffer.resize( int( zstream.total_out + 4 ) );

    // Write size of the original data, ensure portability between little and big endian platforms.
#if OZ_BYTE_ORDER == 4321
    reinterpret_cast<int*>( buffer.end() )[-1] = Endian::bswap32( size );
#else
    reinterpret_cast<int*>( buffer.end() )[-1] = size;
#endif
  }
  return buffer;
}

Buffer Buffer::inflate() const
{
  Buffer buffer;

  z_stream zstream;
  zstream.zalloc = nullptr;
  zstream.zfree  = nullptr;
  zstream.opaque = nullptr;

  if( inflateInit( &zstream ) != Z_OK ) {
    return buffer;
  }

#if OZ_BYTE_ORDER == 4321
  int newSize = Endian::bswap32( reinterpret_cast<int*>( data + size )[-1] );
#else
  int newSize = reinterpret_cast<int*>( data + size )[-1];
#endif

  buffer.allocate( newSize );

  zstream.next_in   = reinterpret_cast<ubyte*>( const_cast<char*>( data ) );
  zstream.avail_in  = uint( size - 4 );
  zstream.next_out  = reinterpret_cast<ubyte*>( buffer.begin() );
  zstream.avail_out = uint( newSize );

  int ret = ::inflate( &zstream, Z_FINISH );
  inflateEnd( &zstream );

  if( ret != Z_STREAM_END ) {
    buffer.deallocate();
  }
  return buffer;
}

void Buffer::resize( int newSize )
{
  hard_assert( newSize >= 0 );

  char* newData = nullptr;

  if( newSize != 0 ) {
    newData = new char[newSize];
    mCopy( newData, data, size_t( min( newSize, size ) ) );
  }
  delete[] data;

  data = newData;
  size = newSize;
}

void Buffer::allocate( int newSize )
{
  hard_assert( size == 0 && newSize > 0 );

  data = new char[newSize];
  size = newSize;
}

void Buffer::deallocate()
{
  delete[] data;

  data = nullptr;
  size = 0;
}

InputStream Buffer::inputStream( Endian::Order order ) const
{
  return InputStream( data, data + size, order );
}

OutputStream Buffer::outputStream( Endian::Order order )
{
  return OutputStream( data, data + size, order );
}

}