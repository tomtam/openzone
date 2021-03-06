/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2016 Davorin Učakar
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
 * @file matrix/Collider.hh
 */

#pragma once

#include <matrix/Orbis.hh>

namespace oz
{

struct Material
{
  static const int NONE        = 0;      ///< No material.

  static const int VOID_BIT    = 0x0080; ///< World bounds, invisible walls preventing you to leave.
  static const int TERRAIN_BIT = 0x0001; ///< Terrain.
  static const int STRUCT_BIT  = 0x0002; ///< Structure.
  static const int SLICK_BIT   = 0x0004; ///< Slick brush in a structure.
  static const int OBJECT_BIT  = 0x0008; ///< Object.

  static const int MASK        = 0x00ff; ///< Material mask (to distinguish from Medium bits when
                                         ///< used together in same variable).
};

struct Medium
{
  static const int NONE        = 0;      ///< No medium.

  static const int LADDER_BIT  = 0x0100; ///< Overlapping with BSP ladder brush.
  static const int AIR_BIT     = 0x0200; ///< Overlapping with BSP air brush.
  static const int WATER_BIT   = 0x0400; ///< Overlapping with BSP water brush or overlapping with
                                         ///< terrain water sea but not BSP air brush.
  static const int LAVA_BIT    = 0x0800; ///< Overlapping with BSP lava brush or overlapping with
                                         ///< terrain lava sea but not BSP air brush.
  static const int SEA_BIT     = 0x1000; ///< Overlapping with terrain water/lava sea but not BSP
                                         ///< air brush or overlapping with BSP sea brush.

  static const int LIQUID_MASK = 0x0c00; ///< Mask for liquids (water or lava bit, but not sea).
  static const int MASK        = 0xff00; ///< Medium mask (to distinguish from Material bits when
                                         ///< used together in same variable).
};

struct Hit
{
  Vec3    normal    = Vec3::ZERO;

  Object* obj       = nullptr;
  Struct* str       = nullptr;
  Entity* entity    = nullptr;

  Struct* mediumStr = nullptr;

  float   ratio     = 1.0f;

  int     material  = Material::NONE;
  int     medium    = Medium::NONE;
  float   depth     = 0.0f;
};

class Collider
{
private:

  SBitset<Orbis::MAX_STRUCTS> visitedStructs;
  SBitset<BSP::MAX_BRUSHES>   visitedBrushes;

  Span                        span;
  Bounds                      trace;
  Vec3                        move;

  AABB                        aabb;
  Point                       startPos;
  Point                       endPos;
  Vec3                        localDim;

  const Dynamic*              obj;
  const Object*               exclObj;
  const Entity*               entity;
  const Struct*               str;
  const BSP*                  bsp;

  int                         flags;
  float                       margin;

public:

  int                         mask = Object::SOLID_BIT; /// Filter for `Object::flags`.
  Hit                         hit;                      /// Collision feedback data.

private:

  /**
   * Return true if brush was already visited and mark it visited.
   */
  bool visitBrush(int index);

  bool overlapsAABBObj(const Object* sObj) const;
  bool overlapsAABBBrush(const BSP::Brush* brush) const;
  bool overlapsAABBNode(int nodeIndex);
  bool overlapsAABBEntities();
  bool overlapsAABBOrbis();

  bool overlapsEntityObjects();

  void trimAABBVoid();
  void trimAABBObj(const Object* sObj);
  void trimAABBBrush(const BSP::Brush* brush);
  void trimAABBLiquid(const BSP::Brush* brush);
  void trimAABBArea(const BSP::Brush* brush);
  void trimAABBNode(int nodeIndex);
  void trimAABBEntities();

  void trimAABBTerraQuad(int x, int y);
  void trimAABBTerra();
  void trimAABBOrbis();

  void trimEntityObjects();

  void getOrbisOverlaps(List<Struct*>* structs, List<Object*>* objects);
  void getEntityOverlaps(List<Object*>* objects);

public:

  bool overlaps(const Point& point, const Object* exclObj = nullptr);
  bool overlaps(const AABB& aabb, const Object* exclObj = nullptr);
  bool overlaps(const Object* obj);
  bool overlaps(const Entity* entity, float margin = 0.0f);

  void translate(const Point& point, const Vec3& move, const Object* exclObj = nullptr);
  void translate(const AABB& aabb, const Vec3& move, const Object* exclObj = nullptr);
  void translate(const Dynamic* obj, const Vec3& move);
  void translate(const Entity* entity, const Vec3& localMove);

  // fill given vectors with objects and structures overlapping with the AABB
  // if either vector is nullptr the respective test is not performed
  void getOverlaps(const AABB& aabb, List<Struct*>* structs, List<Object*>* objects, float margin);
  void getOverlaps(const Entity* entity, List<Object*>* objects, float margin);

};

extern Collider collider;

}
