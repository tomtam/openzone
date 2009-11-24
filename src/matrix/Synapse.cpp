/*
 *  Synapse.cpp
 *
 *  World manipulation interface.
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#include "precompiled.h"

#include "Synapse.h"

namespace oz
{

  Synapse synapse;

  Synapse::Synapse() : isSingle( true ), isServer( false ), isClient( false )
  {}

  void Synapse::genParts( int number, const Vec3 &p,
                          const Vec3 &velocity, float velocitySpread,
                          float rejection, float mass, float lifeTime,
                          const Vec3 &color, float colorSpread )
  {
    float velocitySpread2 = velocitySpread / 2.0f;
    float colorSpread2 = colorSpread / 2.0f;

    for( int i = 0; i < number; i++ ) {
      Vec3 velDisturb = Vec3( velocitySpread * Math::frand() - velocitySpread2,
                              velocitySpread * Math::frand() - velocitySpread2,
                              velocitySpread * Math::frand() - velocitySpread2 );
      Vec3 colorDisturb = Vec3( colorSpread * Math::frand() - colorSpread2,
                                colorSpread * Math::frand() - colorSpread2,
                                colorSpread * Math::frand() - colorSpread2 );
      float timeDisturb = lifeTime * Math::frand();

      addPart( p, velocity + velDisturb, rejection, mass, 0.5f * lifeTime + timeDisturb,
               color + colorDisturb );
    }
  }

  void Synapse::update()
  {
    iFree( deleteObjects.iterator() );
    deleteObjects.clear();

    actions.clear();

    addedStructs.clear();
    addedObjects.clear();
    addedParts.clear();

    removedStructs.clear();
    removedObjects.clear();
    removedParts.clear();
  }

  void Synapse::clearTickets()
  {
    putStructsIndices.clear();
    putObjectsIndices.clear();
    putPartsIndices.clear();
  }

}
