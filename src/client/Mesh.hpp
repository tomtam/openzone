/*
 *  Mesh.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "stable.hpp"

#include "client/common.hpp"

namespace oz
{
namespace client
{

  class MeshData;
  class Compiler;

  struct Vertex
  {
    Point3   pos;
    TexCoord texCoord;
    Vec3     normal;
#ifdef OZ_BUMPMAP
    Vec3     tangent;
    Vec3     binormal;
#endif

    Vertex() = default;

    explicit Vertex( const Point3& pos,
                     const TexCoord& texCoord = TexCoord( 0.0f, 0.0f ),
                     const Vec3& normal = Vec3::ZERO,
                     const Vec3& tangent = Vec3::ZERO,
                     const Vec3& binormal = Vec3::ZERO );

    bool operator == ( const Vertex& v ) const;

    void read( InputStream* stream );
    void write( OutputStream* stream ) const;
  };

  class Mesh
  {
    friend class MeshData;
    friend class Compiler;

    public:

      static const int FIRST_ALPHA_PART_MASK = 0x00ff;
      static const int SOLID_BIT             = 0x0100;
      static const int ALPHA_BIT             = 0x0200;
      static const int EMBEDED_TEX_BIT       = 0x0400;

    private:

      struct Part
      {
        uint   texture;
        float  alpha;
        float  specular;

        int    mode;

        int    nIndices;
        int    firstIndex;
      };

      uint         vao;
      uint         ibo;
      uint         vbo;

      int          flags;
      DArray<int>  texIds;
      DArray<Part> parts;

    public:

      Mesh();
      ~Mesh();

      void load( InputStream* stream, int usage );
      void unload();

      void upload( const Vertex* vertices, int nVertices, int flags = 0 ) const;
      Vertex* map( int access ) const;
      void unmap() const;

      void draw( int mask ) const;

  };

#ifdef OZ_BUILD_TOOLS
  class MeshData
  {
    friend class Compiler;

    public:

      struct Part
      {
        String texture;
        float  alpha;
        float  specular;

        int    mode;

        int    nIndices;
        int    firstIndex;
      };

      Vector<Part>   solidParts;
      Vector<Part>   alphaParts;

      DArray<ushort> indices;
      DArray<Vertex> vertices;

      void write( OutputStream* stream, bool embedTextures = true ) const;

  };
#endif

}
}
