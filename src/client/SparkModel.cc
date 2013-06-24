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
 * @file client/SparkModel.cc
 */

#include <client/SparkModel.hh>

#include <client/Frustum.hh>

namespace oz
{
namespace client
{

//  void SparkGenRender::createSpark( int i )
//  {
//    float velocitySpread2 = sparkGen->velocitySpread * 0.5f;
//    Vec3 velDisturb = Vec3( sparkGen->velocitySpread * Math::frand() - velocitySpread2,
//                            sparkGen->velocitySpread * Math::frand() - velocitySpread2,
//                            sparkGen->velocitySpread * Math::frand() - velocitySpread2 );
//
//    Vec3 colorDisturb = Vec3( sparkGen->colorSpread.x * Math::frand(),
//                              sparkGen->colorSpread.y * Math::frand(),
//                              sparkGen->colorSpread.z * Math::frand() );
//
//    sparks[i].p = sparkGen->p;
//    sparks[i].velocity = sparkGen->velocity + velDisturb;
//    sparks[i].colour = sparkGen->colour + colorDisturb;
//    sparks[i].lifeTime = sparkGen->lifeTime;
//  }
//
//  SparkGenRender::SparkGenRender( SparkGen* sparkGen_ ) :
//    sparkGen( sparkGen_ )
//  {
//    sparks = new Spark[sparkGen->number];
//    startMillis = float( timer.millis );
//    nSparks = 0;
//
//    for( int i = 0; i < sparkGen->number; ++i ) {
//      sparks[i].lifeTime = 0.0f;
//    }
//  }
//
//  SparkGenRender::SparkGenRender( const SparkGenRender& sparkGenRender ) :
//    sparkGen( sparkGenRender.sparkGen ),
//    startMillis( sparkGenRender.startMillis ),
//    sparksPerTick( sparkGenRender.sparksPerTick ),
//    nSparks( sparkGenRender.nSparks )
//  {
//    sparks = new Spark[nSparks];
//    aCopy( sparks, sparkGenRender.sparks, nSparks );
//  }
//
//  SparkGenRender::~SparkGenRender()
//  {
//    delete[] sparks;
//  }
//
//  void SparkGenRender::draw()
//  {
//    float sparkDim = sparkGen->sparkDim;
//
//    glPushMatrix();
//
//    glMultMatrixf( camera.rotMat );
//
//    for( int i = 0; i < nSparks; ++i ) {
//      Vec3& p = sparks[i].p;
//
//      glTranslatef( p.x, p.y, p.z );
//
//      glBegin( GL_QUADS );
//        glTexCoord2f( 0.0f, 0.0f );
//        glVertex3f( -sparkDim, -sparkDim, 0.0f );
//        glTexCoord2f( 0.0f, 1.0f );
//        glVertex3f( +sparkDim, -sparkDim, 0.0f );
//        glTexCoord2f( 1.0f, 1.0f );
//        glVertex3f( +sparkDim, +sparkDim, 0.0f );
//        glTexCoord2f( 1.0f, 0.0f );
//        glVertex3f( -sparkDim, +sparkDim, 0.0f );
//      glEnd();
//
//      glTranslatef( -p.x, -p.y, -p.z );
//    }
//
//    glPopMatrix();
//  }
//
//  void SparkGenRender::update() {
//    if( nSparks != sparkGen->number ) {
//      int desiredNSparks = int( ( timer.millis - startMillis ) * sparksPerTick );
//
//      nSparks = min( desiredNSparks, sparkGen->number );
//    }
//    for( int i = 0; i < nSparks; ++i ) {
//      sparks[i].lifeTime -= Timer::FRAME_TIME;
//
//      if( sparks[i].lifeTime <= 0.0f ) {
//        createSpark( i );
//      }
//    }
//  }

}
}
