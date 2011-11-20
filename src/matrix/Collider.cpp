/*
 * OpenZone - Simple Cross-Platform FPS/RTS Game Engine
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
 * @file matrix/Collider.cpp
 */

#include "stable.hpp"

#include "matrix/Collider.hpp"

namespace oz
{
namespace matrix
{

Collider collider;

const Vec3 Collider::normals[] =
{
  Vec3(  1.0f,  0.0f,  0.0f ),
  Vec3( -1.0f,  0.0f,  0.0f ),
  Vec3(  0.0f,  1.0f,  0.0f ),
  Vec3(  0.0f, -1.0f,  0.0f ),
  Vec3(  0.0f,  0.0f,  1.0f ),
  Vec3(  0.0f,  0.0f, -1.0f )
};

Collider::Collider() : mask( Object::SOLID_BIT )
{}

inline bool Collider::visitBrush( int index )
{
  bool isTested = visitedBrushes.get( index );
  visitedBrushes.set( index );
  return isTested;
}

//***********************************
//*         STATIC AABB CD          *
//***********************************

// checks if AABB and Object overlap
bool Collider::overlapsAABBObj( const Object* sObj ) const
{
  if( flags & sObj->flags & Object::CYLINDER_BIT ) {
    Vec3  relPos  = aabb.p - sObj->p;

    float sumDimX = aabb.dim.x + sObj->dim.x;
    float sumDimZ = aabb.dim.z + sObj->dim.z;

    float distXY2 = relPos.x*relPos.x + relPos.y*relPos.y;
    float radius  = sumDimX + EPSILON;

    return distXY2 <= radius*radius &&
        relPos.z <= +sumDimZ + EPSILON &&
        relPos.z >= -sumDimZ - EPSILON;
  }
  return sObj->overlaps( aabb, EPSILON );
}

// checks if AABB and Brush overlap
bool Collider::overlapsAABBBrush( const BSP::Brush* brush ) const
{
  bool result = true;

  for( int i = 0; i < brush->nSides; ++i ) {
    const Plane& plane = bsp->planes[ bsp->brushSides[brush->firstSide + i] ];

    float offset = localDim * plane.abs();
    float dist   = startPos * plane - offset;

    result &= dist <= EPSILON;
  }
  return result;
}

// recursively check nodes of BSP-tree for AABB-Brush overlapping
bool Collider::overlapsAABBNode( int nodeIndex )
{
  if( nodeIndex < 0 ) {
    const BSP::Leaf& leaf = bsp->leaves[~nodeIndex];

    for( int i = 0; i < leaf.nBrushes; ++i ) {
      int index = bsp->leafBrushes[leaf.firstBrush + i];
      const BSP::Brush& brush = bsp->brushes[index];

      if( !visitBrush( index ) && ( brush.material & Material::STRUCT_BIT ) &&
          overlapsAABBBrush( &brush ) )
      {
        return true;
      }
    }
    return false;
  }
  else {
    const BSP::Node& node  = bsp->nodes[nodeIndex];
    const Plane&     plane = bsp->planes[node.plane];

    float offset = localDim * plane.abs() + 2.0f * EPSILON;
    float dist   = startPos * plane;

    if( dist > offset ) {
      return overlapsAABBNode( node.front );
    }
    else if( dist < -offset ) {
      return overlapsAABBNode( node.back );
    }
    else {
      return overlapsAABBNode( node.front ) || overlapsAABBNode( node.back );
    }
  }
}

bool Collider::overlapsAABBEntities()
{
  if( str->nEntities == 0 ) {
    return false;
  }

  Point3 originalStartPos = startPos;
  Bounds localTrace       = str->toStructCS( trace );

  for( int i = 0; i < bsp->nModels; ++i ) {
    model  = &bsp->models[i];
    entity = &str->entities[i];

    if( localTrace.overlaps( *model + entity->offset ) ) {
      for( int j = 0; j < model->nBrushes; ++j ) {
        int index = model->firstBrush + j;
        const BSP::Brush& brush = bsp->brushes[index];

        hard_assert( !visitedBrushes.get( index ) );

        startPos = originalStartPos - entity->offset;

        if( ( brush.material & Material::STRUCT_BIT ) && overlapsAABBBrush( &brush ) ) {
          startPos = originalStartPos;
          return true;
        }
      }
    }
  }
  startPos = originalStartPos;
  return false;
}

// check for AABB-AABB, AABB-Brush and AABB-Terrain overlapping in the world
bool Collider::overlapsAABBOrbis()
{
  if( !orbis.includes( aabb, -EPSILON ) ) {
    return true;
  }

  if( aabb.p.z - aabb.dim.z - orbis.terra.height( aabb.p.x, aabb.p.y ) <= 0.0f ) {
    return true;
  }

  const Struct* oldStr = null;

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( int i = 0; i < cell.structs.length(); ++i ) {
        str = orbis.structs[ cell.structs[i] ];

        if( str != oldStr && str->overlaps( trace ) ) {
          visitedBrushes.clearAll();

          startPos = str->toStructCS( aabb.p );
          localDim = str->swapDimCS( aabb.dim );
          bsp      = str->bsp;

          if( overlapsAABBNode( 0 ) || overlapsAABBEntities() ) {
            return true;
          }

          oldStr = str;
        }
      }

      for( const Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( sObj != exclObj && ( sObj->flags & mask ) && overlapsAABBObj( sObj ) ) {
          return true;
        }
      }
    }
  }
  return false;
}

