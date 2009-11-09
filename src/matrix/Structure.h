/*
 *  Structure.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#pragma once

#include "matrix.h"

namespace oz
{

  struct Structure : Bounds
  {
    static const int CUT_BIT = 0x00000001;

    enum Rotation
    {
      R0   = 0,
      R90  = 1,
      R180 = 2,
      R270 = 3
    };

    Vec3     p;
    int      index;
    int      flags;
    int      bsp;
    Rotation rot;
    float    life;

    explicit Structure() {}

    explicit Structure( const Vec3 &p_, int bsp_, Rotation rot_ ) :
        p( p_ ), flags( 0 ), bsp( bsp_ ), rot( rot_ )
    {}

    void readFull( InputStream *istream );
    void writeFull( OutputStream *ostream );
  };

}
