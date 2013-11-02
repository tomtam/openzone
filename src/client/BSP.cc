/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 * @file client/BSP.cc
 */

#include <client/BSP.hh>

#include <client/Context.hh>
#include <client/SMM.hh>

namespace oz
{
namespace client
{

BSP::BSP( const oz::BSP* bsp_ ) :
  bsp( bsp_ )
{
  foreach( model, bsp->models.citer() ) {
    context.requestModel( *model );
  }
}

BSP::~BSP()
{
  model.unload();

  if( bsp != nullptr ) {
    foreach( model, bsp->models.citer() ) {
      context.releaseModel( *model );
    }
  }
}

void BSP::schedule( const Struct* str, Model::QueueType queue )
{
  if( str != nullptr ) {
    tf.model = Mat44::translation( str->p - Point::ORIGIN );
    tf.model.rotateZ( float( str->heading ) * Math::TAU / 4.0f );

    for( int i = 0; i < str->entities.length(); ++i ) {
      const Entity& entity = str->entities[i];

      tf.push();
      tf.model.translate( entity.offset );

      model.schedule( i + 1, queue );

      if( entity.clazz->model != -1 ) {
        SMM* smm = context.smms[entity.clazz->model].handle;

        if( smm != nullptr && smm->isLoaded() ) {
          tf.model = tf.model * entity.clazz->modelTransf;

          smm->schedule( 0, queue );
        }
      }

      tf.pop();
    }
  }

  model.schedule( 0, queue );
}

void BSP::preload()
{
  const File* file = model.preload( "@bsp/" + bsp->name + ".ozcModel" );
  InputStream is   = file->inputStream( Endian::LITTLE );

  is.seek( is.available() - 2 * int( sizeof( float[4] ) ) );
  waterFogColour = is.readVec4();
  lavaFogColour  = is.readVec4();
}

void BSP::load()
{
  model.load( GL_STATIC_DRAW );
}

}
}
