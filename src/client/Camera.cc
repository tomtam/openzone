/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
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
 *
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
 */

/**
 * @file client/Camera.cc
 */

#include "stable.hh"

#include "client/Camera.hh"

#include "matrix/Collider.hh"
#include "matrix/Vehicle.hh"

#include "client/ui/UI.hh"

namespace oz
{
namespace client
{

Camera camera;

const float Camera::ROT_LIMIT      = Math::TAU / 2.0f;
const float Camera::MIN_DISTANCE   = 0.1f;
const float Camera::SMOOTHING_COEF = 0.5f;

StrategicProxy Camera::strategicProxy;
BotProxy       Camera::botProxy;

void Camera::align()
{
  h       = Math::fmod( h + Math::TAU, Math::TAU );
  v       = clamp( v, 0.0f, Math::TAU / 2.0f );

  rot     = Quat::rotZ( h ) ^ Quat::rotX( v ) ^ Quat::rotZ( w );
  rotMat  = Mat44::rotation( rot );
  rotTMat = ~rotMat;

  right   = rotMat.x;
  up      = rotMat.y;
  at      = -rotMat.z;
}

void Camera::update()
{
  relH = -float( ui::mouse.overEdgeX ) * mouseXSens;
  relV = +float( ui::mouse.overEdgeY ) * mouseYSens;

  relH = clamp( relH, -ROT_LIMIT, +ROT_LIMIT );
  relV = clamp( relV, -ROT_LIMIT, +ROT_LIMIT );

  if( ui::keyboard.keys[SDLK_LEFT] | ui::keyboard.keys[SDLK_KP1] |
      ui::keyboard.keys[SDLK_KP4] | ui::keyboard.keys[SDLK_KP7] )
  {
    relH += keyYSens;
  }
  if( ui::keyboard.keys[SDLK_RIGHT] | ui::keyboard.keys[SDLK_KP3] |
      ui::keyboard.keys[SDLK_KP6] | ui::keyboard.keys[SDLK_KP9] )
  {
    relH -= keyYSens;
  }
  if( ui::keyboard.keys[SDLK_DOWN] | ui::keyboard.keys[SDLK_KP1] |
      ui::keyboard.keys[SDLK_KP2] | ui::keyboard.keys[SDLK_KP3] )
  {
    relV -= keyXSens;
  }
  if( ui::keyboard.keys[SDLK_UP] | ui::keyboard.keys[SDLK_KP7] |
      ui::keyboard.keys[SDLK_KP8] | ui::keyboard.keys[SDLK_KP9] )
  {
    relV += keyXSens;
  }

  botObj    = bot    == -1 ? null : static_cast<Bot*>( orbis.objects[bot] );
  taggedObj = tagged == -1 ? null : orbis.objects[tagged];
  tagged    = taggedObj == null ? -1 : tagged;

  if( botObj == null || ( botObj->state & Bot::DEAD_BIT ) ) {
    bot    = -1;
    botObj = null;
  }

  if( newState != state ) {
    state = newState;
    isExternal = true;

    if( proxy != null ) {
      proxy->end();
    }

    switch( state ) {
      case NONE: {
        proxy = null;
        break;
      }
      case STRATEGIC: {
        proxy = &strategicProxy;
        break;
      }
      case BOT: {
        proxy = &botProxy;
        break;
      }
    }

    if( proxy != null ) {
      proxy->begin();
    }
    else {
      bot       = -1;
      botObj    = null;

      tagged    = -1;
      taggedObj = null;
    }
  }

  if( proxy != null ) {
    proxy->update();
  }
}

void Camera::prepare()
{
  if( proxy != null ) {
    botObj    = bot    == -1 ? null : static_cast<Bot*>( orbis.objects[bot] );
    taggedObj = tagged == -1 ? null : orbis.objects[tagged];
    tagged    = taggedObj == null ? -1 : tagged;

    if( botObj == null || ( botObj->state & Bot::DEAD_BIT ) ) {
      bot    = -1;
      botObj = null;
    }

    proxy->prepare();
  }
}

void Camera::reset()
{
  p         = Point3::ORIGIN;
  oldP      = Point3::ORIGIN;
  newP      = Point3::ORIGIN;
  h         = 0.0f;
  v         = Math::TAU / 4.0f;
  w         = 0.0f;
  relH      = 0.0f;
  relV      = 0.0f;

  rot       = Quat::ID;
  relRot    = Quat::ID;

  rotMat    = Mat44::rotation( rot );
  rotTMat   = ~rotTMat;

  right     = rotMat.x;
  up        = rotMat.y;
  at        = -rotMat.z;

  tagged    = -1;
  taggedObj = null;
  bot       = -1;
  botObj    = null;

  state     = NONE;
  newState  = NONE;

  strategicProxy.reset();
  botProxy.reset();

  if( proxy != null ) {
    proxy->end();
    proxy = null;
  }

  isExternal         = true;
  allowReincarnation = true;
}

void Camera::read( InputStream* istream )
{
  p         = istream->readPoint3();
  oldP      = p;
  newP      = p;

  h         = istream->readFloat();
  v         = istream->readFloat();
  w         = istream->readFloat();
  relH      = istream->readFloat();
  relV      = istream->readFloat();

  rot       = Quat::rotZ( h ) ^ Quat::rotX( v ) ^ Quat::rotZ( w );
  rotMat    = Mat44::rotation( rot );
  rotTMat   = ~rotMat;

  right     = rotMat.x;
  up        = rotMat.y;
  at        = -rotMat.z;

  tagged    = -1;
  taggedObj = null;
  bot       = istream->readInt();
  botObj    = bot == -1 ? null : static_cast<Bot*>( orbis.objects[bot] );

  state     = NONE;
  newState  = State( istream->readInt() );

  isExternal         = istream->readBool();
  allowReincarnation = istream->readBool();

  strategicProxy.read( istream );
  botProxy.read( istream );
}

void Camera::write( BufferStream* ostream ) const
{
  ostream->writePoint3( newP );

  ostream->writeFloat( h );
  ostream->writeFloat( v );
  ostream->writeFloat( w );
  ostream->writeFloat( relH );
  ostream->writeFloat( relV );

  ostream->writeInt( bot );

  ostream->writeInt( state );

  ostream->writeBool( isExternal );
  ostream->writeBool( allowReincarnation );

  strategicProxy.write( ostream );
  botProxy.write( ostream );
}

void Camera::init( int screenWidth, int screenHeight )
{
  width         = screenWidth;
  height        = screenHeight;
  centreX       = width  / 2;
  centreY       = height / 2;

  aspect        = config.getSet( "camera.aspect",     0.0f );
  mouseXSens    = config.getSet( "camera.mouseXSens", 0.2f ) * Timer::TICK_TIME;
  mouseYSens    = config.getSet( "camera.mouseYSens", 0.2f ) * Timer::TICK_TIME;
  keyXSens      = config.getSet( "camera.keysXSens",  2.0f ) * Timer::TICK_TIME;
  keyYSens      = config.getSet( "camera.keysYSens",  2.0f ) * Timer::TICK_TIME;

  float angle   = Math::rad( config.getSet( "camera.angle", 80.0f ) );

  aspect        = aspect != 0.0f ? aspect : float( width ) / float( height );
  coeff         = Math::tan( angle / 2.0f );
  horizPlane    = coeff * MIN_DISTANCE;
  vertPlane     = aspect * horizPlane;

  state         = NONE;
  newState      = NONE;

  strategicProxy.init();
  botProxy.init();

  reset();
}

}
}