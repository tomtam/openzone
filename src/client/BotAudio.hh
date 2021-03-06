/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2016 Davorin Učakar
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
 * @file client/BotAudio.hh
 */

#pragma once

#include <client/BasicAudio.hh>

namespace oz
{
namespace client
{

class BotAudio : public BasicAudio
{
private:

  static const float FOOTSTEP_DISTANCE_SQ;

  int prevStep;

protected:

  explicit BotAudio(const Object* obj);

public:

  static Pool<BotAudio> pool;

  static Audio* create(const Object* obj);

  void play(const Object* playAt) override;

  OZ_STATIC_POOL_ALLOC(pool)
};

}
}
