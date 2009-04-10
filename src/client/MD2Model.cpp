/*
 *  MD2Model.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "precompiled.h"

#include "MD2Model.h"

#include "Context.h"

namespace oz
{
namespace client
{

  Model *MD2Model::create( const Object *object )
  {
    assert( object->flags & Object::BOT_BIT );

    MD2Model *model = new MD2Model();

    model->bot = (const Bot*) object;
    model->md2 = context.loadMD2Model( object->type->modelPath );

    model->anim.type     = -1;
    model->anim.currTime = 0.0f;
    model->anim.oldTime  = 0.0f;
    model->setAnim( 0 );

    return model;
  }

  void MD2Model::setAnim( int type )
  {
    if( anim.type != type ) {
      anim.startFrame = MD2::animList[type].firstFrame;
      anim.endFrame   = MD2::animList[type].lastFrame;
      anim.currFrame  = anim.startFrame;
      anim.nextFrame  = anim.startFrame + 1;
      anim.fps        = MD2::animList[type].fps;
      anim.frameTime  = 1.0f / anim.fps;
      anim.type       = type;
    }
  }

  void MD2Model::draw()
  {
    setAnim( bot->anim );
    md2->draw( &anim );
  }

}
}
