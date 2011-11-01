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
 * Davorin Učakar <davorin.ucakar@gmail.com>
 */

/**
 * @file oz/common.cpp
 */

#include "common.hpp"

#include "System.hpp"

#ifndef NDEBUG

namespace oz
{

void _hardAssert( const char* message, const char* file, int line, const char* function )
{
  System::trap();
  System::abort( "Hard assertion `%s' failed at %s:%d: %s", message, file, line, function );
}

void _softAssert( const char* message, const char* file, int line, const char* function )
{
  System::trap();
  System::error( "Soft assertion `%s' failed at %s:%d: %s", message, file, line, function );
}

}

#endif
