/*
 *  VehicleAudio.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "client/BasicAudio.hpp"

namespace oz
{
namespace client
{

class VehicleAudio : public BasicAudio
{
  protected:

    explicit VehicleAudio( const Object* obj ) : BasicAudio( obj )
    {}

  public:

    static Pool<VehicleAudio, 32> pool;

    static Audio* create( const Object* obj );

    virtual void play( const Audio* parent );

  OZ_STATIC_POOL_ALLOC( pool )

};

}
}
