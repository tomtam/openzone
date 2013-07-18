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

/*
 * celestial.frag
 *
 * Shader for celestial bodies (except stars).
 */

#include "header.glsl"

varying vec2  exTexCoord;
varying float exAzimuth;

void main()
{
  vec4 colour    = texture2D( oz_Textures[0], exTexCoord );

  gl_FragData[0] = oz_ColourTransform * colour;

#ifdef OZ_POSTPROCESS
  float glow     = ( colour.r + colour.g + colour.b ) / 2.0;

  gl_FragData[1] = vec4( glow, glow, glow, 1.0 );
#endif
}