/*
 *  MD2Model.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/MD2Model.hpp"

#include "matrix/BotClass.hpp"

#include "client/Colours.hpp"
#include "client/Context.hpp"
#include "client/Camera.hpp"

namespace oz
{
namespace client
{

  Pool<MD2Model, 256> MD2Model::pool;

  Model* MD2Model::create( const Object* obj )
  {
    hard_assert( obj->flags & Object::BOT_BIT );

    const Bot* bot = static_cast<const Bot*>( obj );
    MD2Model* model = new MD2Model();

    model->obj   = obj;
    model->flags = Model::MD2MODEL_BIT;
    model->clazz = obj->clazz;
    model->md2   = context.loadMD2( obj->clazz->modelName );

    model->setAnim( bot->anim );
    model->anim.nextFrame = model->anim.endFrame;
    model->anim.currFrame = model->anim.endFrame;

    return model;
  }

  MD2Model::~MD2Model()
  {
    context.releaseMD2( clazz->modelName );
  }

  void MD2Model::setAnim( Anim type_ )
  {
    int type = int( type_ );

    anim.type       = type_;
    anim.repeat     = MD2::animList[type].repeat;

    anim.startFrame = MD2::animList[type].firstFrame;
    anim.endFrame   = MD2::animList[type].lastFrame;
    anim.nextFrame  = anim.startFrame == anim.endFrame ? anim.endFrame : anim.startFrame + 1;

    anim.fps        = MD2::animList[type].fps;
    anim.frameTime  = 1.0f / anim.fps;
    anim.currTime   = 0.0f;
  }

  void MD2Model::draw( const Model* )
  {
    if( !md2->isLoaded ) {
      return;
    }

    const Bot* bot = static_cast<const Bot*>( obj );
    const BotClass* clazz = static_cast<const BotClass*>( bot->clazz );

    glRotatef( bot->h, 0.0f, 0.0f, 1.0f );

    if( bot->anim != anim.type ) {
      setAnim( bot->anim );
    }

    if( bot->state & Bot::DEATH_BIT ) {
      float colour[] = { 1.0f, 1.0f, 1.0f, bot->life / clazz->life * 3.0f };

      glEnable( GL_BLEND );
      glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colour );

      md2->advance( &anim, timer.frameTime );
      md2->draw( &anim );

      glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Colours::WHITE );
      glDisable( GL_BLEND );
    }
    else if( bot->index != camera.bot || camera.isExternal ) {
      if( bot->state & Bot::CROUCHING_BIT ) {
        glTranslatef( 0.0f, 0.0f, clazz->dim.z - clazz->dimCrouch.z );
      }

      md2->advance( &anim, timer.frameTime );
      md2->draw( &anim );

      if( bot->weaponItem != -1 && orbis.objects[bot->weaponItem] != null ) {
        context.drawModel( orbis.objects[bot->weaponItem], this );
      }
    }
    else if( bot->weaponItem != -1 && orbis.objects[bot->weaponItem] != null ) {
      glTranslatef( 0.0f, 0.0f,  bot->camZ );
      glRotatef( bot->v, 1.0f, 0.0f, 0.0f );
      glTranslatef( 0.0f, 0.0f, -bot->camZ );

      md2->advance( &anim, timer.frameTime );
      context.drawModel( orbis.objects[bot->weaponItem], this );
    }
  }

}
}
