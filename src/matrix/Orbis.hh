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
 * @file matrix/Orbis.hh
 */

#pragma once

#include <matrix/Caelum.hh>
#include <matrix/Terra.hh>
#include <matrix/Struct.hh>
#include <matrix/Dynamic.hh>
#include <matrix/Frag.hh>

namespace oz
{

struct Cell
{
  static const int SIZE = 16;

  SList<short, 6> structs;
  Chain<Object>   objects;
  Chain<Frag>     frags;
};

/**
 * Matrix data structure for world (terrain, all structures and objects in the world).
 * The world should not be manipulated directly; use Synapse instead.
 */
class Orbis : public Bounds
{
  friend class Synapse;

  private:

    static const int MAX_STRUCTS = 1 << 14;
    static const int MAX_OBJECTS = 1 << 16;
    static const int MAX_FRAGS   = 1 << 14;

  public:

    // # of cells on each (x, y) axis
    static const int DIM   = MAX_WORLD_COORD;
    static const int CELLS = 2 * DIM / Cell::SIZE;

    Caelum caelum;
    Terra  terra;
    Cell   cells[CELLS][CELLS];

  private:

    SList<Struct*, MAX_STRUCTS> structs;
    SList<Object*, MAX_OBJECTS> objects;
    SList<Frag*,   MAX_FRAGS>   frags;

  private:

    bool position( Struct* str );
    void unposition( Struct* str );

    void position( Object* obj );
    void unposition( Object* obj );

    void position( Frag* frag );
    void unposition( Frag* frag );

    Struct* add( const BSP* bsp, const Point& p, Heading heading );
    Object* add( const ObjectClass* clazz, const Point& p, Heading heading );
    Frag*   add( const FragPool* pool, const Point& p, const Vec3& velocity );

    void remove( Struct* str );
    void remove( Object* obj );
    void remove( Frag* frag );

  public:

    void reposition( Object* obj );
    void reposition( Frag* frag );

    OZ_ALWAYS_INLINE
    int nStructs() const;

    OZ_ALWAYS_INLINE
    int nObjects() const;

    OZ_ALWAYS_INLINE
    int nFrags() const;

    /**
     * Return structure at a given index, nullptr if index is -1.
     */
    OZ_ALWAYS_INLINE
    Struct* str( int index ) const;

    /**
     * Return entity with a given index, nullptr if index is -1 or structure does not exist.
     */
    OZ_ALWAYS_INLINE
    Entity* ent( int index ) const;

    /**
     * Return object at a given index, nullptr if index is -1.
     */
    OZ_ALWAYS_INLINE
    Object* obj( int index ) const;

    /**
     * Return fragment at a given index, nullptr if index is -1.
     */
    OZ_ALWAYS_INLINE
    Frag* frag( int index ) const;

    /**
     * Adjust structure index; return -1 if non-existent, original index otherwise.
     */
    OZ_ALWAYS_INLINE
    int strIndex( int index ) const;

    /**
     * Adjust entity index; return -1 if structure does not exist, original index otherwise.
     */
    OZ_ALWAYS_INLINE
    int entIndex( int index ) const;

    /**
     * Adjust object index; return -1 if non-existent, original index otherwise.
     */
    OZ_ALWAYS_INLINE
    int objIndex( int index ) const;

    /**
     * Adjust fragment index; return -1 if non-existent, original index otherwise.
     */
    OZ_ALWAYS_INLINE
    int fragIndex( int index ) const;

    // get pointer to the cell the point is in
    Cell* getCell( float x, float y );
    Cell* getCell( const Point& p );

    // get indices of the cell the point is in
    Span getInters( float x, float y, float epsilon = 0.0f ) const;
    Span getInters( const Point& p, float epsilon = 0.0f ) const;

    // get indices of min and max cells which the area intersects
    Span getInters( float minPosX, float minPosY, float maxPosX, float maxPosY,
                    float epsilon = 0.0f ) const;

    // get indices of min and max cells which the AABB intersects
    Span getInters( const AABB& bb, float epsilon = 0.0f ) const;

    // get indices of min and max cells which the bounds intersects
    Span getInters( const Bounds& bounds, float epsilon = 0.0f ) const;

    void update();

    void read( InputStream* istream );
    void read( const JSON& json );

    void write( OutputStream* ostream ) const;
    JSON write() const;

    void load();
    void unload();

