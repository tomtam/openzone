/*
 *  Terra.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "matrix/Terra.hpp"

#include "client/Mesh.hpp"

namespace oz
{
namespace client
{

class Terra
{
  public:

    static const int   TILE_QUADS = 16;
    static const int   TILES      = matrix::Terra::QUADS / TILE_QUADS;

  private:

    static const int   TILE_INDICES;
    static const int   TILE_VERTICES;

    static const float TILE_SIZE;
    static const float TILE_INV_SIZE;

    static const float WAVE_BIAS_INC;

    uint  vaos[TILES][TILES];
    uint  vbos[TILES][TILES];
    uint  ibo;

    uint  waterTexId;
    uint  detailTexId;
    uint  mapTexId;

    int   landShaderId;
    int   waterShaderId;

    float waveBias;

    Span span;
    SBitset<TILES * TILES> waterTiles;

  public:

    int id;

    Terra();

    void draw();
    void drawWater();

    void load();
    void unload();

};

extern Terra terra;

}
}
