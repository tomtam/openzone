/*
 *  default.frag
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

const float TAU = 6.283185307179586;

const float TERRA_DETAIL_SCALE = 512.0;
const float TERRA_WATER_SCALE  = 512.0;

/*
 * Transformation
 */
struct Transform
{
  mat4 proj;
  mat4 camera;
  mat4 model;

  mat4 cameraProj;
  mat4 complete;
};

uniform Transform       oz_Transform;

uniform vec3            oz_CameraPosition;

/*
 * Colour
 */
uniform vec4            oz_Colour = vec4( 1.0, 1.0, 1.0, 1.0 );

/*
 * Texturing
 */
uniform bool            oz_IsTextureEnabled = false;
uniform sampler2D       oz_Textures[4];

/*
 * Lighting
 */
struct CaelumLight
{
  vec3 dir;
  vec4 diffuse;
  vec4 ambient;
};

struct Light
{
  vec3 pos;
  vec4 diffuse;
};

uniform CaelumLight     oz_CaelumLight;
uniform Light           oz_PointLights[8];

uniform float           oz_Specular = 0.0;

/*
 * Fog
 */
struct Fog {
  float start;
  float end;
  vec4  colour;
};

uniform Fog             oz_Fog;

/*
 * Water
 */
uniform float           oz_WaveBias;

/*
 * Wind
 */
uniform vec4            oz_Wind;      // vec4( vec2( dirXY ), amplitude, phi )

/*
 * MD2 animation
 */
uniform vec3            oz_MD2Anim;   // vec3( firstFrame, secondFrame, interpolation )

/*
 * FUNCTIONS
 */

vec4 greyscale( vec4 colour )
{
  float avg = ( colour.x + colour.y + colour.z ) / 3.0;
  return vec4( avg, avg, avg, colour.w );
}

vec4 skyLightColour( vec3 normal )
{
  float diffuseFactor = max( dot( -oz_CaelumLight.dir, normal ), 0.0 );
  vec4 colour = diffuseFactor * oz_CaelumLight.diffuse + oz_CaelumLight.ambient;
  return min( colour, vec4( 1.0, 1.0, 1.0, 1.0 ) );
}

vec4 specularColour( vec3 normal, vec3 toCamera )
{
  vec3  reflectedLight = reflect( oz_CaelumLight.dir, normal );
  float factor = oz_Specular * max( dot( reflectedLight, toCamera ), 0.0 );
  return vec4( 1.0, 1.0, 1.0, 1.0 ) + vec4( factor * oz_CaelumLight.diffuse.xyz, 0.0 );
}

vec4 applyFog( vec4 colour, float dist )
{
  float ratio = ( dist - oz_Fog.start ) / ( oz_Fog.end - oz_Fog.start );
  return mix( colour, oz_Fog.colour, clamp( ratio, 0.0, 1.0 ) );
}

vec2 noise( vec2 seed )
{
  return vec2( cos( seed.x ), sin( seed.y ) );
}



































































// this file should have a 'round' number of lines to easier determine error line from compiler
// output as it is biased by the number of lines in header.glsl file
