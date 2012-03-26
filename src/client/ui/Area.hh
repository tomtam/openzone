/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 */

/**
 * @file client/ui/Area.hh
 */

#pragma once

#include "client/ui/Font.hh"
#include "client/ui/Mouse.hh"

namespace oz
{
namespace client
{
namespace ui
{

class Area
{
  friend class DList<Area>;
  friend class UI;

  public:

    // if onUpdate should be called
    static const int UPDATE_BIT      = 0x01;
    // ignore when passing events
    static const int IGNORE_BIT      = 0x02;
    // a child with GRAB_BIT get exclusive focus for events
    static const int GRAB_BIT        = 0x04;
    // do not draw
    static const int HIDDEN_BIT      = 0x08;
    // pin (show in both freelook and interface mode)
    static const int PINNED_BIT      = 0x10;

    // text alignment
    static const int ALIGN_LEFT      = 0x00;
    static const int ALIGN_RIGHT     = 0x01;
    static const int ALIGN_HCENTRE   = 0x02;
    static const int ALIGN_BOTTOM    = 0x00;
    static const int ALIGN_TOP       = 0x04;
    static const int ALIGN_VCENTRE   = 0x08;
    static const int ALIGN_NONE      = ALIGN_LEFT | ALIGN_BOTTOM;
    static const int ALIGN_CENTRE    = ALIGN_HCENTRE | ALIGN_VCENTRE;

  protected:

    static Vector<Area*> updateAreas;

    Area* prev[1];
    Area* next[1];

    int flags;

  public:

    static float uiScaleX;
    static float uiScaleY;
    static int   uiWidth;
    static int   uiHeight;
    static int   uiCentreX;
    static int   uiCentreY;

    Area* parent;
    DList<Area> children;

    // absolute x and y, not relative to parent
    int x;
    int y;
    int width;
    int height;

  protected:

    void fill( int x, int y, int width, int height ) const;
    void rect( int x, int y, int width, int height ) const;

    // return true if event has been caught
    bool passMouseEvents();
    void drawChildren();

    virtual void onVisibilityChange();
    // return true if event has been caught
    virtual bool onMouseEvent();
    virtual void onUpdate();
    virtual void onDraw();

    static void update();

  public:

    explicit Area( int width, int height );
    explicit Area( int x, int y, int width, int height );
    virtual ~Area();

    void realign( int newX, int newY );
    void move( int moveX, int moveY );

    void show( bool doShow );

    void add( Area* area, int relativeX, int relativeY );
    void add( Area* area );
    void remove( Area* area );
    void raise();
    void sink();

};

}
}
}