    void init();
    void destroy();

};

extern Orbis orbis;

OZ_ALWAYS_INLINE
inline int Orbis::nStructs() const
{
  return structs.length() - 1;
}

OZ_ALWAYS_INLINE
inline int Orbis::nObjects() const
{
  return objects.length() - 1;
}

OZ_ALWAYS_INLINE
inline int Orbis::nFrags() const
{
  return frags.length() - 1;
}

OZ_ALWAYS_INLINE
inline Struct* Orbis::str( int index ) const
{
  return structs[1 + index];
}

OZ_ALWAYS_INLINE
inline Entity* Orbis::ent( int index ) const
{
  if( index < 0 ) {
    return nullptr;
  }

  Struct* str = structs[1 + index / Struct::MAX_ENTITIES];
  if( str == nullptr ) {
    return nullptr;
  }
  else {
    return &str->entities[index % Struct::MAX_ENTITIES];
  }
}

OZ_ALWAYS_INLINE
inline Object* Orbis::obj( int index ) const
{
  return objects[1 + index];
}

OZ_ALWAYS_INLINE
inline Frag* Orbis::frag( int index ) const
{
  return frags[1 + index];
}

OZ_ALWAYS_INLINE
inline int Orbis::strIndex( int index ) const
{
  return structs[1 + index] == nullptr ? -1 : index;
}

OZ_ALWAYS_INLINE
inline int Orbis::entIndex( int index ) const
{
  if( index < 0 ) {
    return -1;
  }

  Struct* str = structs[1 + index / Struct::MAX_ENTITIES];
  return str == nullptr ? -1 : index;
}

OZ_ALWAYS_INLINE
inline int Orbis::objIndex( int index ) const
{
  return objects[1 + index] == nullptr ? -1 : index;
}

OZ_ALWAYS_INLINE
inline int Orbis::fragIndex( int index ) const
{
  return frags[1 + index] == nullptr ? -1 : index;
}

OZ_ALWAYS_INLINE
inline Cell* Orbis::getCell( float x, float y )
{
  int ix = int( ( x + Orbis::DIM ) / Cell::SIZE );
  int iy = int( ( y + Orbis::DIM ) / Cell::SIZE );

  ix = clamp( ix, 0, Orbis::CELLS - 1 );
  iy = clamp( iy, 0, Orbis::CELLS - 1 );

  return &cells[ix][iy];
}

OZ_ALWAYS_INLINE
inline Cell* Orbis::getCell( const Point& p )
{
  return getCell( p.x, p.y );
}

OZ_ALWAYS_INLINE
inline Span Orbis::getInters( float x, float y, float epsilon ) const
{
  return {
    max( int( ( x - epsilon + Orbis::DIM ) / Cell::SIZE ), 0 ),
    max( int( ( y - epsilon + Orbis::DIM ) / Cell::SIZE ), 0 ),
    min( int( ( x + epsilon + Orbis::DIM ) / Cell::SIZE ), Orbis::CELLS - 1 ),
    min( int( ( y + epsilon + Orbis::DIM ) / Cell::SIZE ), Orbis::CELLS - 1 )
  };
}

OZ_ALWAYS_INLINE
inline Span Orbis::getInters( const Point& p, float epsilon ) const
{
  return getInters( p.x, p.y, epsilon );
}

OZ_ALWAYS_INLINE
inline Span Orbis::getInters( float minPosX, float minPosY,
                              float maxPosX, float maxPosY, float epsilon ) const
{
  return {
    max( int( ( minPosX - epsilon + Orbis::DIM ) / Cell::SIZE ), 0 ),
    max( int( ( minPosY - epsilon + Orbis::DIM ) / Cell::SIZE ), 0 ),
    min( int( ( maxPosX + epsilon + Orbis::DIM ) / Cell::SIZE ), Orbis::CELLS - 1 ),
    min( int( ( maxPosY + epsilon + Orbis::DIM ) / Cell::SIZE ), Orbis::CELLS - 1 )
  };
}

OZ_ALWAYS_INLINE
inline Span Orbis::getInters( const AABB& bb, float epsilon ) const
{
  return getInters( bb.p.x - bb.dim.x, bb.p.y - bb.dim.y,
                    bb.p.x + bb.dim.x, bb.p.y + bb.dim.y,
                    epsilon );
}

OZ_ALWAYS_INLINE
inline Span Orbis::getInters( const Bounds& bounds, float epsilon ) const
{
  return getInters( bounds.mins.x, bounds.mins.y, bounds.maxs.x, bounds.maxs.y, epsilon );
}

}
