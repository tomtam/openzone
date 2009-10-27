/*
 *  Bot.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "precompiled.h"

#include "Bot.h"

#include "BotClass.h"
#include "Synapse.h"
#include "Physics.h"

namespace oz
{

  const float Bot::GRAB_EPSILON = 0.20f;
  const float Bot::GRAB_STRING_RATIO = 10.0f;
  const float Bot::GRAB_MOM_RATIO = 0.3f;
  // should be smaller than abs( Physics::HIT_MOMENTUM )
  const float Bot::GRAB_MOM_MAX = 1.0f;
  const float Bot::GRAB_MOM_MAX_SQ = 1.0f;

  void Bot::onUpdate()
  {
    BotClass &clazz = *(BotClass*) type;

    if( life <= type->life * 0.5f ) {
      if( ( ~state & DEATH_BIT ) && life > 0.0f ) {
        flags |= WIDE_CULL_BIT | BLEND_BIT;
        flags &= ~CLIP_BIT;
        anim = ANIM_DEATH_FALLBACK;
        life = type->life / 2.0f - EPSILON;
      }
      state |= DEATH_BIT;
    }
    if( state & DEATH_BIT ) {
      life = max( 0.0f, life - type->life * BODY_FACEOUT_FACTOR );
      return;
    }

    h = Math::mod( h + 360.0f, 360.0f );
    v = bound( v, -90.0f, 90.0f );

    // { hsine, hcosine, vsine, vcosine, vcosine * hsine, vcosine * hcosine }
    float hvsc[6];

    Math::sincos( Math::rad( h ), &hvsc[0], &hvsc[1] );
    Math::sincos( Math::rad( v ), &hvsc[2], &hvsc[3] );

    hvsc[4] = hvsc[3] * hvsc[0];
    hvsc[5] = hvsc[3] * hvsc[1];

    /*
     * STATE
     */

    bool isSwimming   = waterDepth >= dim.z;
    bool isUnderWater = waterDepth >= dim.z + camPos.z;
    bool isClimbing   = ( flags & ON_LADDER_BIT ) && grabObjIndex == -1;
    bool isGrounded   = ( lower >= 0 || ( flags & ON_FLOOR_BIT ) ) && !isSwimming;

    flags |= CLIMBER_BIT;

    stepRate *= clazz.stepRateSupp;
    stamina += clazz.staminaGain;
    stamina = min( stamina, clazz.stamina );

    if( isUnderWater ) {
      stamina -= clazz.staminaWaterDrain;

      if( stamina < 0.0f ) {
        life += stamina;
        stamina = 0.0f;
      }
    }

    /*
     * JUMP, CROUCH
     */

    // We want the player to press the key for jump each time, so logical consequence would be to
    // jump when jump key becomes pressed. But then a jump may be missed if we are in air for just
    // a brief period of time, e.g. when swimming or runing down the hill (at those occations the
    // bot is not in water/on floor all the time, but may fly for a few frames in the mean time).
    // So, if we press the jump key, we schedule for a jump, and when jump conditions are met,
    // the jump will be commited if we still hold down the jump key.
    if( actions & ACTION_JUMP ) {
      if( ~oldActions & ACTION_JUMP ) {
        state |= JUMP_SCHED_BIT;
      }
      if( ( state & JUMP_SCHED_BIT ) && ( isGrounded || isSwimming ) && grabObjIndex < 0 &&
          stamina >= clazz.staminaJumpDrain )
      {
        flags &= ~DISABLED_BIT;
        isGrounded = false;
        stamina -= clazz.staminaJumpDrain;

        momentum.z = clazz.jumpMomentum;
        addEvent( EVENT_JUMP, 1.0f );

        state &= ~JUMP_SCHED_BIT;
      }
    }
    else {
      state &= ~JUMP_SCHED_BIT;
    }

    if( actions & ~oldActions & ACTION_CROUCH ) {
      if( state & CROUCHING_BIT ) {
        float oldZ = p.z;

        p.z += clazz.dim.z - clazz.dimCrouch.z;
        dim = clazz.dim;

        if( collider.test( *this, this ) ) {
          camPos = clazz.camPos;
          state  &= ~CROUCHING_BIT;
        }
        else {
          dim = clazz.dimCrouch;
          p.z = oldZ;
        }
      }
      else {
        flags &= ~DISABLED_BIT;
        flags &= ~Object::ON_FLOOR_BIT;
        lower =  -1;

        p.z    += dim.z - clazz.dimCrouch.z;
        dim.z  = clazz.dimCrouch.z;
        camPos = clazz.camPosCrouch;
        state |= CROUCHING_BIT;
      }
    }
    if( stamina < clazz.staminaRunDrain ) {
      state &= ~RUNNING_BIT;
    }

    /*
     * ANIMATION
     */

    if( actions & ( ACTION_FORWARD | ACTION_BACKWARD | ACTION_LEFT | ACTION_RIGHT ) ) {
      anim = ( state & CROUCHING_BIT ) ? ANIM_CROUCH_WALK : ANIM_RUN;
    }
    else if( state & CROUCHING_BIT ) {
      anim = ANIM_CROUCH_STAND;
    }
    else if( state & GESTURE0_BIT ) {
      anim = ANIM_POINT;
    }
    else if( state & GESTURE1_BIT ) {
      anim = ANIM_WAVE;
    }
    else if( state & GESTURE2_BIT ) {
      anim = ANIM_FALLBACK;
    }
    else if( state & GESTURE3_BIT ) {
      anim = ANIM_SALUTE;
    }
    else if( state & GESTURE4_BIT ) {
      anim = ANIM_FLIP;
      if( ~oldState & GESTURE4_BIT ) {
        addEvent( EVENT_FLIP, 1.0f );
      }
    }
    else if( state & DEATH_BIT ) {
      anim = ANIM_DEATH_FALLBACKSLOW;
    }
    else {
      anim = ANIM_STAND;
    }

    /*
     * MOVE
     */

    Vec3 move = Vec3::zero();
    state &= ~MOVING_BIT;

    if( actions & ACTION_FORWARD ) {
      flags &= ~DISABLED_BIT;
      state |= MOVING_BIT;

      if( isSwimming ) {
        move.x -= hvsc[4];
        move.y += hvsc[5];
        move.z += hvsc[2];
      }
      else if( isClimbing ) {
        move.x -= hvsc[4];
        move.y += hvsc[5];
        move.z += v < 0.0f ? -1.0f : 1.0f;
      }
      else {
        move.x -= hvsc[0];
        move.y += hvsc[1];
      }
    }
    if( actions & ACTION_BACKWARD ) {
      flags &= ~DISABLED_BIT;
      state |= MOVING_BIT;

      if( isSwimming || isClimbing ) {
        move.x += hvsc[4];
        move.y -= hvsc[5];
        move.z -= hvsc[2];
      }
      else {
        move.x += hvsc[0];
        move.y -= hvsc[1];
      }
    }
    if( actions & ACTION_RIGHT ) {
      flags &= ~DISABLED_BIT;
      state |= MOVING_BIT;

      move.x += hvsc[1];
      move.y += hvsc[0];
    }
    if( actions & ACTION_LEFT ) {
      flags &= ~DISABLED_BIT;
      state |= MOVING_BIT;

      move.x -= hvsc[1];
      move.y -= hvsc[0];
    }

    if( !move.isZero() ) {
      move.norm();

      Vec3 desiredMomentum = move;

      if( state & CROUCHING_BIT ) {
        desiredMomentum *= clazz.crouchMomentum;
      }
      else if( ( state & RUNNING_BIT ) && grabObjIndex < 0 ) {
        desiredMomentum *= clazz.runMomentum;
      }
      else {
        desiredMomentum *= clazz.walkMomentum;
      }

      if( !isGrounded || ( flags & ON_SLICK_BIT ) ) {
        if( isClimbing ) {
          desiredMomentum *= clazz.climbControl;
        }
        else if( isSwimming ) {
          desiredMomentum *= clazz.waterControl;
        }
        else {
          desiredMomentum *= clazz.airControl;
        }
      }

      if( ( flags & Object::ON_FLOOR_BIT ) && floor.z != 1.0f ) {
        float dot = desiredMomentum * floor;

        if( dot > 0.0f ) {
          desiredMomentum -= dot * floor;
        }
      }
      momentum += desiredMomentum;

      if( ( state & RUNNING_BIT ) && ( isGrounded || isSwimming || isClimbing ) ) {
        stamina -= clazz.staminaRunDrain;
      }

      // First, check if bot's gonna hit an obstacle in the next frame. If it does, check whether it
      // would have moved further if we raised it a bit (over the obstacle). We check different
      // heights (those are specified in configuration file: stepInc and stepMax).
      // To prevent that stepping would result in "climbing" high slopes, we must check that we
      // step over an edge. In other words:
      //
      //      .                                  Start and end position must be on different sides
      //  end  .     end of a failed attempt     of a obstacle side plane we collided to.
      //     \  .   /
      //      o  . x
      // ----------     collision point
      //           \   |
      //            \  |         start
      //             \ |        /
      //              \x<------o
      //               \----------
      //
      //
      if( ( state & STEPPING_BIT ) && !isClimbing && stepRate < clazz.stepRate ) {
        // check if bot's gonna hit a stair in next frame
        Vec3 desiredMove = momentum * Timer::TICK_TIME;

        collider.translate( *this, desiredMove, this );

        if( collider.hit.ratio != 1.0f && collider.hit.normal.z < Physics::FLOOR_NORMAL_Z ) {
          float originalZ = p.z;
          Vec3  normal = collider.hit.normal;
          float negStartDist = ( desiredMove * collider.hit.ratio ) * normal - EPSILON;

          for( float raise = clazz.stepInc; raise <= clazz.stepMax; raise += clazz.stepInc ) {
            p.z += clazz.stepInc;
            if( !collider.test( *this, this ) ) {
              break;
            }
            collider.translate( *this, desiredMove, this );

            Vec3 move = desiredMove * collider.hit.ratio;
            move.z += raise;
            float endDist = move * normal - negStartDist;

            if( endDist < 0.0f ) {
              stepRate += raise;
              goto stepSucceeded;
            }
          }
          p.z = originalZ;
          stepSucceeded:;
        }
      }
    }

    /*
     * USE, GRAB
     */

    DynObject *grabObj = null;
    if( grabObjIndex >= 0 ) {
      grabObj = (DynObject*) world.objects[grabObjIndex];
      if( grabObj == null ) {
        grabObjIndex = -1;
      }
    }

    if( grabObjIndex >= 0 ) {
      if( lower == grabObjIndex || isSwimming ) {
        grabObjIndex = -1;
      }
      else {
        // keep constant length of xy projection of handle
        Vec3  handle   = Vec3( -hvsc[0], hvsc[1], hvsc[2] ) * grabHandle;
        // bottom of the object cannot be raised over the player aabb
        handle.z       = min( handle.z, dim.z - camPos.z + grabObj->dim.z );
        Vec3  string   = p + camPos + handle - grabObj->p;

        if( string.sqL() > grabHandle*grabHandle ) {
          grabObjIndex = -1;
        }
        else {
          Vec3 desiredMom   = string * GRAB_STRING_RATIO;
          Vec3 momDiff      = ( desiredMom - grabObj->momentum ) * GRAB_MOM_RATIO;

          float momDiffSqL  = momDiff.sqL();
          momDiff.z         += Physics::G_VELOCITY;
          if( momDiffSqL > GRAB_MOM_MAX_SQ ) {
            momDiff *= GRAB_MOM_MAX / Math::sqrt( momDiffSqL );
          }
          momDiff.z         -= Physics::G_VELOCITY;

          grabObj->momentum += momDiff;
          grabObj->flags    &= ~Object::DISABLED_BIT;
          flags             &= ~CLIMBER_BIT;
        }
      }
    }

    if( actions & ~oldActions & ACTION_USE ) {
      if( grabObjIndex >= 0 ) {
        synapse.use( this, grabObj );
      }
      else {
        Vec3 eye  = p + camPos;
        Vec3 look = Vec3( -hvsc[4], hvsc[5], hvsc[2] ) * clazz.grabDistance;

        collider.translate( eye, look, this );

        Object *obj = collider.hit.obj;
        if( obj != null ) {
          synapse.use( this, obj );
        }
      }
    }
    else if( actions & ~oldActions & ACTION_TAKE ) {
      if( grabObjIndex >= 0 ) {
        if( grabObj->flags & ITEM_BIT ) {
          items << grabObj;
          synapse.remove( grabObj );
        }
      }
      else {
        Vec3 eye  = p + camPos;
        Vec3 look = Vec3( -hvsc[4], hvsc[5], hvsc[2] ) * clazz.grabDistance;

        collider.translate( eye, look, this );

        Object *obj = collider.hit.obj;
        if( obj != null && ( obj->flags & ITEM_BIT ) ) {
          items << obj;
          synapse.remove( obj );
        }
      }
    }
    else if( actions & ~oldActions & ACTION_THROW ) {
      if( grabObjIndex >= 0 ) {
        Vec3 handle = Vec3( -hvsc[0], hvsc[1], hvsc[2] );

        grabObj->momentum += handle * clazz.throwMomentum;
        grabObjIndex      = -1;
      }
    }
    else if( actions & ~oldActions & ACTION_GRAB ) {
      if( grabObjIndex >= 0 || isSwimming ) {
        grabObjIndex = -1;
      }
      else {
        Vec3 eye  = p + camPos;
        Vec3 look = Vec3( -hvsc[4], hvsc[5], hvsc[2] ) * clazz.grabDistance;

        collider.translate( eye, look, this );

        Object *obj = collider.hit.obj;
        if( obj != null && ( obj->flags & Object::DYNAMIC_BIT ) ) {
          float dimX = dim.x + obj->dim.x;
          float dimY = dim.y + obj->dim.y;
          float dist = Math::sqrt( dimX*dimX + dimY*dimY ) + GRAB_EPSILON;

          if( dist <= clazz.grabDistance ) {
            grabObjIndex = collider.hit.obj->index;
            grabObj      = (DynObject*) collider.hit.obj;
            grabHandle   = dist;
            flags        &= ~ON_LADDER_BIT;
          }
        }
      }
    }

    oldState   = state;
    oldActions = actions;
  }

  void Bot::onHit( const Hit *hit, float hitMomentum )
  {
    if( hit->normal.z >= Physics::FLOOR_NORMAL_Z ) {
      addEvent( EVENT_LAND, hitMomentum / -8.0f );
    }
  }

  void Bot::onDestroy()
  {
    if( life < 0.0f ) {
      Object::onDestroy();
    }
  }

  Bot::Bot() : h( 0.0f ), v( 0.0f ), actions( 0 ), oldActions( 0 ), bob( 0.0f ), grabObjIndex( -1 ),
      stepRate( 0.0f ), anim( ANIM_STAND ), weapon( null )
  {}

  void Bot::readFull( InputStream *istream )
  {
    DynObject::readFull( istream );

    h            = istream->readFloat();
    v            = istream->readFloat();
    state        = istream->readInt();
    oldState     = istream->readInt();
    actions      = istream->readInt();
    oldActions   = istream->readInt();
    stamina      = istream->readFloat();

    grabObjIndex = istream->readInt();
    grabHandle   = istream->readFloat();

    stepRate     = istream->readFloat();
    anim         = (AnimEnum) istream->readInt();

    int nItems = istream->readInt();
    for( int i = 0; i < nItems; i++ ) {
      const String &name = istream->readString();
      items << translator.createObject( name, istream );
    }

    BotClass *clazz = (BotClass*) type;
    dim = ( state & CROUCHING_BIT ) ? clazz->dimCrouch : clazz->dim;
  }

  void Bot::writeFull( OutputStream *ostream ) const
  {
    DynObject::writeFull( ostream );

    ostream->writeFloat( h );
    ostream->writeFloat( v );
    ostream->writeInt( state );
    ostream->writeInt( oldState );
    ostream->writeInt( actions );
    ostream->writeInt( oldActions );
    ostream->writeFloat( stamina );

    ostream->writeInt( grabObjIndex );
    ostream->writeFloat( grabHandle );

    ostream->writeFloat( stepRate );
    ostream->writeInt( anim );

    ostream->writeInt( items.length() );
    foreach( item, items.iterator() ) {
      ostream->writeString( ( *item )->type->name );
      ( *item )->writeFull( ostream );
    }
  }

  void Bot::readUpdate( InputStream *istream )
  {
    Object::readUpdate( istream );

    state        = istream->readInt();
    anim         = (AnimEnum) istream->readByte();
    h            = istream->readFloat();

    grabObjIndex = istream->readInt();
  }

  void Bot::writeUpdate( OutputStream *ostream ) const
  {
    DynObject::writeUpdate( ostream );

    ostream->writeInt( state );
    ostream->writeByte( anim );
    ostream->writeFloat( h );

    ostream->writeInt( grabObjIndex );
  }

}
