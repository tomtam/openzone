/*
 *  Audio.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/Audio.hpp"

#include "client/Camera.hpp"
#include "client/Context.hpp"

#include "client/OpenAL.hpp"

namespace oz
{
namespace client
{

  const float Audio::REFERENCE_DISTANCE = 2.0f;
  const float Audio::ROLLOFF_FACTOR     = 0.25f;

  void Audio::playSound( int sample, float volume, const Object* obj, const Object* parent ) const
  {
    hard_assert( uint( sample ) < uint( library.sounds.length() ) );

    uint srcId;

    alGenSources( 1, &srcId );
    if( alGetError() != AL_NO_ERROR ) {
      log.println( "AL: Too many sources" );
      return;
    }

    alSourcei( srcId, AL_BUFFER, int( context.sounds[sample].id ) );

    // If the object moves since source starts playing and source stands still, it's usually
    // not noticeable for short-time source. After all, sound source many times does't move
    // together with the object in many cases (e.g. the sound when an objects hits something).
    //
    // However, when the sound is generated by the player (e.g. cries, talk) it is often annoying
    // if the sound source doesn't move with the player. That's why we position such sounds
    // at the origin of the coordinate system relative to player.
    if( obj->index == camera.bot || parent->index == camera.bot ||
        ( camera.botObj != null && parent->index == camera.botObj->parent ) )
    {
      alSourcei( srcId, AL_SOURCE_RELATIVE, AL_TRUE );
      alSourcefv( srcId, AL_POSITION, Vec3::ZERO );
    }
    else {
      alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );
      alSourcef( srcId, AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR );

      alSourcefv( srcId, AL_POSITION, parent->p );
    }

    alSourcef( srcId, AL_GAIN, volume );
    alSourcePlay( srcId );

    context.addSource( srcId, sample );

    OZ_AL_CHECK_ERROR();
  }

  void Audio::playContSound( int sample, float volume, const Object* obj,
                             const Object* parent ) const
  {
    hard_assert( uint( sample ) < uint( library.sounds.length() ) );

    int key = obj->index * ObjectClass::AUDIO_SAMPLES + sample;

    Context::ContSource* contSource = context.objSources.find( key );

    if( contSource == null ) {
      uint srcId;

      alGenSources( 1, &srcId );
      if( alGetError() != AL_NO_ERROR ) {
        log.println( "AL: Too many sources" );
        return;
      }

      alSourcei( srcId, AL_BUFFER, int( context.sounds[sample].id ) );
      alSourcei( srcId, AL_LOOPING, AL_TRUE );
      alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );
      alSourcef( srcId, AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR );

      alSourcefv( srcId, AL_POSITION, parent->p );
      alSourcef( srcId, AL_GAIN, volume );
      alSourcePlay( srcId );

      context.addObjSource( srcId, sample, key );
    }
    else {
      alSourcefv( contSource->id, AL_POSITION, parent->p );
      alSourcef( contSource->id, AL_GAIN, volume );

      contSource->isUpdated = true;
    }

    OZ_AL_CHECK_ERROR();
  }

  void Audio::playEngineSound( int sample, float volume, float pitch, const Object* obj ) const
  {
    hard_assert( uint( sample ) < uint( library.sounds.length() ) );

    int key = obj->index * ObjectClass::AUDIO_SAMPLES + sample;

    Context::ContSource* contSource = context.objSources.find( key );

    if( contSource == null ) {
      uint srcId;

      alGenSources( 1, &srcId );
      if( alGetError() != AL_NO_ERROR ) {
        log.println( "AL: Too many sources" );
        return;
      }

      alSourcei( srcId, AL_BUFFER, int( context.sounds[sample].id ) );
      alSourcei( srcId, AL_LOOPING, AL_TRUE );
      alSourcef( srcId, AL_ROLLOFF_FACTOR, 0.25f );

      alSourcefv( srcId, AL_POSITION, obj->p );
      alSourcef( srcId, AL_GAIN, volume );
      alSourcef( srcId, AL_PITCH, pitch );
      alSourcePlay( srcId );

      context.addObjSource( srcId, sample, key );
    }
    else {
      alSourcefv( contSource->id, AL_POSITION, obj->p );
      alSourcef( contSource->id, AL_GAIN, volume );
      alSourcef( contSource->id, AL_PITCH, pitch );

      contSource->isUpdated = true;
    }

    OZ_AL_CHECK_ERROR();
  }

  Audio::Audio( const Object* obj_ ) : obj( obj_ ), clazz( obj_->clazz ), flags( 0 )
  {
    const int* samples = clazz->audioSamples;

    for( int i = 0; i < ObjectClass::AUDIO_SAMPLES; ++i ) {
      if( samples[i] != -1 ) {
        context.requestSound( samples[i] );
      }
    }
    OZ_AL_CHECK_ERROR();
  }

  Audio::~Audio()
  {
    const int* samples = clazz->audioSamples;

    for( int i = 0; i < ObjectClass::AUDIO_SAMPLES; ++i ) {
      if( samples[i] != -1 ) {
        context.releaseSound( samples[i] );
      }
    }
    OZ_AL_CHECK_ERROR();
  }

}
}