// check for AABB-AABB overlapping in the world
bool Collider::overlapsAABBOrbisOO()
{
  if( !orbis.includes( aabb, -EPSILON ) ) {
    return true;
  }

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( const Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( sObj != exclObj && ( sObj->flags & mask ) && overlapsAABBObj( sObj ) ) {
          return true;
        }
      }
    }
  }
  return false;
}

// check for AABB-AABB and AABB-Brush overlapping in the world
bool Collider::overlapsAABBOrbisOSO()
{
  if( !orbis.includes( aabb, -EPSILON ) ) {
    return true;
  }

  const Struct* oldStr = null;

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( int i = 0; i < cell.structs.length(); ++i ) {
        str = orbis.structs[ cell.structs[i] ];

        if( str != oldStr && str->overlaps( trace ) ) {
          visitedBrushes.clearAll();

          startPos = str->toStructCS( aabb.p );
          localDim = str->swapDimCS( aabb.dim );
          bsp      = str->bsp;

          if( overlapsAABBNode( 0 ) || overlapsAABBEntities() ) {
            return true;
          }

          oldStr = str;
        }
      }

      for( const Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( sObj != exclObj && ( sObj->flags & mask ) && overlapsAABBObj( sObj ) ) {
          return true;
        }
      }
    }
  }
  return false;
}

//***********************************
//*        STATIC ENTITY CD         *
//***********************************

