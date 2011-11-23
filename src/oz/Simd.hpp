/*
 * OpenZone - Simple Cross-Platform FPS/RTS Game Engine
 * Copyright (C) 2002-2011  Davorin Učakar
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
 *
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
 */

/**
 * @file oz/Simd.hpp
 */

#include "common.hpp"

#ifdef OZ_SIMD

namespace oz
{

/**
 * SIMD vector of four integers.
 *
 * @ingroup oz
 */
typedef int __attribute__(( vector_size( 16 ) )) int4;

/**
 * SIMD vector of four unsigned integers.
 *
 * @ingroup oz
 */
typedef uint __attribute__(( vector_size( 16 ) )) uint4;

/**
 * SIMD vector of four floats.
 *
 * @ingroup oz
 */
typedef float __attribute__(( vector_size( 16 ) )) float4;

/**
 * @def int4
 * "Constructor" for <tt>int4</tt> type.
 *
 * @ingroup oz
 */
# define int4( x, y, z, w ) (int4) { x, y, z, w }

/**
 * @def uint4
 * "Constructor" for <tt>uint4</tt> type.
 *
 * @ingroup oz
 */
# define uint4( x, y, z, w ) (uint4) { x, y, z, w }

/**
 * @def float4
 * "Constructor" for <tt>float4</tt> type.
 *
 * @ingroup oz
 */
# define float4( x, y, z, w ) (float4) { x, y, z, w }

/**
 * Base class for classes representing a SIMD register.
 *
 * @ingroup oz
 */
struct Simd
{
  union
  {
    int4   i4;   ///< Integer SIMD vector.
    int    i[4]; ///< Integer components of SIMD vector.

    uint4  u4;   ///< Unsigned integer SIMD vector.
    uint   u[4]; ///< Unsigned integer component of SIMD vector.

    float4 f4;   ///< Float SIMD vector.
    float  f[4]; ///< Float components of SIMD vector.

    /**
     * %Vector components.
     */
    struct
    {
      float x; ///< X component.
      float y; ///< Y component.
      float z; ///< Z component.
      float w; ///< W component.
    };

    /**
     * Plane components.
     */
    struct
    {
      float nx; ///< X component of the normal.
      float ny; ///< Y component of the normal.
      float nz; ///< Z component of the normal.
      float d;  ///< Distance from origin.
    };
  };

  /**
   * Create an uninitialised instance.
   */
  Simd() = default;

  /**
   * Create from an int SIMD vector.
   */
  OZ_ALWAYS_INLINE
  Simd( int4 i4_ ) : i4( i4_ )
  {}

  /**
   * Create from an uint SIMD vector.
   */
  OZ_ALWAYS_INLINE
  Simd( uint4 u4_ ) : u4( u4_ )
  {}

  /**
   * Create from a float SIMD vector.
   */
  OZ_ALWAYS_INLINE
  Simd( float4 f4_ ) : f4( f4_ )
  {}
};

}

#endif