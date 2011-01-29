/*
 *  Button.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "ui/Button.hpp"

#include <SDL_opengl.h>

namespace oz
{
namespace client
{
namespace ui
{

  bool Button::onMouseEvent()
  {
    isHighlighted = true;

    if( mouse.leftClick ) {
      isClicked = true;

      if( callback != null ) {
        callback( this );
      }
    }
    return true;
  }

  void Button::onDraw()
  {
    if( isClicked ) {
      glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    }
    else if( isHighlighted ) {
      glColor4f( 0.8f, 0.8f, 0.8f, 0.4f );
    }
    else {
      glColor4f( 0.6f, 0.6f, 0.6f, 0.4f );
    }
    fill( 0, 0, width, height );
    setFont( SANS );
    setFontColour( 200, 200, 200 );
    printCentred( width / 2, height / 2, "%s", label.cstr() );

    isHighlighted = false;
    isClicked = false;
  }

  Button::Button( const char* label_, Callback* callback, int width, int height ) :
      Area( width, height ), callback( callback ), isHighlighted( false ), isClicked( false ),
      label( label_ )
  {}

  void Button::setCallback( Callback* callback_ )
  {
    callback = callback_;
  }

}
}
}
