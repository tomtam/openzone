/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
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

/*
 * terraLiquid.vert
 *
 * Terrain (lava) sea surface shader.
 */

#version 100

precision mediump float;

const float TERRA_WATER_SCALE = 512.0;
const vec3  NORMAL            = vec3( 0.0, 0.0, 1.0 );

uniform mat4  oz_ProjCamera;
uniform mat4  oz_Model;
uniform vec3  oz_CameraPos;
uniform float oz_WaveBias;

attribute vec3 inPosition;
attribute vec2 inTexCoord;
attribute vec3 inNormal;
#ifdef OZ_BUMP_MAP
attribute vec3 inTangent;
attribute vec3 inBinormal;
#endif

varying vec3 exLook;
varying vec2 exTexCoord;
varying vec3 exNormal;
#ifdef OZ_BUMP_MAP
varying vec3 exTangent;
varying vec3 exBinormal;
#endif

float noise( vec2 pos, float t )
{
  return sin( pos.x*pos.x + pos.y*pos.y + t );
}

void main()
{
  float z        = 0.15 * sin( oz_WaveBias + inPosition.x + inPosition.y );
  vec4  position = oz_Model * vec4( inPosition.x, inPosition.y, z, 1.0 );
  float dx       = 0.2 * noise( 0.1 * position.xy, oz_WaveBias );
  float dy       = 0.2 * noise( 0.1 * position.yx, oz_WaveBias );

  exLook      = position.xyz - oz_CameraPos;
  exTexCoord  = inTexCoord * TERRA_WATER_SCALE;
  exNormal    = vec3( dx, dy, 1.0 );
#ifdef OZ_BUMP_MAP
  exTangent   = vec3( 1.0, 0.0, 0.0 );
  exBinormal  = vec3( 0.0, 1.0, 0.0 );
#endif
  gl_Position = oz_ProjCamera * position;
}
