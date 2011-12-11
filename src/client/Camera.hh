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
 * @file client/Camera.hh
 */

#pragma once

#include "matrix/Orbis.hh"
#include "matrix/Bot.hh"

#include "client/StrategicProxy.hh"
#include "client/BotProxy.hh"

namespace oz
{
namespace client
{

class Camera
{
  public:

    static constexpr float ROT_LIMIT      = Math::TAU / 2.0f;
    static constexpr float MIN_DISTANCE   = 0.1f;
    static constexpr float SMOOTHING_COEF = 0.5f;

    enum State
    {
      NONE,
      STRATEGIC,
      BOT
    };

    float mouseXSens;
    float mouseYSens;
    float keyXSens;
    float keyYSens;

    static StrategicProxy strategicProxy;
    static BotProxy       botProxy;

    Proxy* proxy;

  public:

    Point3 p;
    Point3 newP;
    Point3 oldP;

    // relative to the object the camera is bound to
    float  h;
    float  v;
    float  w;
    float  relH;
    float  relV;

    Quat   relRot;
    Quat   rot;

    // global rotation matrix and it's inverse
    Mat44  rotMat;
    Mat44  rotTMat;

    Vec3   right;
    Vec3   up;
    Vec3   at;

    int    tagged;
    const Object* taggedObj;

    int    bot;
    Bot*   botObj;

    State  state;
    State  newState;

    int    width;
    int    height;
    int    centreX;
    int    centreY;

    float  coeff;
    float  aspect;
    float  vertPlane;
    float  horizPlane;
    float  maxDist;

    bool   isExternal;
    bool   allowReincarnation;

    void setState( State state )
    {
      newState = state;
    }

    void setTagged( const Object* obj )
    {
      tagged    = obj == null ? -1 : obj->index;
      taggedObj = obj;
    }

    void setBot( Bot* botObj_ )
    {
      if( botObj != null ) {
        botObj->state &= ~Bot::PLAYER_BIT;
      }

      if( botObj_ == null ) {
        bot    = -1;
        botObj = null;
      }
      else {
        bot    = botObj_->index;
        botObj = botObj_;

        botObj_->state |= Bot::PLAYER_BIT;
      }

      hard_assert( botObj == null || ( botObj->flags & Object::BOT_BIT ) );
    }

    void move( const Point3& pos )
    {
      p    = Math::mix( oldP, pos, SMOOTHING_COEF );
      newP = pos;
      oldP = p;
    }

    void warp( const Point3& pos )
    {
      oldP = pos;
      newP = pos;
      p    = pos;
    }

    void warpMoveZ( const Point3& pos )
    {
      p.x  = pos.x;
      p.y  = pos.y;
      p.z  = Math::mix( oldP.z, pos.z, SMOOTHING_COEF );
      newP = pos;
      oldP = p;
    }

    void align();
    void update();
    void prepare();
    void reset();

    void read( InputStream* istream );
    void write( BufferStream* ostream ) const;

    void init( int screenWidth, int screenHeight );

};

extern Camera camera;

}
}
