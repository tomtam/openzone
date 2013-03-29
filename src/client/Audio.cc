/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 * @file client/Audio.cc
 */

#include <stable.hh>
#include <client/Audio.hh>

#include <client/Camera.hh>
#include <client/Context.hh>
#include <ozEngine/AL.hh>

namespace oz
{
namespace client
{

const float Audio::REFERENCE_DISTANCE   = 2.0f;
const float Audio::COCKPIT_GAIN_FACTOR  = 0.35f;
const float Audio::COCKPIT_PITCH_FACTOR = 0.95f;

void Audio::playSound( int sound, float volume, const Object* parent ) const
{
  hard_assert( uint( sound ) < uint( liber.sounds.length() ) );

  const Dynamic* dynParent = static_cast<const Dynamic*>( parent );

  uint srcId = context.addSource( sound );
  if( srcId == Context::INVALID_SOURCE ) {
    return;
  }

  alSourcei( srcId, AL_BUFFER, int( context.sounds[sound].id ) );
  alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );

  alSourcef( srcId, AL_GAIN, volume );

  // If the object moves since source starts playing and source stands still, it's usually
  // not noticeable for short-time source. After all, sound source many times does't move
  // together with the object in many cases (e.g. the sound when an objects hits something).
  //
  // However, when the sound is generated by the player (e.g. cries, talk) it is often annoying
  // if the sound source doesn't move with the player. That's why we position such sounds
  // at the origin of the coordinate system relative to player.
  if( parent == camera.botObj || obj == camera.botObj ||
      ( camera.botObj != nullptr && parent->index == camera.botObj->parent ) )
  {
    alSourcei( srcId, AL_SOURCE_RELATIVE, AL_TRUE );
  }
  else {
    alSourcefv( srcId, AL_POSITION, parent->p );
    if( parent->flags & Object::DYNAMIC_BIT ) {
      alSourcefv( srcId, AL_VELOCITY, dynParent->velocity );
    }
  }

  alSourcePlay( srcId );

  OZ_AL_CHECK_ERROR();
}

void Audio::playContSound( int sound, float volume, const Object* parent ) const
{
  hard_assert( uint( sound ) < uint( liber.sounds.length() ) );

  int key = obj->index * ObjectClass::MAX_SOUNDS + sound;

  Context::ContSource* contSource = context.contSources.find( key );
  const Dynamic*       dynParent  = static_cast<const Dynamic*>( parent );

  if( contSource == nullptr ) {
    uint srcId = context.addContSource( sound, key );
    if( srcId == Context::INVALID_SOURCE ) {
      return;
    }

    alSourcei( srcId, AL_BUFFER, int( context.sounds[sound].id ) );
    alSourcei( srcId, AL_LOOPING, AL_TRUE );
    alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );

    alSourcef( srcId, AL_GAIN, volume );
    alSourcefv( srcId, AL_POSITION, parent->p );
    if( parent->flags & Object::DYNAMIC_BIT ) {
      alSourcefv( srcId, AL_VELOCITY, dynParent->velocity );
    }

    alSourcePlay( srcId );
  }
  else {
    uint srcId = contSource->id;

    alSourcef( srcId, AL_GAIN, volume );
    alSourcefv( srcId, AL_POSITION, parent->p );
    if( parent->flags & Object::DYNAMIC_BIT ) {
      alSourcefv( srcId, AL_VELOCITY, dynParent->velocity );
    }

    contSource->isUpdated = true;
  }

  OZ_AL_CHECK_ERROR();
}

bool Audio::playSpeak( const char* text, float volume, const Object* parent ) const
{
  const Dynamic* dynParent = static_cast<const Dynamic*>( parent );

  if( context.speakSource.owner < 0 ) {
    if( text == nullptr ) {
      return false;
    }

    uint srcId = context.requestSpeakSource( text, obj->index );
    if( srcId == Context::INVALID_SOURCE ) {
      return false;
    }

    alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );

    alSourcef( srcId, AL_GAIN, volume );
    alSourcefv( srcId, AL_POSITION, parent->p );
    if( parent->flags & Object::DYNAMIC_BIT ) {
      alSourcefv( srcId, AL_VELOCITY, dynParent->velocity );
    }
  }
  else if( context.speakSource.owner == obj->index ) {
    uint srcId = context.speakSource.id;

    alSourcef( srcId, AL_GAIN, volume );
    alSourcefv( srcId, AL_POSITION, parent->p );
    if( parent->flags & Object::DYNAMIC_BIT ) {
      alSourcefv( srcId, AL_VELOCITY, dynParent->velocity );
    }
  }

  OZ_AL_CHECK_ERROR();

  return true;
}

void Audio::playEngineSound( int sound, float volume, float pitch ) const
{
  hard_assert( uint( sound ) < uint( liber.sounds.length() ) );
  hard_assert( obj->flags & Object::VEHICLE_BIT );

  const Vehicle* veh = static_cast<const Vehicle*>( obj );

  if( !camera.isExternal && veh->pilot >= 0 && veh->pilot == camera.bot ) {
    volume *= COCKPIT_GAIN_FACTOR;
    pitch  *= COCKPIT_PITCH_FACTOR;
  }

  int key = veh->index * ObjectClass::MAX_SOUNDS + sound;

  Context::ContSource* contSource = context.contSources.find( key );

  if( contSource == nullptr ) {
    uint srcId = context.addContSource( sound, key );
    if( srcId == Context::INVALID_SOURCE ) {
      return;
    }

    alSourcei( srcId, AL_BUFFER, int( context.sounds[sound].id ) );
    alSourcei( srcId, AL_LOOPING, AL_TRUE );
    alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );

    alSourcef( srcId, AL_PITCH, pitch );
    alSourcef( srcId, AL_GAIN, volume );
    alSourcefv( srcId, AL_POSITION, veh->p );
    alSourcefv( srcId, AL_VELOCITY, veh->velocity );

    alSourcePlay( srcId );
  }
  else {
    uint srcId = contSource->id;

    alSourcef( contSource->id, AL_PITCH, pitch );
    alSourcef( contSource->id, AL_GAIN, volume );
    alSourcefv( srcId, AL_POSITION, veh->p );
    alSourcefv( srcId, AL_VELOCITY, veh->velocity );

    contSource->isUpdated = true;
  }

  OZ_AL_CHECK_ERROR();
}

Audio::Audio( const Object* obj_ ) :
  obj( obj_ ), clazz( obj_->clazz ), flags( 0 )
{
  const int* sounds = clazz->audioSounds;

  Log::verboseMode = true;

  for( int i = 0; i < ObjectClass::MAX_SOUNDS; ++i ) {
    if( sounds[i] >= 0 ) {
      context.requestSound( sounds[i] );
    }
  }

  Log::verboseMode = false;

  OZ_AL_CHECK_ERROR();
}

Audio::~Audio()
{
  const int* sounds = clazz->audioSounds;

  Log::verboseMode = true;

  for( int i = 0; i < ObjectClass::MAX_SOUNDS; ++i ) {
    if( sounds[i] >= 0 ) {
      context.releaseSound( sounds[i] );
    }
  }

  Log::verboseMode = false;

  OZ_AL_CHECK_ERROR();
}

}
}
