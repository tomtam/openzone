/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 * @file client/MD2.cc
 */

#include "stable.hh"

#include "client/MD2.hh"

#include "client/Context.hh"
#include "client/OpenGL.hh"

namespace oz
{
namespace client
{

const MD2::AnimInfo MD2::ANIM_LIST[] =
{
  // first, last, frequency (FPS), nextAnim
  {   0,  39,  9.0f, ANIM_STAND         }, // STAND
  {  40,  45, 10.0f, ANIM_RUN           }, // RUN
  {  46,  53, 16.0f, ANIM_ATTACK        }, // ATTACK
  {  54,  57,  7.0f, ANIM_STAND         }, // PAIN_A
  {  58,  61,  7.0f, ANIM_STAND         }, // PAIN_B
  {  62,  65,  7.0f, ANIM_STAND         }, // PAIN_C
  {  67,  67,  9.0f, ANIM_NONE          }, // JUMP
  {  72,  83,  7.0f, ANIM_STAND         }, // FLIP
  {  84,  94,  7.0f, ANIM_STAND         }, // SALUTE
  {  95, 111, 10.0f, ANIM_STAND         }, // WAVE
  { 112, 122,  7.0f, ANIM_STAND         }, // FALLBACK
  { 123, 134,  6.0f, ANIM_STAND         }, // POINT
  { 135, 153, 10.0f, ANIM_CROUCH_STAND  }, // CROUCH_STAND
  { 154, 159,  7.0f, ANIM_CROUCH_WALK   }, // CROUCH_WALK
  { 160, 168, 18.0f, ANIM_CROUCH_ATTACK }, // CROUCH_ATTACK
  { 169, 172,  7.0f, ANIM_CROUCH_STAND  }, // CROUCH_PAIN
  { 173, 177,  5.0f, ANIM_NONE          }, // CROUCH_DEATH
  { 178, 183,  7.0f, ANIM_NONE          }, // DEATH_FALLBACK
  { 184, 189,  7.0f, ANIM_NONE          }, // DEATH_FALLFORWARD
  { 190, 197,  7.0f, ANIM_NONE          }  // DEATH_FALLBACKSLOW
};

const float MD2::AnimState::MIN_SHOT_INTERVAL_SYNC = 0.2f;

MD2::AnimType MD2::AnimState::extractAnim()
{
  const Weapon* weapon = bot->weapon < 0 ?
                         null : static_cast<const Weapon*>( orbis.objects[bot->weapon] );

  if( bot->state & Bot::DEAD_BIT ) {
    if( ANIM_DEATH_FALLBACK <= currType && currType <= ANIM_DEATH_FALLBACKSLOW ) {
      return nextType;
    }
    else {
      return AnimType( ANIM_DEATH_FALLBACK + Math::rand( 3 ) );
    }
  }
  else if( bot->cell == null ) {
    return ANIM_CROUCH_STAND;
  }
  else if( ( bot->actions & Bot::ACTION_JUMP ) &&
           !( bot->state & ( Bot::GROUNDED_BIT | Bot::CLIMBING_BIT ) ) )
  {
    return ANIM_JUMP;
  }
  else if( bot->state & Bot::MOVING_BIT ) {
    if( bot->state & Bot::CROUCHING_BIT ) {
      return ANIM_CROUCH_WALK;
    }
    else {
      return ANIM_RUN;
    }
  }
  else if( bot->cargo < 0 ) {
    if( weapon != null && ( prevAttack || weapon->shotTime != 0.0f ) ) {
      prevAttack = weapon->shotTime != 0.0f;

      return bot->state & Bot::CROUCHING_BIT ? ANIM_CROUCH_ATTACK : ANIM_ATTACK;
    }
    else if( bot->state & Bot::CROUCHING_BIT ) {
      return ANIM_CROUCH_STAND;
    }
    else if( bot->state & Bot::GESTURE_MASK ) {
      if( bot->state & Bot::GESTURE_POINT_BIT ) {
        return ANIM_POINT;
      }
      else if( bot->state & Bot::GESTURE_BACK_BIT ) {
        return ANIM_FALLBACK;
      }
      else if( bot->state & Bot::GESTURE_SALUTE_BIT ) {
        return ANIM_SALUTE;
      }
      else if( bot->state & Bot::GESTURE_WAVE_BIT ) {
        return ANIM_WAVE;
      }
      else {
        return ANIM_FLIP;
      }
    }
  }

  if( ANIM_FLIP <= currType && currType <= ANIM_POINT ) {
    return nextType;
  }
  else {
    return bot->state & Bot::CROUCHING_BIT ? ANIM_CROUCH_STAND : ANIM_STAND;
  }
}

MD2::AnimState::AnimState( const Bot* bot_ ) :
  bot( bot_ ), currType( ANIM_STAND ), nextType( ANIM_STAND ),
  frameRatio( 0.0f ), prevAttack( false )
{
  nextType = extractAnim();

  setAnim();

  currFrame = lastFrame;
  nextFrame = lastFrame;
}

void MD2::AnimState::setAnim()
{
  currType   = nextType;
  firstFrame = ANIM_LIST[nextType].firstFrame;
  lastFrame  = ANIM_LIST[nextType].lastFrame;
  frameFreq  = ANIM_LIST[nextType].frameFreq;
}

void MD2::AnimState::advance()
{
  AnimType inferredType = extractAnim();

  frameFreq = ANIM_LIST[inferredType].frameFreq;

  if( inferredType == ANIM_RUN || inferredType == ANIM_CROUCH_WALK ) {
    const BotClass* clazz = static_cast<const BotClass*>( bot->clazz );

    int   nFrames = lastFrame - firstFrame + 1;
    float stepInc = clazz->stepRunInc;

    if( ( bot->state & ( Bot::CROUCHING_BIT | Bot::WALKING_BIT ) ) || bot->cargo >= 0 ) {
      stepInc = clazz->stepWalkInc;
    }

    frameFreq = float( nFrames ) * stepInc / Timer::TICK_TIME;
  }
  else if( ( inferredType == ANIM_ATTACK || inferredType == ANIM_CROUCH_ATTACK ) &&
           bot->weapon >= 0 )
  {
    const Weapon* weapon = static_cast<const Weapon*>( orbis.objects[bot->weapon] );

    if( weapon != null ) {
      const WeaponClass* clazz = static_cast<const WeaponClass*>( weapon->clazz );

      if( clazz->shotInterval >= MIN_SHOT_INTERVAL_SYNC ) {
        int nFrames = lastFrame - firstFrame + 1;

        frameFreq = float( nFrames ) / clazz->shotInterval;
      }
    }
  }


  frameRatio += timer.frameTime * frameFreq;

  if( frameRatio >= 1.0f ) {
    currType  = nextType;
    nextType  = inferredType;
    currFrame = nextFrame;

    do {
      frameRatio -= 1.0f;

      if( inferredType != currType ) {
        nextType = inferredType;
        setAnim();
        nextFrame = firstFrame;
      }
      else if( firstFrame <= nextFrame && nextFrame < lastFrame ) {
        ++nextFrame;
      }
      else {
        AnimType newType = ANIM_LIST[currType].nextType;

        if( newType == currType ) {
          nextFrame = firstFrame;
        }
        else if( newType != ANIM_NONE ) {
          nextType = newType;
          setAnim();
          nextFrame = firstFrame;
        }
      }
    }
    while( frameRatio >= 1.0f );
  }

  hard_assert( 0.0f <= frameRatio && frameRatio < 1.0f );
}

MD2::MD2( int id_ ) :
  id( id_ ), isPreloaded( false ), isLoaded( false )
{}

MD2::~MD2()
{
  dmesh.unload();
}

void MD2::preload()
{
  file.setPath( library.models[id].path );

  if( !file.map() ) {
    throw Exception( "MD2 model file '%s' mmap failed", file.path().cstr() );
  }

  isPreloaded = true;
}

void MD2::load()
{
  InputStream is  = file.inputStream();

  shaderId        = library.shaderIndex( is.readString() );
  weaponTransf    = is.readMat44();

  dmesh.load( &is, file.path() );

  hard_assert( !is.isAvailable() );

  file.setPath( "" );

  isLoaded = true;
}

void MD2::drawFrame( int frame ) const
{
  shader.program( shaderId );
  tf.apply();

  dmesh.drawFrame( Mesh::SOLID_BIT, frame );
}

void MD2::draw( const AnimState* anim ) const
{
  shader.program( shaderId );
  tf.apply();

  dmesh.drawAnim( Mesh::SOLID_BIT, anim->currFrame, anim->nextFrame, anim->frameRatio );
}

}
}
