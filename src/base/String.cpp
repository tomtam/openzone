/*
 *  String.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "base.h"

#include <stdio.h>

namespace oz
{

  String::String( float f ) : buffer( baseBuffer )
  {
    // %g gives 6-digit precision + sign + dot + exponent (worst case e-xx) + terminating null char
    // 13 chars should be enough
    assert( BUFFER_SIZE >= 13 );

    count = snprintf( buffer, BUFFER_SIZE, "%#g", f );
  }

  String::String( double d ) : buffer( baseBuffer )
  {
    // %g gives 6-digit precision + sign + dot + exponent (worst case e-xxx) + terminating null char
    // 14 chars should be enough
    assert( BUFFER_SIZE >= 14 );

    count = snprintf( buffer, BUFFER_SIZE, "%#g", d );
  }

}
