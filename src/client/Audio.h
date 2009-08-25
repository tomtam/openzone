/*
 *  Audio.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

#include "matrix/Object.h"

namespace oz
{
namespace client
{

  class Audio
  {
    friend class List<Audio, 0>;

    private:

      static const float REFERENCE_DISTANCE;

    public:

      typedef Audio *( *InitFunc )( const Object *object );

    private:

      Audio *next[1];

    protected:

      const Object      *obj;

      void playSound( int sample, float volume ) const;
      void playContSound( int sample, float volume, uint key ) const;
      void requestSounds() const;
      void releaseSounds() const;

    public:

      bool isUpdated;

      Audio( const Object *obj );
      virtual ~Audio();

      virtual void update() = 0;

  };

}
}
