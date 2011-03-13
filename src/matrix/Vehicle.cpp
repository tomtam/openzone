/*
 *  Vehicle.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "matrix/Vehicle.hpp"

#include "matrix/Collider.hpp"
#include "matrix/VehicleClass.hpp"
#include "matrix/Bot.hpp"

namespace oz
{

  const float Vehicle::EJECT_MOVE     = 4.0f;
  const float Vehicle::EJECT_MOMENTUM = 20.0f;

  Pool<Vehicle> Vehicle::pool;

  void ( Vehicle::* Vehicle::handlers[] )() = {
    &Vehicle::wheeledHandler,
    &Vehicle::trackedHandler,
    &Vehicle::hoverHandler,
    &Vehicle::airHandler
  };

  void Vehicle::wheeledHandler()
  {}

  void Vehicle::trackedHandler()
  {}

  void Vehicle::hoverHandler()
  {}

  void Vehicle::airHandler()
  {
    const VehicleClass* clazz = static_cast<const VehicleClass*>( this->clazz );

    Mat44 rotMat = Mat44::rotation( rot );
    Vec3 right = rotMat.x;
    Vec3 at    = rotMat.y;
    Vec3 up    = rotMat.z;

    // controls
    Vec3 move = Vec3::ZERO;

    if( actions & Bot::ACTION_FORWARD ) {
      move += at;
    }
    if( actions & Bot::ACTION_BACKWARD ) {
      move -= at;
    }
    if( actions & Bot::ACTION_RIGHT ) {
      move += right;
    }
    if( actions & Bot::ACTION_LEFT ) {
      move -= right;
    }
    if( actions & Bot::ACTION_JUMP ) {
      move += up;
    }
    if( actions & Bot::ACTION_CROUCH ) {
      move -= up;
    }

    momentum += move * clazz->moveMomentum;
  }

  void Vehicle::onDestroy()
  {
    for( int i = 0; i < CREW_MAX; ++i ) {
      if( crew[i] != -1 ) {
        Bot* bot = static_cast<Bot*>( orbis.objects[ crew[i] ] );

        if( bot != null ) {
          bot->exit();
          bot->kill();
        }
      }
    }
    Object::onDestroy();
  }

  void Vehicle::onUpdate()
  {
    // clean invalid crew references and throw out dead crew
    for( int i = 0; i < CREW_MAX; ++i ) {
      if( crew[i] != -1 ) {
        Bot* bot = static_cast<Bot*>( orbis.objects[crew[i]] );

        hard_assert( bot == null || bot->parent == index );

        if( bot == null ) {
          crew[i] = -1;
        }
        else if( bot->flags & Bot::DEATH_BIT ) {
          crew[i] = -1;
          bot->exit();
        }
      }
    }

    const VehicleClass* clazz = static_cast<const VehicleClass*>( this->clazz );

    flags &= ~HOVER_BIT;
    actions = 0;
    if( crew[PILOT] != -1 ) {
      Bot* pilot = static_cast<Bot*>( orbis.objects[ crew[PILOT] ] );

      if( pilot != null ) {
        rot = Quat::rotZYX( pilot->h, 0.0f, pilot->v - Math::TAU / 4.0f );
        actions = pilot->actions;
        flags |= HOVER_BIT;
        flags &= ~DISABLED_BIT;
      }
    }

    ( this->*handlers[clazz->type] )();

    Mat44 rotMat = Mat44::rotation( rot );
    Vec3 right = rotMat.x;
    Vec3 at    = rotMat.y;
    Vec3 up    = rotMat.z;

    for( int i = 0; i < CREW_MAX; ++i ) {
      if( crew[i] != -1 ) {
        Bot* bot = static_cast<Bot*>( orbis.objects[crew[i]] );

        if( bot->actions & Bot::ACTION_EXIT ) {
          crew[i] = -1;
          bot->exit();
        }
        else if( bot->actions & Bot::ACTION_EJECT ) {
          crew[i] = -1;
          bot->exit();

          // move up a bit to prevent colliding with the vehicle
          bot->p += up * EJECT_MOVE;
          bot->momentum += ( up + 0.5f * at ) * EJECT_MOMENTUM;
        }
        else {
          bot->p = p + rotMat * clazz->crewPos[0] + momentum * Timer::TICK_TIME;
          bot->momentum = velocity;
        }
      }
    }

    oldActions = actions;
    oldState   = state;
  }

  void Vehicle::onUse( Bot* user )
  {
    if( crew[PILOT] == -1 ) {
      crew[PILOT] = user->index;
      user->enter( index );
    }
  }

  Vehicle::Vehicle() : rot( Quat::ID ), actions( 0 ), oldActions( 0 )
  {
    aSet( crew, -1, CREW_MAX );
  }

  void Vehicle::readFull( InputStream* istream )
  {
    Dynamic::readFull( istream );

    rot          = istream->readQuat();
    state        = istream->readInt();
    oldState     = istream->readInt();
    actions      = istream->readInt();
    oldActions   = istream->readInt();

    for( int i = 0; i < CREW_MAX; ++i ) {
      crew[i] = istream->readInt();
    }
  }

  void Vehicle::writeFull( OutputStream* ostream ) const
  {
    Dynamic::writeFull( ostream );

    ostream->writeQuat( rot );
    ostream->writeInt( state );
    ostream->writeInt( oldState );
    ostream->writeInt( actions );
    ostream->writeInt( oldActions );

    for( int i = 0; i < CREW_MAX; ++i ) {
      ostream->writeInt( crew[i] );
    }
  }

  void Vehicle::readUpdate( InputStream* istream )
  {
    Dynamic::readUpdate( istream );

    rot   = istream->readQuat();
    state = istream->readInt();
  }

  void Vehicle::writeUpdate( OutputStream* ostream ) const
  {
    Dynamic::writeUpdate( ostream );

    ostream->writeQuat( rot );
    ostream->writeInt( state );
  }

}
