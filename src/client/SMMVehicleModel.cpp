/*
 *  SMMVehicleModel.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/SMMVehicleModel.hpp"

#include "matrix/Vehicle.hpp"
#include "matrix/VehicleClass.hpp"
#include "client/Context.hpp"
#include "client/Camera.hpp"

namespace oz
{
namespace client
{

  Pool<SMMVehicleModel> SMMVehicleModel::pool;

  Model* SMMVehicleModel::create( const Object* obj )
  {
    hard_assert( obj->flags & Object::VEHICLE_BIT );

    SMMVehicleModel* model = new SMMVehicleModel();

    model->obj   = obj;
    model->clazz = obj->clazz;
    model->smm   = context.requestSMM( obj->clazz->modelIndex );
    return model;
  }

  SMMVehicleModel::~SMMVehicleModel()
  {
    context.releaseSMM( clazz->modelIndex );
  }

  void SMMVehicleModel::draw( const Model* )
  {
    const Vehicle* veh = static_cast<const Vehicle*>( obj );
    const VehicleClass* clazz = static_cast<const VehicleClass*>( obj->clazz );

    if( !smm->isLoaded ) {
      return;
    }

    if( shader.activeProgram != shader.ui ) {
      tf.model.rotate( veh->rot );

      if( veh->state & Vehicle::CREW_VISIBLE_BIT ) {
        if( veh->pilot != -1 ) {
          const Bot* bot = static_cast<const Bot*>( orbis.objects[veh->pilot] );

          tf.push();
          tf.model.translate( clazz->pilotPos );

          context.drawModel( bot, this );

          tf.pop();
        }
      }
    }

    smm->draw();
  }

}
}