// check for BSP Model-AABB overlapping in the world
bool Collider::overlapsEntityOrbisOO()
{
  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( const Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( sObj->overlaps( trace ) ) {
          startPos = str->toStructCS( sObj->p ) - entity->offset;
          localDim = str->swapDimCS( sObj->dim + Vec3( margin, margin, margin ) );

          for( int i = 0; i < model->nBrushes; ++i ) {
            const BSP::Brush& brush = bsp->brushes[model->firstBrush + i];

            if( ( brush.material & Material::STRUCT_BIT ) && ( sObj->flags & mask ) &&
                overlapsAABBBrush( &brush ) )
            {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

//***********************************
//*        DYNAMIC AABB CD          *
//***********************************

// finds out if AABB-Orbis bounding box collision occurs and the time when it occurs
void Collider::trimAABBVoid()
{
  for( int i = 0; i < 3; ++i ) {
    int side = move[i] >= 0.0f;
    const Vec3& normal = normals[i * 2 + side];

    float startDist = orbis.maxs[i] + startPos[i] * normal[i] - aabb.dim[i];
    float endDist   = orbis.maxs[i] + endPos[i]   * normal[i] - aabb.dim[i];

    if( endDist <= EPSILON && endDist <= startDist ) {
      float ratio = startDist / max( startDist - endDist, Math::EPSILON );

      if( ratio < hit.ratio ) {
        hit.ratio    = max( 0.0f, ratio );
        hit.normal   = normal;
        hit.obj      = null;
        hit.str      = null;
        hit.entity   = null;
        hit.material = Material::VOID_BIT;
      }
    }
  }
}

// finds out if AABB-AABB collision occurs and the time when it occurs
void Collider::trimAABBObj( const Object* sObj )
{
  float minRatio   = -1.0f;
  float maxRatio   = +1.0f;
  Vec3  lastNormal = Vec3::ZERO;

  Vec3  relStartPos = startPos - sObj->p;
  Vec3  relEndPos   = endPos   - sObj->p;
  Vec3  sumDim      = aabb.dim + sObj->dim;

  int   firstPlane  = 0;

  if( flags & sObj->flags & Object::CYLINDER_BIT ) {
    firstPlane = 2;

    float px         = relStartPos.x;
    float py         = relStartPos.y;
    float rx         = relEndPos.x;
    float ry         = relEndPos.y;
    float sx         = move.x;
    float sy         = move.y;
    float startDist2 = px*px + py*py;
    float endDist2   = rx*rx + ry*ry;
    float radius2    = sumDim.x*sumDim.x;
    float radiusEps2 = ( sumDim.x + EPSILON ) * ( sumDim.x + EPSILON );

    if( endDist2 > radiusEps2 ) {
      float moveDist2    = sx*sx + sy*sy;
      float pxsx_pysy    = px*sx + py*sy;
      float pxsy_pysx    = px*sy - py*sx;
      float discriminant = radiusEps2 * moveDist2 - pxsy_pysx * pxsy_pysx;

      if( startDist2 < radius2 ) {
        hard_assert( discriminant > 0.0f );

        float sqrtDiscr = Math::sqrt( discriminant );
        float endRatio  = ( -pxsx_pysy + sqrtDiscr ) / max( moveDist2, Math::EPSILON );

        maxRatio = min( maxRatio, endRatio );
      }
      else if( discriminant < 0.0f ) {
        return;
      }
      else {
        float sqrtDiscr = Math::sqrt( discriminant );
        float endRatio  = ( -pxsx_pysy + sqrtDiscr ) / max( moveDist2, Math::EPSILON );

        if( endRatio <= 0.0f ) {
          return;
        }

        maxRatio = min( maxRatio, endRatio );

        float startRatio = ( -pxsx_pysy - sqrtDiscr ) / max( moveDist2, Math::EPSILON );

        if( startRatio > minRatio ) {
          minRatio = startRatio;
          lastNormal = ~Vec3( px + startRatio*sx, py + startRatio*sy, 0.0f );
        }
      }
    }
    else if( startDist2 >= radius2 && endDist2 <= startDist2 ) {
      float moveDist2    = sx*sx + sy*sy;
      float pxsx_pysy    = px*sx + py*sy;
      float pxsy_pysx    = px*sy - py*sx;
      float discriminant = radiusEps2 * moveDist2 - pxsy_pysx * pxsy_pysx;
      float sqrtDiscr    = Math::sqrt( max( discriminant, 0.0f ) );
      float startRatio   = ( -pxsx_pysy - sqrtDiscr ) / max( moveDist2, Math::EPSILON );

      if( startRatio > minRatio ) {
        minRatio   = startRatio;
        lastNormal = ~Vec3( px + startRatio*sx, py + startRatio*sy, 0.0f );
      }
    }
  }

  for( int i = firstPlane; i < 3; ++i ) {
    float startDist, endDist;

    startDist = +relStartPos[i] - sumDim[i];
    endDist   = +relEndPos[i]   - sumDim[i];

    if( endDist > EPSILON ) {
      if( startDist < 0.0f ) {
        maxRatio = min( maxRatio, startDist / ( startDist - endDist ) );
      }
      else {
        return;
      }
    }
    else if( startDist >= 0.0f && endDist <= startDist ) {
      float ratio = ( startDist - EPSILON ) / max( startDist - endDist, Math::EPSILON );

      if( ratio > minRatio ) {
        minRatio   = ratio;
        lastNormal = normals[2*i];
      }
    }

    startDist = -relStartPos[i] - sumDim[i];
    endDist   = -relEndPos[i]   - sumDim[i];

    if( endDist > EPSILON ) {
      if( startDist < 0.0f ) {
        maxRatio = min( maxRatio, startDist / ( startDist - endDist ) );
      }
      else {
        return;
      }
    }
    else if( startDist >= 0.0f && endDist <= startDist ) {
      float ratio = ( startDist - EPSILON ) / max( startDist - endDist, Math::EPSILON );

      if( ratio > minRatio ) {
        minRatio   = ratio;
        lastNormal = normals[2*i + 1];
      }
    }
  }

  if( minRatio != -1.0f && minRatio <= maxRatio && minRatio < hit.ratio ) {
    hit.ratio    = max( 0.0f, minRatio );
    hit.normal   = lastNormal;
    hit.obj      = const_cast<Object*>( sObj );
    hit.str      = null;
    hit.entity   = null;
    hit.material = Material::OBJECT_BIT;
  }
}

// finds out if AABB-Brush collision occurs and the time when it occurs
void Collider::trimAABBBrush( const BSP::Brush* brush )
{
  float minRatio   = -1.0f;
  float maxRatio   = +1.0f;
  Vec3  lastNormal = Vec3::ZERO;

  for( int i = 0; i < brush->nSides; ++i ) {
    const Plane& plane = bsp->planes[ bsp->brushSides[brush->firstSide + i] ];

    float offset    = localDim * plane.abs();
    float startDist = startPos * plane - offset;
    float endDist   = endPos   * plane - offset;

    if( endDist > EPSILON ) {
      if( startDist < 0.0f ) {
        maxRatio = min( maxRatio, startDist / ( startDist - endDist ) );
      }
      else {
        return;
      }
    }
    else if( startDist >= 0.0f && endDist <= startDist ) {
      float ratio = ( startDist - EPSILON ) / max( startDist - endDist, Math::EPSILON );

      if( ratio > minRatio ) {
        minRatio   = ratio;
        lastNormal = plane.n();
      }
    }
  }
  if( minRatio != -1.0f && minRatio <= maxRatio && minRatio < hit.ratio ) {
    hit.ratio    = max( 0.0f, minRatio );
    hit.normal   = str->toAbsoluteCS( lastNormal );
    hit.obj      = null;
    hit.str      = const_cast<Struct*>( str );
    hit.entity   = const_cast<Struct::Entity*>( entity );
    hit.material = brush->material;
  }
}

// checks if AABB and Brush overlap and if AABB centre is inside a brush
void Collider::trimAABBWater( const BSP::Brush* brush )
{
  float depth = Math::INF;

  for( int i = 0; i < brush->nSides; ++i ) {
    const Plane& plane = bsp->planes[ bsp->brushSides[brush->firstSide + i] ];

    float offset = localDim * plane.abs();
    float dist   = startPos * plane - offset;

    if( dist >= 0.0f ) {
      return;
    }
    else if( plane.nz > 0.0f ) {
      float lowerDist = ( plane.d - startPos.x*plane.nx - startPos.y*plane.ny ) / plane.nz -
          startPos.z + aabb.dim.z;

      if( lowerDist > 0.0f ) {
        depth = min( depth, lowerDist );
      }
      else {
        return;
      }
    }
  }

  hard_assert( depth > 0.0f );

  hit.waterDepth = max( hit.waterDepth, depth );
  hit.medium |= Material::WATER_BIT;
}

// checks if AABB and Brush overlap and if AABB centre is inside a brush
void Collider::trimAABBLadder( const BSP::Brush* brush )
{
  for( int i = 0; i < brush->nSides; ++i ) {
    const Plane& plane = bsp->planes[ bsp->brushSides[brush->firstSide + i] ];

    float offset = localDim * plane.abs();
    float dist   = startPos * plane - offset;

    if( dist > 0.0f ) {
      return;
    }
  }

  hit.medium |= Material::LADDER_BIT;
}

// recursively check nodes of BSP-tree for AABB-Brush collisions
void Collider::trimAABBNode( int nodeIndex )
{
  if( nodeIndex < 0 ) {
    const BSP::Leaf& leaf = bsp->leaves[~nodeIndex];

    for( int i = 0; i < leaf.nBrushes; ++i ) {
      int index = bsp->leafBrushes[leaf.firstBrush + i];
      const BSP::Brush& brush = bsp->brushes[index];

      if( !visitBrush( index ) ) {
        if( brush.material & Material::STRUCT_BIT ) {
          trimAABBBrush( &brush );
        }
        else if( brush.material & Material::WATER_BIT ) {
          trimAABBWater( &brush );
        }
        else if( ( brush.material & Material::LADDER_BIT ) &&
                 obj != null && ( obj->flags & Object::CLIMBER_BIT ) )
        {
          trimAABBLadder( &brush );
        }
      }
    }
  }
  else {
    const BSP::Node& node  = bsp->nodes[nodeIndex];
    const Plane&     plane = bsp->planes[node.plane];

    float offset    = localDim * plane.abs() + 2.0f * EPSILON;
    float startDist = startPos * plane;
    float endDist   = endPos   * plane;

    if( startDist > offset && endDist > offset ) {
      trimAABBNode( node.front );
    }
    else if( startDist < -offset && endDist < -offset ) {
      trimAABBNode( node.back );
    }
    else {
      trimAABBNode( node.front );
      trimAABBNode( node.back );
    }
  }
}

void Collider::trimAABBEntities()
{
  if( str->nEntities == 0 ) {
    return;
  }

  Point3 originalStartPos = startPos;
  Point3 originalEndPos   = endPos;
  Bounds localTrace       = str->toStructCS( trace );

  for( int i = 0; i < bsp->nModels; ++i ) {
    model  = &bsp->models[i];
    entity = &str->entities[i];

    if( localTrace.overlaps( *model + entity->offset ) ) {
      for( int j = 0; j < model->nBrushes; ++j ) {
        int index = model->firstBrush + j;
        const BSP::Brush& brush = bsp->brushes[index];

        hard_assert( !visitBrush( index ) );

        startPos = originalStartPos - entity->offset;
        endPos   = originalEndPos   - entity->offset;

        trimAABBBrush( &brush );
      }
    }
  }

  startPos = originalStartPos;
  endPos   = originalEndPos;
}

// terrain collision detection is penetration-safe
void Collider::trimAABBTerraQuad( int x, int y )
{
  const Terra::Quad& quad     = orbis.terra.quads[x    ][y    ];
  const Terra::Quad& nextQuad = orbis.terra.quads[x + 1][y + 1];

  const Point3& minVert = quad.vertex;
  const Point3& maxVert = nextQuad.vertex;

  Vec3 localStartPos = startPos - minVert;
  Vec3 localEndPos   = endPos   - minVert;

  float startDist = localStartPos * quad.triNormal[0];
  float endDist   = localEndPos   * quad.triNormal[0];

  if( endDist <= EPSILON && endDist <= startDist ) {
    float ratio = max( 0.0f, startDist - EPSILON ) / max( startDist - endDist, Math::EPSILON );

    float impactX = startPos.x + ratio * move.x;
    float impactY = startPos.y + ratio * move.y;

    if( impactX - minVert.x >= impactY - minVert.y &&
        minVert.x <= impactX && impactX <= maxVert.x &&
        minVert.y <= impactY && impactY <= maxVert.y &&
        ratio < hit.ratio )
    {
      hit.ratio    = ratio;
      hit.normal   = quad.triNormal[0];
      hit.obj      = null;
      hit.str      = null;
      hit.entity   = null;
      hit.material = Material::TERRAIN_BIT;

      return;
    }
  }

  startDist = localStartPos * quad.triNormal[1];
  endDist   = localEndPos   * quad.triNormal[1];

  if( endDist <= EPSILON && endDist <= startDist ) {
    float ratio = max( 0.0f, startDist - EPSILON ) / max( startDist - endDist, Math::EPSILON );

    float impactX = startPos.x + ratio * move.x;
    float impactY = startPos.y + ratio * move.y;

    if( impactX - minVert.x <= impactY - minVert.y &&
        minVert.x <= impactX && impactX <= maxVert.x &&
        minVert.y <= impactY && impactY <= maxVert.y &&
        ratio < hit.ratio )
    {
      hit.ratio    = ratio;
      hit.normal   = quad.triNormal[1];
      hit.obj      = null;
      hit.str      = null;
      hit.entity   = null;
      hit.material = Material::TERRAIN_BIT;

      return;
    }
  }
}

void Collider::trimAABBTerra()
{
  if( startPos.z < 0.0f ) {
    hit.waterDepth = max( hit.waterDepth, -startPos.z );
    hit.medium |= Material::WATER_BIT;
  }

  float minPosX = min( startPos.x, endPos.x );
  float minPosY = min( startPos.y, endPos.y );
  float maxPosX = max( startPos.x, endPos.x );
  float maxPosY = max( startPos.y, endPos.y );

  Span terraSpan = orbis.terra.getInters( minPosX, minPosY, maxPosX, maxPosY );

  for( int x = terraSpan.minX; x <= terraSpan.maxX; ++x ) {
    for( int y = terraSpan.minY; y <= terraSpan.maxY; ++y ) {
      trimAABBTerraQuad( x, y );
    }
  }
}

// move AABB until first collisons occurs
void Collider::trimAABBOrbis()
{
  hit.ratio      = 1.0f;
  hit.obj        = null;
  hit.str        = null;
  hit.entity     = null;
  hit.medium     = 0;
  hit.material   = 0;
  hit.waterDepth = 0.0f;

  Point3 originalStartPos = aabb.p;
  Point3 originalEndPos   = aabb.p + move;

  startPos = originalStartPos;
  endPos   = originalEndPos;

  if( !orbis.includes( trace ) ) {
    trimAABBVoid();
  }

  const Struct* oldStr = null;

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( int i = 0; i < cell.structs.length(); ++i ) {
        str = orbis.structs[ cell.structs[i] ];

        // to prevent some of duplicated structure tests
        if( str != oldStr && str->overlaps( trace ) ) {
          visitedBrushes.clearAll();

          startPos = str->toStructCS( originalStartPos );
          endPos   = str->toStructCS( originalEndPos );
          localDim = str->swapDimCS( aabb.dim );
          bsp      = str->bsp;
          entity   = null;

          trimAABBNode( 0 );
          trimAABBEntities();

          oldStr = str;
        }
      }

      startPos = originalStartPos;
      endPos   = originalEndPos;

      for( const Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( sObj != exclObj && ( sObj->flags & mask ) && sObj->overlaps( trace ) ) {
          trimAABBObj( sObj );
        }
      }
    }
  }

  startPos.z -= aabb.dim.z;
  endPos.z   -= aabb.dim.z;

  trimAABBTerra();

  hard_assert( 0.0f <= hit.ratio && hit.ratio <= 1.0f );
  hard_assert( ( ( hit.material & Material::OBJECT_BIT ) != 0 ) == ( hit.obj != null ) );
}

//***********************************
//*          OVERLAPPING            *
//***********************************

// get all objects and structures that overlap with our trace
void Collider::getOrbisOverlaps( Vector<Object*>* objects, Vector<Struct*>* structs )
{
  hard_assert( objects != null || structs != null );

  const Struct* oldStr = null;

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      if( structs != null ) {
        for( int i = 0; i < cell.structs.length(); ++i ) {
          Struct* str = orbis.structs[ cell.structs[i] ];

          if( str != oldStr && str->overlaps( trace ) && !structs->contains( str ) ) {
            visitedBrushes.clearAll();

            startPos = str->toStructCS( aabb.p );
            localDim = str->swapDimCS( aabb.dim );
            bsp      = str->bsp;

            if( overlapsAABBNode( 0 ) || overlapsAABBEntities() ) {
              structs->add( str );
            }
          }

          oldStr = str;
        }
      }

      if( objects != null ) {
        for( Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
          if( ( sObj->flags & mask ) && sObj->overlaps( trace ) ) {
            objects->add( sObj );
          }
        }
      }
    }
  }
}

// get all objects which are included in our trace
void Collider::getOrbisIncludes( Vector<Object*>* objects ) const
{
  hard_assert( objects != null );

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( ( sObj->flags & mask ) && trace.includes( *sObj ) ) {
          objects->add( sObj );
        }
      }
    }
  }
}

