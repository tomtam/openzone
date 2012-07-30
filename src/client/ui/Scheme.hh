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
 * @file client/ui/Scheme.hh
 */

#pragma once

#include "client/common.hh"

namespace oz
{
namespace client
{
namespace ui
{

class Scheme
{
  public:

    Vec4   text;
    Vec4   textBackground;

    Vec4   button;
    Vec4   buttonHover;
    Vec4   buttonClicked;

    Vec4   tile;
    Vec4   tileHover;

    Vec4   frame;
    Vec4   background;

    Vec4   menuStrip;

  public:

    void init();
    void free();

};

extern Scheme scheme;

}
}
}