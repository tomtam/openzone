/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
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
 */

/**
 * @file matrix/BotClass.hh
 */

#pragma once

#include "matrix/DynamicClass.hh"

namespace oz
{
namespace matrix
{

class BotClass : public DynamicClass
{
  public:

    int    state;

    Vec3   crouchDim;
    Vec3   corpseDim;

    float  camZ;
    float  crouchCamZ;

    float  bobWalkInc;
    float  bobRunInc;
    float  bobSwimInc;
    float  bobSwimRunInc;
    float  bobRotation;
    float  bobAmplitude;
    float  bobSwimAmplitude;

    float  walkMomentum;
    float  runMomentum;
    float  crouchMomentum;
    float  jumpMomentum;

    float  airControl;
    float  climbControl;
    float  waterControl;
    float  slickControl;

    float  stepInc;
    float  stepMax;
    float  stepRateLimit;
    float  stepRateCoeff;
    float  stepRateSupp;

    float  climbInc;
    float  climbMax;
    float  climbMomentum;

    float  stamina;
    float  staminaGain;
    float  staminaRunDrain;
    float  staminaWaterDrain;
    float  staminaClimbDrain;
    float  staminaJumpDrain;
    float  staminaThrowDrain;

    float  regeneration;

    float  reachDist;
    float  grabMass;
    float  throwMomentum;

    int    weaponItem;

    int    nameList;

    String mindFunc;

    static ObjectClass* createClass();

    virtual void initClass( const Config* config );

    virtual Object* create( int index, const Point3& pos, Heading heading ) const;
    virtual Object* create( InputStream* istream ) const;

};

}
}