void Collider::touchOrbisOverlaps() const
{
  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( ( sObj->flags & Object::DYNAMIC_BIT ) && sObj->overlaps( trace ) ) {
          sObj->flags &= ~Object::MOVE_CLEAR_MASK;
        }
      }
    }
  }
}

void Collider::getEntityOverlaps( Vector<Object*>* objects )
{
  hard_assert( objects != null );

  for( int x = span.minX; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      const Cell& cell = orbis.cells[x][y];

      for( Object* sObj = cell.objects.first(); sObj != null; sObj = sObj->next[0] ) {
        if( ( sObj->flags & mask ) && sObj->overlaps( trace ) ) {
          startPos = str->toStructCS( sObj->p ) - entity->offset;
          localDim = str->swapDimCS( sObj->dim + Vec3( margin, margin, margin ) );

          for( int i = 0; i < model->nBrushes; ++i ) {
            const BSP::Brush& brush = bsp->brushes[model->firstBrush + i];

            if( overlapsAABBBrush( &brush ) ) {
              objects->add( sObj );
            }
          }
        }
      }
    }
  }
}

//***********************************
//*            PUBLIC               *
//***********************************

bool Collider::overlaps( const Point3& point, const Object* exclObj_ )
{
  aabb = AABB( point, Vec3::ZERO );
  exclObj = exclObj_;
  flags = Object::CYLINDER_BIT;

  span = orbis.getInters( point, AABB::MAX_DIM );

  return overlapsAABBOrbis();
}

