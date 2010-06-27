/*
 *  MD2StaticModel.cc
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING file for details.
 */

#include "stable.hh"

#include "client/MD2StaticModel.hh"

#include "client/Context.hh"

namespace oz
{
namespace client
{

  Pool<MD2StaticModel, 0, 256> MD2StaticModel::pool;

  Model* MD2StaticModel::create( const Object* obj )
  {
    MD2StaticModel* model = new MD2StaticModel();

    model->obj  = obj;
    model->list = context.loadStaticMD2( obj->type->modelName );
    return model;
  }

  MD2StaticModel::~MD2StaticModel()
  {
    context.releaseStaticMD2( obj->type->modelName );
  }

  void MD2StaticModel::draw( const Model* )
  {
    glCallList( list );
  }

}
}