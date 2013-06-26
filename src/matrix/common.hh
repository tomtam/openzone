/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 * @file matrix/common.hh
 *
 * Common matrix includes and definitions for matrix.
 */

#pragma once

#include <common/common.hh>
#include <common/Point.hh>
#include <common/AABB.hh>
#include <common/Bounds.hh>

namespace oz
{
/**
 * Matrix layer.
 */
namespace matrix
{

using namespace common;

using common::Point;
using common::AABB;
using common::Bounds;

/**
 * Discrete heading for structures and objects.
 */
enum Heading
{
  NORTH = 0x00, ///< Y+.
  WEST  = 0x01, ///< X-.
  SOUTH = 0x02, ///< Y-.
  EAST  = 0x03  ///< X+.
};

/// Bitwise AND with this mask is non-zero iff heading is either `EAST` or `WEST`.
const int WEST_EAST_MASK = 0x01;

}
}
