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

  void ( Vehicle::* Vehicle::handlers[] )( const Mat44& rotMat ) = {
    &Vehicle::wheeledHandler,
    &Vehicle::trackedHandler,
    &Vehicle::hoverHandler,
    &Vehicle::airHandler
  };

  void Vehicle::wheeledHandler( const Mat44& )
  {}

  void Vehicle::trackedHandler( const Mat44& )
  {}

  void Vehicle::hoverHandler( const Mat44& rotMat )
  {
    const VehicleClass* clazz = static_cast<const VehicleClass*>( this->clazz );

    const Vec3& right = rotMat.x;
    const Vec3& at    = rotMat.y;
    const Vec3& up    = rotMat.z;

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

    float height = p.z - dim.z - orbis.terra.height( p.x, p.y );

    if( height < 2.0 ) {
      momentum.z += 10.0f * ( 2.0f - height ) * Timer::TICK_TIME;
    }
  }

  void Vehicle::airHandler( const Mat44& rotMat )
  {
    const VehicleClass* clazz = static_cast<const VehicleClass*>( this->clazz );

    const Vec3& right = rotMat.x;
    const Vec3& at    = rotMat.y;
    const Vec3& up    = rotMat.z;

    if( crew[PILOT] == -1 ) {
      flags &= ~HOVER_BIT;
    }
    else {
      flags |= HOVER_BIT;
    }

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
    const VehicleClass* clazz = static_cast<const VehicleClass*>( this->clazz );

    // clean invalid crew references and throw out dead crew
    for( int i = 0; i < CREW_MAX; ++i ) {
      if( crew[i] != -1 ) {
        Bot* bot = static_cast<Bot*>( orbis.objects[ crew[i] ] );

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

    actions = 0;

    if( crew[PILOT] != -1 ) {
      Bot* pilot = static_cast<Bot*>( orbis.objects[ crew[PILOT] ] );

      rot = Quat::rotZYX( pilot->h, 0.0f, pilot->v - Math::TAU / 4.0f );
      actions = pilot->actions;
      flags &= ~DISABLED_BIT;
    }

    Mat44 rotMat = Mat44::rotation( rot );
    const Vec3& at = rotMat.y;
    const Vec3& up = rotMat.z;

    ( this->*handlers[clazz->type] )( rotMat );

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
