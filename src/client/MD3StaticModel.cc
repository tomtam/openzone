/*
 *  MD3StaticModel.cc
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING file for details.
 */

#include "stable.hh"

#include "client/MD3StaticModel.hh"

#include "client/Context.hh"

namespace oz
{
namespace client
{

  Pool<MD3StaticModel, 0, 256> MD3StaticModel::pool;

  Model* MD3StaticModel::create( const Object* obj )
  {
    MD3StaticModel* model = new MD3StaticModel();

    model->obj  = obj;
    model->list = context.loadStaticMD3( obj->type->modelName );
    return model;
  }

  MD3StaticModel::~MD3StaticModel()
  {
    context.releaseStaticMD3( obj->type->modelName );
  }

  void MD3StaticModel::draw( const Model* )
  {
    glCallList( list );
  }

}
}