/*
 *  BSP.h
 *
 *  Data structure and loaders for Internal BSP, Quake3 BSP and OpenBSP formats
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

#include "bv.h"

namespace oz
{

  struct BSP : Bounds
  {

    static const int LIGHTMAP_DIM = 128;
    static const int LIGHTMAP_BPP = 3;
    static const int LIGHTMAP_SIZE = LIGHTMAP_DIM * LIGHTMAP_DIM * LIGHTMAP_BPP;

    static const int SOLID_BIT  = 0x01;
    static const int SLICK_BIT  = 0x02;
    static const int WATER_BIT  = 0x04;
    static const int LADDER_BIT = 0x08;

    struct Plane
    {
      Vec3  normal;
      float distance;
    };

    struct Node
    {
      int plane;

      int front;
      int back;
    };

    struct Leaf : Bounds
    {
      int cluster;

      int firstFace;
      int nFaces;

      int firstBrush;
      int nBrushes;
    };

    struct Brush
    {
      int firstSide;
      int nSides;
      int content;
    };

    struct Vertex
    {
      Vec3  p;
      float texCoord[2];
      float lightmapCoord[2];
    };

    struct Face
    {
      int  texture;
      int  lightmap;
      int  content;

      Vec3 normal;

      int  firstVertex;
      int  nVertices;

      int  firstIndex;
      int  nIndices;
    };

    struct Lightmap
    {
      char bits[LIGHTMAP_SIZE];
    };

    struct VisualData
    {
      int    nClusters;
      int    clusterLength;
      Bitset *bitsets;

      VisualData() : bitsets( null )
      {}

      ~VisualData()
      {
        if( bitsets != null ) {
          delete[] bitsets;
        }
      }
    };

    float      maxDim;

    int        nTextures;
    int        nLeafs;
    int        nFaces;
    int        nLightmaps;

    int        *textures;
    Plane      *planes;

    Node       *nodes;
    Leaf       *leafs;
    int        *leafFaces;
    int        *leafBrushes;

    Brush      *brushes;
    int        *brushSides;

    Vertex     *vertices;
    int        *indices;
    Face       *faces;
    Lightmap   *lightmaps;

    VisualData visual;

    explicit BSP();
    ~BSP();

    bool loadQBSP( const char *fileName, float scale, float maxDim );
    bool load( const char *name );
    void free();
  };

}