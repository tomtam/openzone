/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 * plant.frag
 *
 * Mesh shader that deforms mesh according to the given wind.
 */

varying vec3 exPosition;
varying vec2 exTexCoord;
varying vec3 exNormal;

void main()
{
  vec3  toCamera = oz_CameraPosition - exPosition;
  vec3  normal   = normalize( exNormal );
  float dist     = length( toCamera );

  vec4 colourSample = texture2D( oz_Textures[0], exTexCoord );
  vec4 masksSample  = texture2D( oz_Textures[1], exTexCoord );

  if( oz_NightVision ) {
    vec4  diffuse    = skyLightColour( normal );
    vec4  fragColour = oz_Colour * colourSample + vec4( diffuse.xyz, 0.0 );
    float avgColour  = 0.33 * ( fragColour.r + fragColour.g + fragColour.b );

    gl_FragData[0] = applyFog( vec4( 0.0, avgColour, 0.0, fragColour.a ), dist );
  }
  else {
    vec4 diffuse    = skyLightColour( normal );
    vec4 fragColour = oz_Colour * colourSample * diffuse;

    gl_FragData[0] = applyFog( fragColour, dist );
  }
}