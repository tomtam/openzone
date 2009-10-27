/*
 *  Bot.h
 *
 *  Bot class
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

#include "DynObject.h"
#include "World.h"
#include "Weapon.h"

namespace oz
{

  class Mind;

  class Bot : public DynObject
  {
    public:

      static const int ACTION_FORWARD  = 0x00000001;
      static const int ACTION_BACKWARD = 0x00000002;
      static const int ACTION_LEFT     = 0x00000004;
      static const int ACTION_RIGHT    = 0x00000008;
      static const int ACTION_JUMP     = 0x00000010;
      static const int ACTION_CROUCH   = 0x00000020;
      static const int ACTION_USE      = 0x00000040;
      static const int ACTION_TAKE     = 0x00000080;
      static const int ACTION_GRAB     = 0x00000100;
      static const int ACTION_THROW    = 0x00000200;
      static const int ACTION_SUICIDE  = 0x00000400;

      static const int EVENT_LAND      = 0;
      static const int EVENT_JUMP      = 1;
      static const int EVENT_FLIP      = 2;

      // can step over obstacles, e.g. walk up the stairs
      static const int STEPPING_BIT    = 0x00000001;
      static const int CROUCHING_BIT   = 0x00000002;
      static const int FREELOOK_BIT    = 0x00000004;
      static const int JUMP_SCHED_BIT  = 0x00000008;
      static const int RUNNING_BIT     = 0x00000010;
      static const int SHOOTING_BIT    = 0x00000020;
      static const int GROUNDED_BIT    = 0x00000040;
      static const int MOVING_BIT      = 0x00000080;
      static const int GESTURE0_BIT    = 0x00000100;
      static const int GESTURE1_BIT    = 0x00000200;
      static const int GESTURE2_BIT    = 0x00000400;
      static const int GESTURE3_BIT    = 0x00000800;
      static const int GESTURE4_BIT    = 0x00001000;
      static const int DEATH_BIT       = 0x00002000;
      // bot is controlled by a player, nirvana shouldn't bind a mind to it
      static const int PLAYER_BIT      = 0x00004000;

      static const float GRAB_EPSILON;
      static const float GRAB_STRING_RATIO;
      static const float GRAB_MOM_RATIO;
      static const float GRAB_MOM_MAX;
      static const float GRAB_MOM_MAX_SQ;

      enum AnimEnum
      {
        ANIM_STAND,
        ANIM_RUN,
        ANIM_ATTACK,
        ANIM_PAIN_A,
        ANIM_PAIN_B,
        ANIM_PAIN_C,
        ANIM_JUMP,
        ANIM_FLIP,
        ANIM_SALUTE,
        ANIM_FALLBACK,
        ANIM_WAVE,
        ANIM_POINT,
        ANIM_CROUCH_STAND,
        ANIM_CROUCH_WALK,
        ANIM_CROUCH_ATTACK,
        ANIM_CROUCH_PAIN,
        ANIM_CROUCH_DEATH,
        ANIM_DEATH_FALLBACK,
        ANIM_DEATH_FALLFORWARD,
        ANIM_DEATH_FALLBACKSLOW,
        ANIM_MAX
      };

      static const float BODY_FACEOUT_FACTOR = 0.0005f;

    protected:

      void onUpdate();
      void onHit( const Hit *hit, float hitMomentum );
      void onDestroy();

    public:

      float    h, v;
      int      state, oldState;
      int      actions, oldActions;
      float    stamina;

      float    bob;
      Vec3     camPos;

      int      grabObjIndex;
      float    grabHandle;

      float    stepRate;
      AnimEnum anim;

      Weapon   *weapon;
      Vector<Object*> items;

      explicit Bot();

      Quat getRot() const
      {
        return Quat::rotZYX( Math::rad( h ), 0.0f, Math::rad( v ) );
      }

      virtual void readFull( InputStream *istream );
      virtual void writeFull( OutputStream *ostream ) const;
      virtual void readUpdate( InputStream *istream );
      virtual void writeUpdate( OutputStream *ostream ) const;
  };

}
