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
 * @file client/Colours.cc
 */

#include "stable.hh"

#include "client/Colours.hh"

namespace oz
{
namespace client
{

const Vec4 Colours::GLOBAL_AMBIENT = Vec4( 0.15f, 0.15f, 0.20f, 1.00f );
const Vec4 Colours::WATER = Vec4( 0.00f, 0.05f, 0.25f, 1.0f );
const Vec4 Colours::LAVA = Vec4( 0.30f, 0.20f, 0.00f, 1.0f );

Vec4 Colours::diffuse = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );
Vec4 Colours::ambient = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );
Vec4 Colours::caelum = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );
Vec4 Colours::liquid = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );

Vec4 Colours::waterBlend1 = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );
Vec4 Colours::waterBlend2 = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );

}
}
