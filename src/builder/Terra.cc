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
 * @file builder/Terra.cc
 */

#include <builder/Terra.hh>

#include <client/Terra.hh>
#include <builder/Context.hh>

#include <FreeImage.h>

namespace oz
{
namespace builder
{

void Terra::load()
{
  File configFile( "@terra/" + name + ".json" );
  File imageFile( "@terra/" + name + ".png" );

  JSON config;
  if( !config.load( configFile ) ) {
    OZ_ERROR( "Failed to loa terra configuration '%s'", configFile.path().cstr() );
  }

  float minHeight = config["minHeight"].get( -1000.0f );
  float maxHeight = config["maxHeight"].get( +1000.0f );

  String sLiquid = config["liquid"].get( "WATER" );

  if( sLiquid.equals( "WATER" ) ) {
    liquid = Medium::WATER_BIT | Medium::SEA_BIT;
  }
  else if( sLiquid.equals( "LAVA" ) ) {
    liquid = Medium::LAVA_BIT | Medium::SEA_BIT;
  }
  else {
    OZ_ERROR( "Liquid should be either WATER or LAVA" );
  }

  liquidColour   = config["liquidFogColour"].get( Vec4( 0.00f, 0.05f, 0.20f, 1.00f ) );
  liquidTexture  = config["liquidTexture"].get( "" );
  detailTexture  = config["detailTexture"].get( "" );
  mapTexture     = config["mapTexture"].get( "" );

  config.clear( true );

  Log::print( "Loading terrain heightmap '%s' ...", name.cstr() );

  String realPath = imageFile.realPath();

  FIBITMAP* image = FreeImage_Load( FIF_PNG, realPath );
  if( image == nullptr ) {
    OZ_ERROR( "Failed to load heightmap '%s'", realPath.cstr() );
  }

  imageWidth  = int( FreeImage_GetWidth( image ) );
  imageHeight = int( FreeImage_GetHeight( image ) );
  int bpp     = int( FreeImage_GetBPP( image ) );
  int type    = int( FreeImage_GetImageType( image ) );

  if( type != FIT_RGB16 || bpp != 48 || imageWidth <= 0 || imageHeight <= 0 ||
      imageWidth > matrix::Terra::VERTS || imageHeight > matrix::Terra::VERTS )
  {
    OZ_ERROR( "Invalid terrain heightmap format %d x %d %d bpp, should be at most %d x %d and"
              " 48 bpp RGB (red channel is used as height, green and blue are ignored)",
              imageWidth, imageHeight, bpp, matrix::Terra::VERTS, matrix::Terra::VERTS );
  }

  Log::printEnd( " OK" );
  Log::print( "Calculating triangles ..." );

  for( int y = matrix::Terra::VERTS - 1; y >= 0; --y ) {
    for( int x = 0; x < matrix::Terra::VERTS; ++x ) {
      quads[x][y].vertex.x     = float( x * matrix::Terra::Quad::SIZE - matrix::Terra::DIM );
      quads[x][y].vertex.y     = float( y * matrix::Terra::Quad::SIZE - matrix::Terra::DIM );
      quads[x][y].vertex.z     = 0.0f;
      quads[x][y].triNormal[0] = Vec3::ZERO;
      quads[x][y].triNormal[1] = Vec3::ZERO;
    }
  }

  int minVertX = ( matrix::Terra::VERTS - imageWidth ) / 2;
  int minVertY = ( matrix::Terra::VERTS - imageHeight ) / 2;

  for( int y = imageHeight - 1; y >= 0; --y ) {
    const ushort* pixel = reinterpret_cast<const ushort*>( FreeImage_GetScanLine( image, y ) );

    for( int x = 0; x < imageWidth; ++x ) {
      float value = float( *pixel ) / float( USHRT_MAX );

      quads[minVertX + x][minVertY + y].vertex.z = Math::mix( minHeight, maxHeight, value );

      pixel += 3;
    }
  }

  for( int x = 0; x < matrix::Terra::QUADS; ++x ) {
    for( int y = 0; y < matrix::Terra::QUADS; ++y ) {
      if( x != matrix::Terra::QUADS && y != matrix::Terra::QUADS ) {
        //
        // 0. triangle -- upper left
        // 1. triangle -- lower right
        //
        //    |  ...  |         D        C
        //    +---+---+-         o----->o
        //    |1 /|1 /|          |      ^
        //    | / | / |          |      |
        //    |/ 0|/ 0|          |      |
        //    +---+---+- ...     v      |
        //    |1 /|1 /|          o<-----o
        //    | / | / |         A        B
        //    |/ 0|/ 0|
        //  (0,0)
        //
        const Point& a = quads[x    ][y    ].vertex;
        const Point& b = quads[x + 1][y    ].vertex;
        const Point& c = quads[x + 1][y + 1].vertex;
        const Point& d = quads[x    ][y + 1].vertex;

        quads[x][y].triNormal[0] = ~( ( c - b ) ^ ( a - b ) );
        quads[x][y].triNormal[1] = ~( ( a - d ) ^ ( c - d ) );
      }
    }
  }

  FreeImage_Unload( image );

  Log::printEnd( " OK" );
}

void Terra::saveMatrix()
{
  File destFile( "terra/" + name + ".ozTerra" );

  Log::print( "Dumping terrain structure to '%s' ...", destFile.path().cstr() );

  OutputStream os( 0 );

  os.writeInt( matrix::Terra::VERTS );

  for( int x = 0; x < matrix::Terra::VERTS; ++x ) {
    for( int y = 0; y < matrix::Terra::VERTS; ++y ) {
      os.writeFloat( quads[x][y].vertex.z );
    }
  }

  os.writeInt( liquid );

  if( !destFile.write( os.begin(), os.tell() ) ) {
    OZ_ERROR( "Failed to write '%s'", destFile.path().cstr() );
  }

  Log::printEnd( " OK" );
}

void Terra::saveClient()
{
  File destFile( "terra/" + name + ".ozcTerra" );

  Log::println( "Compiling terrain model to '%s' {", destFile.path().cstr() );
  Log::indent();

  Context::Texture liquidTex = context.loadTexture( "@terra/" + liquidTexture );
  Context::Texture detailTex = context.loadTexture( "@terra/" + detailTexture );
  Context::Texture mapTex    = context.loadTexture( "@terra/" + mapTexture );

  OutputStream os( 0 );

  liquidTex.write( &os );
  detailTex.write( &os );
  mapTex.write( &os );

  // generate index buffer
  int index = 0;
  for( int x = 0; x < client::Terra::TILE_QUADS; ++x ) {
    if( x != 0 ) {
      os.writeUShort( ushort( index + client::Terra::TILE_QUADS + 1 ) );
    }
    for( int y = 0; y <= client::Terra::TILE_QUADS; ++y ) {
      os.writeUShort( ushort( index + client::Terra::TILE_QUADS + 1 ) );
      os.writeUShort( ushort( index ) );
      ++index;
    }
    if( x != client::Terra::TILE_QUADS - 1 ) {
      os.writeUShort( ushort( index - 1 ) );
    }
  }

  // generate vertex buffers
  Bitset waterTiles( client::Terra::TILES * client::Terra::TILES );
  waterTiles.clearAll();

  for( int i = 0; i < client::Terra::TILES; ++i ) {
    for( int j = 0; j < client::Terra::TILES; ++j ) {
      // tile
      for( int k = 0; k <= client::Terra::TILE_QUADS; ++k ) {
        for( int l = 0; l <= client::Terra::TILE_QUADS; ++l ) {
          int x = i * client::Terra::TILE_QUADS + k;
          int y = j * client::Terra::TILE_QUADS + l;

          Vec3 normal = Vec3::ZERO;

          if( x < matrix::Terra::QUADS && y < matrix::Terra::QUADS ) {
            normal += quads[x][y].triNormal[0];
            normal += quads[x][y].triNormal[1];
          }
          if( x > 0 && y < matrix::Terra::QUADS ) {
            normal += quads[x - 1][y].triNormal[0];
          }
          if( x > 0 && y > 0 ) {
            normal += quads[x - 1][y - 1].triNormal[0];
            normal += quads[x - 1][y - 1].triNormal[1];
          }
          if( x < matrix::Terra::QUADS && y > 0 ) {
            normal += quads[x][y - 1].triNormal[1];
          }
          normal = ~normal;

          if( ( quads[x][y].vertex.z < 0.0f ) ||
              ( x + 1 < matrix::Terra::VERTS && quads[x + 1][y].vertex.z < 0.0f ) ||
              ( y + 1 < matrix::Terra::VERTS && quads[x][y + 1].vertex.z < 0.0f ) ||
              ( x + 1 < matrix::Terra::VERTS && y + 1 < matrix::Terra::VERTS &&
                quads[x + 1][y + 1].vertex.z < 0.0f ) )
          {
            waterTiles.set( i * client::Terra::TILES + j );
          }

          os.writeByte( byte( normal.x * 127.0f ) );
          os.writeByte( byte( normal.y * 127.0f ) );
          os.writeByte( byte( normal.z * 127.0f ) );
        }
      }
    }
  }

  for( int i = 0; i < waterTiles.length(); ++i ) {
    os.writeBool( waterTiles.get( i ) );
  }

  os.writeVec4( liquidColour );

  if( !destFile.write( os.begin(), os.tell() ) ) {
    OZ_ERROR( "Failed to write '%s'", destFile.path().cstr() );
  }

  Log::unindent();
  Log::println( "}" );

  File minimapIn( "@terra/" + mapTexture );
  File minimapOut( "terra/" + name + ".dds" );

  Log::print( "Writing minimap texture '%s' ...", minimapOut.path().cstr() );

  os.rewind();
  if( !Builder::buildDDS( minimapIn, 0, &os ) ) {
    OZ_ERROR( "Minimap texture '%s' loading failed", minimapIn.path().cstr() );
  }

  if( !minimapOut.write( os.begin(), os.tell() ) ) {
    OZ_ERROR( "Minimap texture '%s' writing failed", minimapOut.path().cstr() );
  }

  Log::printEnd( " OK" );
}

void Terra::build( const char* name_ )
{
  name = name_;

  load();
  saveMatrix();
  saveClient();

  name          = "";
  liquidTexture = "";
  detailTexture = "";
  mapTexture    = "";
}

Terra terra;

}
}
