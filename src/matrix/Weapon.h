/*
 *  Weapon.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#pragma once

#include "Dynamic.h"
#include "WeaponClass.h"

namespace oz
{

  class Weapon : public Dynamic
  {
    protected:

      virtual void onUpdate();
      virtual void onUse( Bot *user );
      virtual void onShot( Bot *user );

    public:

      static const int EVENT_SHOT       = 7;
      static const int EVENT_SHOT_EMPTY = 8;

      // -1: unlimited
      int   nShots;
      float shotTime;

      explicit Weapon() : Dynamic() {}

      void trigger( Bot *user )
      {
        assert( user != null );

        if( shotTime == 0.0f ) {
          if( nShots == 0 ) {
            addEvent( EVENT_SHOT_EMPTY, 1.0f );
          }
          else {
            const WeaponClass *clazz = static_cast<const WeaponClass*>( type );

            addEvent( EVENT_SHOT, 1.0f );
            onShot( user );

            nShots = max( -1, nShots - 1 );
            shotTime = clazz->shotInterval;
          }
        }
      }

      virtual void readFull( InputStream *istream );
      virtual void writeFull( OutputStream *ostream ) const;
      virtual void readUpdate( InputStream *istream );
      virtual void writeUpdate( OutputStream *ostream ) const;

  };

}
