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
 * @file client/MD2.hh
 */

#pragma once

#include <matrix/Vehicle.hh>
#include <client/common.hh>

namespace oz
{
namespace client
{

class MD2
{
public:

  enum AnimType
  {
    ANIM_NONE = -1,
    ANIM_STAND,
    ANIM_RUN,
    ANIM_ATTACK,
    ANIM_PAIN_A,
    ANIM_PAIN_B,
    ANIM_PAIN_C,
    ANIM_JUMP,
    ANIM_FLIP,
    ANIM_SALUTE,
    ANIM_WAVE,
    ANIM_FALLBACK,
    ANIM_POINT,
    ANIM_CROUCH_STAND,
    ANIM_CROUCH_WALK,
    ANIM_CROUCH_ATTACK,
    ANIM_CROUCH_PAIN,
    ANIM_CROUCH_DEATH,
    ANIM_DEATH_FALLBACK,
    ANIM_DEATH_FALLFORWARD,
    ANIM_DEATH_FALLBACKSLOW
  };

  struct AnimInfo
  {
    int      firstFrame;
    int      lastFrame;
    float    frameFreq;
    AnimType nextType;
  };

  class AnimState
  {
    friend class Model;

  private:

    static const float MIN_SHOT_INTERVAL_SYNC;

    const Bot*     bot;
    const Vehicle* vehicle;

    AnimType       currType;
    AnimType       nextType;

    int            firstFrame;
    int            lastFrame;
    int            currFrame;
    int            nextFrame;

    float          frameFreq;
    float          frameRatio;
    bool           prevAttack;

    AnimType extractAnim();
    void setAnim();

  public:

    explicit AnimState(const Bot* bot);
    explicit AnimState(const Vehicle* vehicle);

    void advance();

  };

  static const AnimInfo ANIM_LIST[];

};

}
}