bool Collider::overlapsOO( const Point3& point, const Object* exclObj_ )
{
  aabb = AABB( point, Vec3::ZERO );
  exclObj = exclObj_;
  flags = Object::CYLINDER_BIT;

  span = orbis.getInters( point, AABB::MAX_DIM );

  return overlapsAABBOrbisOO();
}

bool Collider::overlapsOSO( const Point3& point, const Object* exclObj_ )
{
  aabb = AABB( point, Vec3::ZERO );
  exclObj = exclObj_;
  flags = Object::CYLINDER_BIT;

  span = orbis.getInters( point, AABB::MAX_DIM );

  return overlapsAABBOrbisOSO();
}

bool Collider::overlaps( const AABB& aabb_, const Object* exclObj_ )
{
  aabb = aabb_;
  exclObj = exclObj_;
  flags = 0;

  trace = aabb.toBounds( 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsAABBOrbis();
}

bool Collider::overlapsOO( const AABB& aabb_, const Object* exclObj_ )
{
  aabb = aabb_;
  exclObj = exclObj_;
  flags = 0;

  trace = aabb.toBounds( 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsAABBOrbisOO();
}

bool Collider::overlapsOSO( const AABB& aabb_, const Object* exclObj_ )
{
  aabb = aabb_;
  exclObj = exclObj_;
  flags = 0;

  trace = aabb.toBounds( 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsAABBOrbisOSO();
}

bool Collider::overlaps( const Object* obj_, const Object* exclObj_ )
{
  aabb = *obj_;
  exclObj = exclObj_;
  flags = obj_->flags;

  trace = aabb.toBounds( 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsAABBOrbis();
}

bool Collider::overlapsOO( const Object* obj_, const Object* exclObj_ )
{
  aabb = *obj_;
  exclObj = exclObj_;
  flags = obj_->flags;

  trace = aabb.toBounds( 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsAABBOrbisOO();
}

bool Collider::overlapsOSO( const Object* obj_, const Object* exclObj_ )
{
  aabb = *obj_;
  exclObj = exclObj_;
  flags = obj_->flags;

  trace = aabb.toBounds( 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsAABBOrbisOSO();
}

bool Collider::overlapsOO( const Struct::Entity* entity_, float margin_ )
{
  str = entity_->str;
  entity = entity_;
  bsp = entity_->model->bsp;
  model = entity_->model;
  margin = margin_;

  Bounds bounds = Bounds( model->mins - Vec3( margin, margin, margin ) + entity->offset,
                          model->maxs + Vec3( margin, margin, margin ) + entity->offset );

  trace = str->toAbsoluteCS( bounds );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  return overlapsEntityOrbisOO();
}

void Collider::getOverlaps( const AABB& aabb_, Vector<Object*>* objects,
                            Vector<Struct*>* structs, float eps )
{
  aabb = aabb_;
  exclObj = null;

  trace = aabb.toBounds( eps );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  getOrbisOverlaps( objects, structs );
}

void Collider::getIncludes( const AABB& aabb_, Vector<Object*>* objects, float eps )
{
  aabb = aabb_;
  exclObj = null;

  trace = aabb.toBounds( eps );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  getOrbisIncludes( objects );
}

void Collider::touchOverlaps( const AABB& aabb_, float eps )
{
  aabb = aabb_;
  exclObj = null;

  trace = aabb.toBounds( eps );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  touchOrbisOverlaps();
}

void Collider::getOverlaps( const Struct::Entity* entity_, Vector<Object*>* objects,
                            float margin_ )
{
  str = entity_->str;
  entity = entity_;
  bsp = entity_->model->bsp;
  model = entity_->model;
  margin = margin_;

  Bounds bounds = Bounds( model->mins - Vec3( margin, margin, margin ) + entity->offset,
                          model->maxs + Vec3( margin, margin, margin ) + entity->offset );

  trace = str->toAbsoluteCS( bounds );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  getEntityOverlaps( objects );
}

void Collider::translate( const Point3& point, const Vec3& move_, const Object* exclObj_ )
{
  aabb = AABB( point, Vec3::ZERO );
  move = move_;
  exclObj = exclObj_;
  flags = Object::CYLINDER_BIT;

  trace.fromPointMove( point, move, 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  trimAABBOrbis();
}

void Collider::translate( const AABB& aabb_, const Vec3& move_, const Object* exclObj_ )
{
  obj = null;
  aabb = aabb_;
  move = move_;
  exclObj = exclObj_;
  flags = 0;

  trace.fromAABBMove( aabb, move, 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  trimAABBOrbis();
}

void Collider::translate( const Dynamic* obj_, const Vec3& move_ )
{
  hard_assert( obj_->cell != null );

  obj = obj_;
  aabb = *obj_;
  move = move_;
  exclObj = obj_;
  flags = obj_->flags;

  trace.fromAABBMove( aabb, move, 4.0f * EPSILON );
  span = orbis.getInters( trace, AABB::MAX_DIM );

  trimAABBOrbis();
}

}
}
