/*
 *  BotClass.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#pragma once

#include "DynObjectClass.h"

#include "Object.h"

namespace oz
{

  struct BotClass : DynObjectClass
  {
    static const int BASE_FLAGS = Object::DYNAMIC_BIT | Object::UPDATE_FUNC_BIT |
        Object::HIT_FUNC_BIT | Object::BOT_BIT;

    Vec3   dimCrouch;

    Vec3   camPos;
    Vec3   camPosCrouch;

    float  bobInc;
    float  bobAmplitude;

    float  walkMomentum;
    float  runMomentum;
    float  crouchMomentum;
    float  jumpMomentum;

    float  stepInc;
    float  stepMax;
    float  stepRate;
    float  stepRateSupp;

    float  airControl;
    float  climbControl;
    float  waterControl;

    float  grabDistance;
    float  grabMass;
    float  throwMomentum;

    float  stamina;
    float  staminaGain;
    float  staminaWaterDrain;
    float  staminaRunDrain;
    float  staminaJumpDrain;

    int    state;

    float  lookLimitHMin;
    float  lookLimitHMax;
    float  lookLimitVMin;
    float  lookLimitVMax;

    String mindType;
    String mindFunction;

    static ObjectClass *init( const String &name, const Config *config );

    virtual Object *create( const Vec3 &pos );
    virtual Object *create( InputStream *istream );
  };

}
