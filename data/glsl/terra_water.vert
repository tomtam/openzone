/*
 *  terra_water.vert
 *
 *  Terrain water (sea) shader.
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

attribute vec3 inPosition;
attribute vec2 inTexCoord;
attribute vec3 inNormal;

varying vec2  exTexCoord;
varying vec4  exColour;
varying float exDistance;

void main()
{
  float z = 0.15 * sin( oz_WaveBias + inPosition.x + inPosition.y );
  vec4 localPos  = vec4( inPosition.x, inPosition.y, z, 1.0 );
  vec3 position  = ( oz_Transform.camera * oz_Transform.model * localPos ).xyz;
  vec3 normal    = vec3( 0.0, 0.0, 1.0 );

  exTexCoord     = inTexCoord;
  exColour       = skyLightColour( normal );
  exDistance     = length( position.xyz );

  gl_Position    = oz_Transform.complete * localPos;
}
