/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 * Copyright (C) 2002-2011  Davorin Učakar
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
 *
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
 */

/**
 * @file matrix/VehicleClass.cc
 */

#include "stable.hh"

#include "matrix/VehicleClass.hh"

#include "matrix/Library.hh"
#include "matrix/Vehicle.hh"

#define OZ_CLASS_SET_STATE( stateBit, varName, defValue ) \
  if( config->get( varName, defValue ) ) { \
    state |= stateBit; \
  }

namespace oz
{
namespace matrix
{

ObjectClass* VehicleClass::createClass()
{
  return new VehicleClass();
}

void VehicleClass::initClass( const Config* config )
{
  flags = Object::DYNAMIC_BIT | Object::VEHICLE_BIT | Object::USE_FUNC_BIT |
      Object::UPDATE_FUNC_BIT | Object::CYLINDER_BIT;

  OZ_CLASS_SET_FLAG( Object::DESTROY_FUNC_BIT,   "flag.onDestroy",    true  );
  OZ_CLASS_SET_FLAG( Object::DAMAGE_FUNC_BIT,    "flag.onDamage",     false );
  OZ_CLASS_SET_FLAG( Object::HIT_FUNC_BIT,       "flag.onHit",        false );
  OZ_CLASS_SET_FLAG( Object::SOLID_BIT,          "flag.solid",        true  );
  OZ_CLASS_SET_FLAG( Object::NO_DRAW_BIT,        "flag.noDraw",       false );
  OZ_CLASS_SET_FLAG( Object::WIDE_CULL_BIT,      "flag.wideCull",     false );

  fillCommonConfig( config );

  if( audioType != -1 ) {
    const char* soundName;
    int         soundIndex;

    soundName  = config->get( "audioSound.splash", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Object::EVENT_SPLASH] = soundIndex;

    soundName  = config->get( "audioSound.fricting", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Object::EVENT_FRICTING] = soundIndex;

    soundName  = config->get( "audioSound.engine", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_ENGINE] = soundIndex;

    soundName  = config->get( "audioSound.nextWeapon", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_NEXT_WEAPON] = soundIndex;

    soundName  = config->get( "audioSound.shot0", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_SHOT0] = soundIndex;

    soundName  = config->get( "audioSound.shot1", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_SHOT1] = soundIndex;

    soundName  = config->get( "audioSound.shot2", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_SHOT2] = soundIndex;

    soundName  = config->get( "audioSound.shot3", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_SHOT3] = soundIndex;

    soundName  = config->get( "audioSound.shotEmpty", "" );
    soundIndex = String::isEmpty( soundName ) ? -1 : library.soundIndex( soundName );
    audioSounds[Vehicle::EVENT_SHOT_EMPTY] = soundIndex;
  }

  state = 0;

  OZ_CLASS_SET_STATE( Vehicle::CREW_VISIBLE_BIT, "state.crewVisible", false );
  OZ_CLASS_SET_STATE( Vehicle::HAS_EJECT_BIT,    "state.hasEject",    false );
  OZ_CLASS_SET_STATE( Vehicle::AUTO_EJECT_BIT,   "state.autoEject",   false );

  mass = config->get( "mass", 100.0f );
  lift = config->get( "lift", 13.0f );

  if( mass < 0.01f ) {
    throw Exception( "%s: Invalid object mass. Should be >= 0.01.", name.cstr() );
  }
  if( lift < 0.0f ) {
    throw Exception( "%s: Invalid object lift. Should be >= 0.", name.cstr() );
  }

  const char* sType = config->get( "type", "" );
  if( String::equals( sType, "STATIC" ) ) {
    type = Vehicle::WHEELED;
  }
  else if( String::equals( sType, "WHEELED" ) ) {
    type = Vehicle::WHEELED;
  }
  else if( String::equals( sType, "TRACKED" ) ) {
    type = Vehicle::TRACKED;
  }
  else if( String::equals( sType, "MECH" ) ) {
    type = Vehicle::TRACKED;
  }
  else if( String::equals( sType, "HOVER" ) ) {
    type = Vehicle::HOVER;
  }
  else if( String::equals( sType, "AIR" ) ) {
    type = Vehicle::AIR;
  }
  else {
    throw Exception( "%s: Invalid vehicle type, should be either STATIC, WHEELED, TRACKED, MECH, "
                     "HOVER or AIR", name.cstr() );
  }

  pilotPos = Vec3( config->get( "pilotPos.x", 0.0f ),
                   config->get( "pilotPos.y", 0.0f ),
                   config->get( "pilotPos.z", 0.0f ) );
  pilotRot = Quat::rotZXZ( config->get( "pilotRot.z", 0.0f ),
                           config->get( "pilotRot.x", 0.0f ),
                           0.0f );

  rotVelLimit            = Math::rad( config->get( "rotVelLimit", 60.0f ) ) * Timer::TICK_TIME;

  moveMomentum           = config->get( "moveMomentum", 2.0f );

  hoverHeight            = config->get( "hoverHeight", 2.0f );
  hoverHeightStiffness   = config->get( "hoverHeightStiffness", 40.0f );
  hoverMomentumStiffness = config->get( "hoverMomentumStiffness", 160.0f );

  enginePitchBias  = config->get( "enginePitchBias", 1.0f );
  enginePitchRatio = config->get( "enginePitchRatio", 0.001f );
  enginePitchLimit = config->get( "enginePitchLimit", 2.00f );

  fuel             = config->get( "fuel", 100.0f );
  fuelConsumption  = config->get( "fuelConsumption", 0.0f ) * Timer::TICK_TIME;

  char weaponNameBuffer[]   = "weapon  .name";
  char onShotBuffer[]       = "weapon  .onShot";
  char nRoundsBuffer[]      = "weapon  .nRounds";
  char shotIntervalBuffer[] = "weapon  .shotInterval";

  nWeapons = MAX_WEAPONS;
  for( int i = 0; i < MAX_WEAPONS; ++i ) {
    hard_assert( i < 100 );

    weaponNameBuffer[6] = char( '0' + ( i / 10 ) );
    weaponNameBuffer[7] = char( '0' + ( i % 10 ) );

    onShotBuffer[6] = char( '0' + ( i / 10 ) );
    onShotBuffer[7] = char( '0' + ( i % 10 ) );

    nRoundsBuffer[6] = char( '0' + ( i / 10 ) );
    nRoundsBuffer[7] = char( '0' + ( i % 10 ) );

    shotIntervalBuffer[6] = char( '0' + ( i / 10 ) );
    shotIntervalBuffer[7] = char( '0' + ( i % 10 ) );

    weaponNames[i]  = gettext( config->get( weaponNameBuffer, "" ) );
    onShot[i]       = config->get( onShotBuffer, "" );
    nRounds[i]      = config->get( nRoundsBuffer, -1 );
    shotInterval[i] = config->get( shotIntervalBuffer, 0.5f );

    if( weaponNames[i].isEmpty() && nWeapons > i ) {
      nWeapons = i;
    }
    if( !String::isEmpty( onShot[i] ) ) {
      flags |= Object::LUA_BIT;
    }
  }
}

Object* VehicleClass::create( int index, const Point3& pos, Heading heading ) const
{
  return new Vehicle( this, index, pos, heading );
}

Object* VehicleClass::create( InputStream* istream ) const
{
  return new Vehicle( this, istream );
}

}
}