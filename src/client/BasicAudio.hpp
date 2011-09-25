/*
 *  BasicAudio.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "stable.hpp"

#include "client/Audio.hpp"

namespace oz
{
namespace client
{

  class BasicAudio : public Audio
  {
    protected:

      int oldFlags[2];

      explicit BasicAudio( const Object* obj );

    public:

      static Pool<BasicAudio, 1024> pool;

      static Audio* create( const Object* obj );

      void play( const Audio* parent );

    OZ_STATIC_POOL_ALLOC( pool )

  };

}
}
