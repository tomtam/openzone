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
 * @file matrix/Frag.cpp
 */

#include "stable.hpp"

#include "matrix/Frag.hpp"

namespace oz
{
namespace matrix
{

const float Frag::MAX_ROTVELOCITY = 8.0f * Timer::TICK_TIME;
const float Frag::DAMAGE_THRESHOLD = 50.0f;

Pool<Frag, 2048> Frag::pool;

Frag::Frag( int index_, const Point3& p_, const Vec3& velocity_, const Vec3& colour_,
                    float restitution_, float mass_, float lifeTime_ ) :
    cell( null ), index( index_ ), p( p_ ), velocity( velocity_ ), colour( colour_ ),
    restitution( restitution_ ), mass( mass_ ), lifeTime( lifeTime_ )
{
  hard_assert( 1.0f + EPSILON < restitution && restitution < 2.0f - EPSILON );
  hard_assert( mass >= 0.0f );
  hard_assert( lifeTime > 0.0f );
}

void Frag::readFull( InputStream* istream )
{
  p           = istream->readPoint3();
  velocity    = istream->readVec3();
  colour      = istream->readVec3();

  restitution = istream->readFloat();
  mass        = istream->readFloat();
  lifeTime    = istream->readFloat();
}

void Frag::writeFull( BufferStream* ostream )
{
  ostream->writePoint3( p );
  ostream->writeVec3( velocity );
  ostream->writeVec3( colour );

  ostream->writeFloat( restitution );
  ostream->writeFloat( mass );
  ostream->writeFloat( lifeTime );
}

void Frag::readUpdate( InputStream* istream )
{
  p        = istream->readPoint3();
  velocity = istream->readVec3();
}

void Frag::writeUpdate( BufferStream* ostream )
{
  ostream->writePoint3( p );
  ostream->writeVec3( velocity );
}

}
}