/*
 * OpenZone - Simple Cross-Platform FPS/RTS Game Engine
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
 * Davorin Učakar <davorin.ucakar@gmail.com>
 */

/**
 * @file matrix/Lua.cpp
 *
 * Lua scripting engine for Matrix
 */

#include "stable.hpp"

#include "matrix/Lua.hpp"

#include "matrix/BotClass.hpp"
#include "matrix/VehicleClass.hpp"
#include "matrix/Library.hpp"
#include "matrix/Collider.hpp"
#include "matrix/Synapse.hpp"

#include "luamacros.hpp"

namespace oz
{
namespace matrix
{

Lua lua;

bool Lua::readVariable( InputStream* istream )
{
  char ch = istream->readChar();

  switch( ch ) {
    case 'N': {
      pushnil();
      return true;
    }
    case 'b': {
      pushbool( istream->readBool() );
      return true;
    }
    case 'n': {
      pushdouble( istream->readDouble() );
      return true;
    }
    case 's': {
      pushstring( istream->readString() );
      return true;
    }
    case '[': {
      newtable();

      while( readVariable( istream ) ) { // key
        readVariable( istream ); // value

        rawset( -3 );
      }
      return true;
    }
    case ']': {
      return false;
    }
    default: {
      throw Exception( "Invalid type char '" + String( ch ) + "' in serialised Lua data" );
    }
  }
}

void Lua::writeVariable( OutputStream* ostream )
{
  int type = type( -1 );

  switch( type ) {
    case LUA_TNIL: {
      ostream->writeChar( 'N' );
      break;
    }
    case LUA_TBOOLEAN: {
      ostream->writeChar( 'b' );
      ostream->writeBool( tobool( -1 ) != 0 );
      break;
    }
    case LUA_TNUMBER: {
      ostream->writeChar( 'n' );
      ostream->writeDouble( todouble( -1 ) );
      break;
    }
    case LUA_TSTRING: {
      ostream->writeChar( 's' );
      ostream->writeString( tostring( -1 ) );
      break;
    }
    case LUA_TTABLE: {
      ostream->writeChar( '[' );

      pushnil();
      while( next( -2 ) != 0 ) {
        // key
        pushvalue( -2 );
        writeVariable( ostream );
        pop( 1 );

        // value
        writeVariable( ostream );

        pop( 1 );
      }

      ostream->writeChar( ']' );
      break;
    }
    default: {
      throw Exception( "Serialisation is only supported for LUA_TNIL, LUA_TBOOLEAN, LUA_TNUMBER, "
                       "LUA_TSTRING and LUA_TTABLE data types" );
    }
  }
}

Lua::Lua() : l( null )
{}

void Lua::objectCall( const char* functionName, Object* self_, Bot* user_ )
{
  self         = self_;
  user         = user_;
  obj          = self_;
  str          = null;
  part         = null;
  objIndex     = 0;
  strIndex     = 0;
  event        = List<Object::Event>::CIterator();
  isFirstEvent = false;
  hasUseFailed = false;

  hard_assert( gettop() == 1 && self != null );

  getglobal( functionName );
  rawgeti( 1, self->index );
  lua_pcall( l, 1, 0, 0 );

  if( gettop() != 1 ) {
    log.println( "M! [%s] %s", functionName, tostring( -1 ) );
    settop( 1 );

    if( !config.get( "lua.tolerant", false ) ) {
      throw Exception( "Matrix Lua function call finished with an error" );
    }
  }
}

void Lua::registerObject( int index )
{
  // we cannot depend that ozLocalData exists at index 1 as this function can be called via a
  // script creating an object
  getglobal( "ozLocalData" );
  newtable();
  rawseti( -2, index );
  pop( 1 );
}

void Lua::unregisterObject( int index )
{
  // we cannot depend that ozLocalData exists at index 1 as this function can be called via a
  // script creating an object
  getglobal( "ozLocalData" );
  pushnil();
  rawseti( -2, index );
  pop( 1 );
}

void Lua::read( InputStream* istream )
{
  hard_assert( gettop() == 1 );
  hard_assert( ( pushnil(), next( 1 ) == 0 ) );

  char ch = istream->readChar();
  hard_assert( ch ==  '[' );

  ch = istream->readChar();

  while( ch != ']' ) {
    hard_assert( ch == 'i' );

    int index = istream->readInt();
    readVariable( istream );

    rawseti( 1, index );

    ch = istream->readChar();
  }
}

void Lua::write( OutputStream* ostream )
{
  hard_assert( gettop() == 1 );

  ostream->writeChar( '[' );

  pushnil();
  while( next( 1 ) != 0 ) {
    hard_assert( type( -2 ) == LUA_TNUMBER );
    hard_assert( type( -1 ) == LUA_TTABLE );

    ostream->writeChar( 'i' );
    ostream->writeInt( toint( -2 ) );
    writeVariable( ostream );

    pop( 1 );
  }

  ostream->writeChar( ']' );
}

void Lua::registerFunction( const char* name, LuaAPI func )
{
  lua_register( l, name, func );
}

void Lua::registerConstant( const char* name, bool value )
{
  pushbool( value );
  setglobal( name );
}

void Lua::registerConstant( const char* name, int value )
{
  pushint( value );
  setglobal( name );
}

void Lua::registerConstant( const char* name, float value )
{
  pushfloat( value );
  setglobal( name );
}

void Lua::registerConstant( const char* name, const char* value )
{
  pushstring( value );
  setglobal( name );
}

void Lua::init()
{
  log.println( "Initialising Matrix Lua {" );
  log.indent();

  config.getSet( "lua.tolerant", false );

  l = luaL_newstate();
  if( l == null ) {
    log.println( "Failed to create state" );
    throw Exception( "Lua initialisation failed" );
  }

  luaL_openlibs( l );

  /*
   * General functions
   */

  OZ_LUA_FUNC( ozPrintln );
  OZ_LUA_FUNC( ozException );
  OZ_LUA_FUNC( ozGettext );

  OZ_LUA_FUNC( ozUseFailed );

  /*
   * Orbis
   */

  OZ_LUA_FUNC( ozOrbisAddStr );
  OZ_LUA_FUNC( ozOrbisTryAddStr );
  OZ_LUA_FUNC( ozOrbisAddObj );
  OZ_LUA_FUNC( ozOrbisTryAddObj );
  OZ_LUA_FUNC( ozOrbisAddPart );
  OZ_LUA_FUNC( ozOrbisGenParts );

  OZ_LUA_FUNC( ozBindAllOverlaps );
  OZ_LUA_FUNC( ozBindStrOverlaps );
  OZ_LUA_FUNC( ozBindObjOverlaps );

  /*
   * Terra
   */

  OZ_LUA_FUNC( ozTerraLoad );

  OZ_LUA_FUNC( ozTerraHeight );

  /*
   * Caelum
   */

  OZ_LUA_FUNC( ozCaelumLoad );

  OZ_LUA_FUNC( ozCaelumGetHeading );
  OZ_LUA_FUNC( ozCaelumSetHeading );
  OZ_LUA_FUNC( ozCaelumGetPeriod );
  OZ_LUA_FUNC( ozCaelumSetPeriod );
  OZ_LUA_FUNC( ozCaelumGetTime );
  OZ_LUA_FUNC( ozCaelumSetTime );
  OZ_LUA_FUNC( ozCaelumAddTime );

  /*
   * Structure
   */

  OZ_LUA_FUNC( ozStrBindIndex );
  OZ_LUA_FUNC( ozStrBindNext );

  OZ_LUA_FUNC( ozStrIsNull );

  OZ_LUA_FUNC( ozStrGetIndex );
  OZ_LUA_FUNC( ozStrGetBounds );
  OZ_LUA_FUNC( ozStrGetPos );
  OZ_LUA_FUNC( ozStrGetBSP );
  OZ_LUA_FUNC( ozStrGetHeading );
  OZ_LUA_FUNC( ozStrGetLife );
  OZ_LUA_FUNC( ozStrSetLife );
  OZ_LUA_FUNC( ozStrAddLife );

  OZ_LUA_FUNC( ozStrDamage );
  OZ_LUA_FUNC( ozStrDestroy );
  OZ_LUA_FUNC( ozStrRemove );

  OZ_LUA_FUNC( ozStrVectorFromSelf );
  OZ_LUA_FUNC( ozStrVectorFromSelfEye );
  OZ_LUA_FUNC( ozStrDirectionFromSelf );
  OZ_LUA_FUNC( ozStrDirectionFromSelfEye );
  OZ_LUA_FUNC( ozStrDistanceFromSelf );
  OZ_LUA_FUNC( ozStrDistanceFromSelfEye );
  OZ_LUA_FUNC( ozStrHeadingFromSelf );
  OZ_LUA_FUNC( ozStrRelativeHeadingFromSelf );
  OZ_LUA_FUNC( ozStrPitchFromSelf );
  OZ_LUA_FUNC( ozStrPitchFromSelfEye );

  OZ_LUA_FUNC( ozStrBindAllOverlaps );
  OZ_LUA_FUNC( ozStrBindStrOverlaps );
  OZ_LUA_FUNC( ozStrBindObjOverlaps );

  /*
   * Object
   */

  OZ_LUA_FUNC( ozEventBindNext );

  OZ_LUA_FUNC( ozEventGet );

  OZ_LUA_FUNC( ozObjBindIndex );
  OZ_LUA_FUNC( ozObjBindPilot );
  OZ_LUA_FUNC( ozObjBindSelf );
  OZ_LUA_FUNC( ozObjBindUser );
  OZ_LUA_FUNC( ozObjBindNext );

  OZ_LUA_FUNC( ozObjIsNull );
  OZ_LUA_FUNC( ozObjIsSelf );
  OZ_LUA_FUNC( ozObjIsUser );
  OZ_LUA_FUNC( ozObjIsCut );
  OZ_LUA_FUNC( ozObjIsBrowsable );
  OZ_LUA_FUNC( ozObjIsDynamic );
  OZ_LUA_FUNC( ozObjIsItem );
  OZ_LUA_FUNC( ozObjIsWeapon );
  OZ_LUA_FUNC( ozObjIsBot );
  OZ_LUA_FUNC( ozObjIsVehicle );

  OZ_LUA_FUNC( ozObjGetIndex );
  OZ_LUA_FUNC( ozObjGetPos );
  OZ_LUA_FUNC( ozObjSetPos );
  OZ_LUA_FUNC( ozObjAddPos );
  OZ_LUA_FUNC( ozObjGetDim );
  OZ_LUA_FUNC( ozObjGetFlags );
  OZ_LUA_FUNC( ozObjGetHeading );
  OZ_LUA_FUNC( ozObjGetClassName );
  OZ_LUA_FUNC( ozObjGetLife );
  OZ_LUA_FUNC( ozObjSetLife );
  OZ_LUA_FUNC( ozObjAddLife );

  OZ_LUA_FUNC( ozObjAddEvent );

  OZ_LUA_FUNC( ozObjAddItem );
  OZ_LUA_FUNC( ozObjRemoveItem );
  OZ_LUA_FUNC( ozObjRemoveAllItems );

  OZ_LUA_FUNC( ozObjEnableUpdate );
  OZ_LUA_FUNC( ozObjDamage );
  OZ_LUA_FUNC( ozObjDestroy );
  OZ_LUA_FUNC( ozObjQuietDestroy );
  OZ_LUA_FUNC( ozObjRemove );

  OZ_LUA_FUNC( ozObjVectorFromSelf );
  OZ_LUA_FUNC( ozObjVectorFromSelfEye );
  OZ_LUA_FUNC( ozObjDirectionFromSelf );
  OZ_LUA_FUNC( ozObjDirectionFromSelfEye );
  OZ_LUA_FUNC( ozObjDistanceFromSelf );
  OZ_LUA_FUNC( ozObjDistanceFromSelfEye );
  OZ_LUA_FUNC( ozObjHeadingFromSelf );
  OZ_LUA_FUNC( ozObjRelativeHeadingFromSelf );
  OZ_LUA_FUNC( ozObjPitchFromSelf );
  OZ_LUA_FUNC( ozObjPitchFromSelfEye );
  OZ_LUA_FUNC( ozObjIsVisibleFromSelf );
  OZ_LUA_FUNC( ozObjIsVisibleFromSelfEye );

  OZ_LUA_FUNC( ozObjBindEvents );
  OZ_LUA_FUNC( ozObjBindItems );

  OZ_LUA_FUNC( ozObjBindAllOverlaps );
  OZ_LUA_FUNC( ozObjBindStrOverlaps );
  OZ_LUA_FUNC( ozObjBindObjOverlaps );

  /*
   * Dynamic object
   */

  OZ_LUA_FUNC( ozDynBindParent );

  OZ_LUA_FUNC( ozDynGetVelocity );
  OZ_LUA_FUNC( ozDynSetVelocity );
  OZ_LUA_FUNC( ozDynAddVelocity );
  OZ_LUA_FUNC( ozDynGetMomentum );
  OZ_LUA_FUNC( ozDynSetMomentum );
  OZ_LUA_FUNC( ozDynAddMomentum );
  OZ_LUA_FUNC( ozDynGetMass );
  OZ_LUA_FUNC( ozDynGetLift );

  /*
   * Weapon
   */

  OZ_LUA_FUNC( ozWeaponGetDefaultRounds );
  OZ_LUA_FUNC( ozWeaponGetRounds );
  OZ_LUA_FUNC( ozWeaponSetRounds );
  OZ_LUA_FUNC( ozWeaponAddRounds );
  OZ_LUA_FUNC( ozWeaponReload );

  /*
   * Bot
   */

  OZ_LUA_FUNC( ozBotBindPilot );

  OZ_LUA_FUNC( ozBotGetName );
  OZ_LUA_FUNC( ozBotSetName );
  OZ_LUA_FUNC( ozBotGetMindFunc );
  OZ_LUA_FUNC( ozBotSetMindFunc );

  OZ_LUA_FUNC( ozBotGetState );
  OZ_LUA_FUNC( ozBotGetEyePos );
  OZ_LUA_FUNC( ozBotGetH );
  OZ_LUA_FUNC( ozBotSetH );
  OZ_LUA_FUNC( ozBotAddH );
  OZ_LUA_FUNC( ozBotGetV );
  OZ_LUA_FUNC( ozBotSetV );
  OZ_LUA_FUNC( ozBotAddV );
  OZ_LUA_FUNC( ozBotGetDir );

  OZ_LUA_FUNC( ozBotGetStamina );
  OZ_LUA_FUNC( ozBotSetStamina );
  OZ_LUA_FUNC( ozBotAddStamina );

  OZ_LUA_FUNC( ozBotActionForward );
  OZ_LUA_FUNC( ozBotActionBackward );
  OZ_LUA_FUNC( ozBotActionRight );
  OZ_LUA_FUNC( ozBotActionLeft );
  OZ_LUA_FUNC( ozBotActionJump );
  OZ_LUA_FUNC( ozBotActionCrouch );
  OZ_LUA_FUNC( ozBotActionUse );
  OZ_LUA_FUNC( ozBotActionTake );
  OZ_LUA_FUNC( ozBotActionGrab );
  OZ_LUA_FUNC( ozBotActionThrow );
  OZ_LUA_FUNC( ozBotActionAttack );
  OZ_LUA_FUNC( ozBotActionExit );
  OZ_LUA_FUNC( ozBotActionEject );
  OZ_LUA_FUNC( ozBotActionSuicide );

  OZ_LUA_FUNC( ozBotIsRunning );
  OZ_LUA_FUNC( ozBotSetRunning );
  OZ_LUA_FUNC( ozBotToggleRunning );

  OZ_LUA_FUNC( ozBotSetGesture );

  OZ_LUA_FUNC( ozBotSetWeaponItem );

  OZ_LUA_FUNC( ozBotHeal );
  OZ_LUA_FUNC( ozBotRearm );
  OZ_LUA_FUNC( ozBotKill );

  OZ_LUA_FUNC( ozBotIsVisibleFromSelfEyeToEye );

  /*
   * Vehicle
   */

  OZ_LUA_FUNC( ozVehicleGetH );
  OZ_LUA_FUNC( ozVehicleSetH );
  OZ_LUA_FUNC( ozVehicleAddH );
  OZ_LUA_FUNC( ozVehicleGetV );
  OZ_LUA_FUNC( ozVehicleSetV );
  OZ_LUA_FUNC( ozVehicleAddV );
  OZ_LUA_FUNC( ozVehicleGetDir );

  OZ_LUA_FUNC( ozVehicleEmbarkPilot );

  OZ_LUA_FUNC( ozVehicleService );

  /*
   * Particle
   */

  OZ_LUA_FUNC( ozPartBindIndex );

  OZ_LUA_FUNC( ozPartIsNull );

  OZ_LUA_FUNC( ozPartGetPos );
  OZ_LUA_FUNC( ozPartSetPos );
  OZ_LUA_FUNC( ozPartAddPos );
  OZ_LUA_FUNC( ozPartGetIndex );
  OZ_LUA_FUNC( ozPartGetVelocity );
  OZ_LUA_FUNC( ozPartSetVelocity );
  OZ_LUA_FUNC( ozPartAddVelocity );
  OZ_LUA_FUNC( ozPartGetLife );
  OZ_LUA_FUNC( ozPartSetLife );
  OZ_LUA_FUNC( ozPartAddLife );

  OZ_LUA_FUNC( ozPartRemove );

  /*
   * Constants
   */

  OZ_LUA_CONST( "OZ_ORBIS_DIM",                   Orbis::DIM );

  OZ_LUA_CONST( "OZ_NORTH",                       NORTH );
  OZ_LUA_CONST( "OZ_WEST",                        WEST );
  OZ_LUA_CONST( "OZ_SOUTH",                       SOUTH );
  OZ_LUA_CONST( "OZ_EAST",                        EAST );

  OZ_LUA_CONST( "OZ_EVENT_CREATE",                Object::EVENT_CREATE );
  OZ_LUA_CONST( "OZ_EVENT_DESTROY",               Object::EVENT_DESTROY );
  OZ_LUA_CONST( "OZ_EVENT_DAMAGE",                Object::EVENT_DAMAGE );
  OZ_LUA_CONST( "OZ_EVENT_HIT",                   Object::EVENT_HIT );
  OZ_LUA_CONST( "OZ_EVENT_SPLASH",                Object::EVENT_SPLASH );
  OZ_LUA_CONST( "OZ_EVENT_FRICTING",              Object::EVENT_FRICTING );
  OZ_LUA_CONST( "OZ_EVENT_USE",                   Object::EVENT_USE );
  OZ_LUA_CONST( "OZ_EVENT_SHOT",                  Weapon::EVENT_SHOT );
  OZ_LUA_CONST( "OZ_EVENT_SHOT_EMPTY",            Weapon::EVENT_SHOT_EMPTY );
  OZ_LUA_CONST( "OZ_EVENT_HIT_HARD",              Bot::EVENT_HIT_HARD );
  OZ_LUA_CONST( "OZ_EVENT_LAND",                  Bot::EVENT_LAND );
  OZ_LUA_CONST( "OZ_EVENT_JUMP",                  Bot::EVENT_JUMP );
  OZ_LUA_CONST( "OZ_EVENT_FLIP",                  Bot::EVENT_FLIP );
  OZ_LUA_CONST( "OZ_EVENT_DEATH",                 Bot::EVENT_DEATH );
  OZ_LUA_CONST( "OZ_EVENT_ENGINE",                Vehicle::EVENT_ENGINE );
  OZ_LUA_CONST( "OZ_EVENT_NEXT_WEAPON",           Vehicle::EVENT_NEXT_WEAPON );
  OZ_LUA_CONST( "OZ_EVENT_SHOT0",                 Vehicle::EVENT_SHOT0 );
  OZ_LUA_CONST( "OZ_EVENT_SHOT0_EMPTY",           Vehicle::EVENT_SHOT0_EMPTY );
  OZ_LUA_CONST( "OZ_EVENT_SHOT1",                 Vehicle::EVENT_SHOT1 );
  OZ_LUA_CONST( "OZ_EVENT_SHOT1_EMPTY",           Vehicle::EVENT_SHOT1_EMPTY );
  OZ_LUA_CONST( "OZ_EVENT_SHOT2",                 Vehicle::EVENT_SHOT2 );
  OZ_LUA_CONST( "OZ_EVENT_SHOT2_EMPTY",           Vehicle::EVENT_SHOT2_EMPTY );

  OZ_LUA_CONST( "OZ_OBJECT_DYNAMIC_BIT",          Object::DYNAMIC_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_WEAPON_BIT",           Object::WEAPON_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_BOT_BIT",              Object::BOT_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_VEHICLE_BIT",          Object::VEHICLE_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_ITEM_BIT",             Object::ITEM_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_BROWSABLE_BIT",        Object::BROWSABLE_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_NO_DRAW_BIT",          Object::NO_DRAW_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_WIDE_CULL_BIT",        Object::WIDE_CULL_BIT );

  OZ_LUA_CONST( "OZ_OBJECT_LUA_BIT",              Object::LUA_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_DESTROY_FUNC_BIT",     Object::DESTROY_FUNC_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_DAMAGE_FUNC_BIT",      Object::DAMAGE_FUNC_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_HIT_FUNC_BIT",         Object::HIT_FUNC_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_UPDATE_FUNC_BIT",      Object::UPDATE_FUNC_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_USE_FUNC_BIT",         Object::USE_FUNC_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_UPDATE_FUNC_BIT",      Object::UPDATE_FUNC_BIT );

  OZ_LUA_CONST( "OZ_OBJECT_DEVICE_BIT",           Object::DEVICE_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_IMAGO_BIT",            Object::IMAGO_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_AUDIO_BIT",            Object::AUDIO_BIT );

  OZ_LUA_CONST( "OZ_OBJECT_DESTROYED_BIT",        Object::DESTROYED_BIT );

  OZ_LUA_CONST( "OZ_OBJECT_DISABLED_BIT",         Object::DISABLED_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_HIT_BIT",              Object::HIT_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_FRICTING_BIT",         Object::FRICTING_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_BELOW_BIT",            Object::BELOW_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_ON_FLOOR_BIT",         Object::ON_FLOOR_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_ON_SLICK_BIT",         Object::ON_SLICK_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_IN_WATER_BIT",         Object::IN_WATER_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_ON_LADDER_BIT",        Object::ON_LADDER_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_SOLID_BIT",            Object::SOLID_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_CYLINDER_BIT",         Object::CYLINDER_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_CLIMBER_BIT",          Object::CLIMBER_BIT );
  OZ_LUA_CONST( "OZ_OBJECT_PUSHER_BIT",           Object::PUSHER_BIT );

  OZ_LUA_CONST( "OZ_BOT_DEAD_BIT",                Bot::DEAD_BIT );
  OZ_LUA_CONST( "OZ_BOT_MECHANICAL_BIT",          Bot::MECHANICAL_BIT );
  OZ_LUA_CONST( "OZ_BOT_INCARNATABLE_BIT",        Bot::INCARNATABLE_BIT );
  OZ_LUA_CONST( "OZ_BOT_PLAYER_BIT",              Bot::PLAYER_BIT );

  OZ_LUA_CONST( "OZ_BOT_STEPABLE_BIT",            Bot::STEPABLE_BIT );
  OZ_LUA_CONST( "OZ_BOT_CROUCHING_BIT",           Bot::CROUCHING_BIT );
  OZ_LUA_CONST( "OZ_BOT_RUNNING_BIT",             Bot::RUNNING_BIT );
  OZ_LUA_CONST( "OZ_BOT_SHOOTING_BIT",            Bot::SHOOTING_BIT );
  OZ_LUA_CONST( "OZ_BOT_MOVING_BIT",              Bot::MOVING_BIT );
  OZ_LUA_CONST( "OZ_BOT_GRAB_BIT",                Bot::GRAB_BIT );
  OZ_LUA_CONST( "OZ_BOT_CROUCHING_BIT",           Bot::CROUCHING_BIT );

  OZ_LUA_CONST( "OZ_BOT_GESTURE0_BIT",            Bot::GESTURE0_BIT );
  OZ_LUA_CONST( "OZ_BOT_GESTURE1_BIT",            Bot::GESTURE1_BIT );
  OZ_LUA_CONST( "OZ_BOT_GESTURE2_BIT",            Bot::GESTURE2_BIT );
  OZ_LUA_CONST( "OZ_BOT_GESTURE3_BIT",            Bot::GESTURE3_BIT );
  OZ_LUA_CONST( "OZ_BOT_GESTURE4_BIT",            Bot::GESTURE4_BIT );

  newtable();
  setglobal( "ozLocalData" );
  getglobal( "ozLocalData" );

  DArray<File> luaFiles;
  File luaDir( "lua/matrix" );
  luaDir.ls( &luaFiles );

  foreach( file, luaFiles.citer() ) {
    if( file->hasExtension( "lua" ) ) {
      log.print( "%s ...", file->name() );

      if( luaL_dofile( l, file->path() ) != 0 ) {
        log.printEnd( " Failed" );
        throw Exception( "Matrix Lua script error" );
      }

      log.printEnd( " OK" );
    }
  }

  hard_assert( gettop() == 1 );

  log.unindent();
  log.println( "}" );
}

void Lua::free()
{
  if( l == null ) {
    return;
  }

  log.print( "Freeing Matrix Lua ..." );

  objects.clear();
  objects.dealloc();

  structs.clear();
  structs.dealloc();

  lua_close( l );
  l = null;

  log.printEnd( " OK" );
}

/*
 * General functions
 */

int Lua::ozPrintln( lua_State* l )
{
  ARG( 1 );

  log.println( "M> %s", tostring( 1 ) );
  return 0;
}

int Lua::ozException( lua_State* l )
{
  ARG( 1 );

  const char* message = tostring( 1 );
  throw Exception( message );
}

int Lua::ozGettext( lua_State* l )
{
  ARG( 1 );

  pushstring( gettext( tostring( 1 ) ) );
  return 1;
}

int Lua::ozUseFailed( lua_State* l )
{
  ARG( 0 );

  lua.hasUseFailed = true;
  return 1;
}

/*
 * Orbis
 */

int Lua::ozOrbisAddStr( lua_State* l )
{
  ARG( 5 );

  const char* name = tostring( 1 );
  Point3 p = Point3( tofloat( 2 ), tofloat( 3 ), tofloat( 4 ) );
  Heading heading = Heading( toint( 5 ) );

  int index = synapse.addStruct( name, p, heading );
  lua.str = orbis.structs[index];
  pushint( index );
  return 1;
}

int Lua::ozOrbisTryAddStr( lua_State* l )
{
  ARG( 5 );

  const char* name = tostring( 1 );
  Point3 p = Point3( tofloat( 2 ), tofloat( 3 ), tofloat( 4 ) );
  Heading heading = Heading( toint( 5 ) );

  int id = library.bspIndex( name );
  Bounds bounds = Struct::rotate( library.bspClasses[id].bounds, heading ) + ( p - Point3::ORIGIN );

  if( collider.overlaps( bounds.toAABB() ) ) {
    lua.str = null;
    pushint( -1 );
  }
  else {
    int index = synapse.addStruct( name, p, heading );
    lua.str = orbis.structs[index];
    pushint( index );
  }
  return 1;
}

int Lua::ozOrbisAddObj( lua_State* l )
{
  ARG_GE( 4 );

  const char* name = tostring( 1 );
  Point3 p = Point3( tofloat( 2 ), tofloat( 3 ), tofloat( 4 ) );
  Heading heading = Heading( toint( 5 ) );

  int index = synapse.addObject( name, p, heading );
  lua.obj = orbis.objects[index];
  pushint( index );
  return 1;
}

int Lua::ozOrbisTryAddObj( lua_State* l )
{
  ARG_GE( 4 );

  const char* name = tostring( 1 );
  Point3 p = Point3( tofloat( 2 ), tofloat( 3 ), tofloat( 4 ) );
  Heading heading = Heading( toint( 5 ) );

  const ObjectClass* const* value = library.classes.find( name );
  if( value == null ) {
    ERROR( "invalid object class" );
  }

  AABB aabb = AABB( p, ( *value )->dim );

  if( heading == WEST || heading == EAST ) {
    swap( aabb.dim.x, aabb.dim.y );
  }

  if( collider.overlaps( aabb ) ) {
    lua.obj = null;
    pushint( -1 );
  }
  else {
    int index = synapse.addObject( name, p, heading );
    lua.obj = orbis.objects[index];
    pushint( index );
  }
  return 1;
}

int Lua::ozOrbisAddPart( lua_State* l )
{
  ARG( 12 );

  Point3 p           = Point3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) );
  Vec3   velocity    = Vec3( tofloat( 4 ), tofloat( 5 ), tofloat( 6 ) );
  Vec3   colour      = Vec3( tofloat( 7 ), tofloat( 8 ), tofloat( 9 ) );
  float  restitution = tofloat( 10 );
  float  mass        = tofloat( 11 );
  float  lifeTime    = tofloat( 12 );

  int index = synapse.addPart( p, velocity, colour, restitution, mass, lifeTime );
  lua.part = orbis.parts[index];
  pushint( index );
  return 1;
}

int Lua::ozOrbisGenParts( lua_State* l )
{
  ARG( 15 );

  int    number         = int( tofloat( 1 ) );
  Point3 p              = Point3( tofloat( 2 ), tofloat( 3 ), tofloat( 4 ) );
  Vec3   velocity       = Vec3( tofloat( 5 ), tofloat( 6 ), tofloat( 7 ) );
  float  velocitySpread = tofloat( 8 );
  Vec3   colour         = Vec3( tofloat( 9 ), tofloat( 10 ), tofloat( 11 ) );
  float  colourSpread   = tofloat( 12 );
  float  restitution    = tofloat( 13 );
  float  mass           = tofloat( 14 );
  float  lifeTime       = tofloat( 15 );

  synapse.genParts( number, p, velocity, velocitySpread, colour, colourSpread,
                    restitution, mass, lifeTime );
  lua.part = null;
  return 0;
}

int Lua::ozBindAllOverlaps( lua_State* l )
{
  ARG( 6 );

  AABB aabb = AABB( Point3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ),
                    Vec3( tofloat( 4 ), tofloat( 5 ), tofloat( 6 ) ) );

  lua.objects.clear();
  lua.structs.clear();
  collider.getOverlaps( aabb, &lua.objects, &lua.structs );
  lua.objIndex = 0;
  lua.strIndex = 0;
  return 0;
}

int Lua::ozBindStrOverlaps( lua_State* l )
{
  ARG( 6 );

  AABB aabb = AABB( Point3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ),
                    Vec3( tofloat( 4 ), tofloat( 5 ), tofloat( 6 ) ) );

  lua.structs.clear();
  collider.getOverlaps( aabb, null, &lua.structs );
  lua.strIndex = 0;
  return 0;
}

int Lua::ozBindObjOverlaps( lua_State* l )
{
  ARG( 6 );

  AABB aabb = AABB( Point3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ),
                    Vec3( tofloat( 4 ), tofloat( 5 ), tofloat( 6 ) ) );

  lua.objects.clear();
  collider.getOverlaps( aabb, &lua.objects, null );
  lua.objIndex = 0;
  return 0;
}

/*
 * Terra
 */

int Lua::ozTerraLoad( lua_State* l )
{
  ARG( 1 );

  String name = tostring( 1 );
  int id = library.terraIndex( name );

  orbis.terra.load( id );
  return 0;
}

int Lua::ozTerraHeight( lua_State* l )
{
  ARG( 2 );

  float x = tofloat( 1 );
  float y = tofloat( 2 );

  pushfloat( orbis.terra.height( x, y ) );
  return 1;
}

/*
 * Caelum
 */

int Lua::ozCaelumLoad( lua_State* l )
{
  ARG( 1 );

  String name = tostring( 1 );
  int id = library.caelumIndex( name );

  orbis.caelum.id = id;
  return 0;
}

int Lua::ozCaelumGetHeading( lua_State* l )
{
  ARG( 0 );

  pushfloat( orbis.caelum.heading );
  return 1;
}

int Lua::ozCaelumSetHeading( lua_State* l )
{
  ARG( 1 );

  orbis.caelum.heading = tofloat( 1 );
  return 0;
}

int Lua::ozCaelumGetPeriod( lua_State* l )
{
  ARG( 0 );

  pushfloat( orbis.caelum.period );
  return 1;
}

int Lua::ozCaelumSetPeriod( lua_State* l )
{
  ARG( 1 );

  orbis.caelum.period = tofloat( 1 );
  return 0;
}

int Lua::ozCaelumGetTime( lua_State* l )
{
  ARG( 0 );

  pushfloat( orbis.caelum.time );
  return 1;
}

int Lua::ozCaelumSetTime( lua_State* l )
{
  ARG( 1 );

  orbis.caelum.time = tofloat( 1 );
  return 0;
}

int Lua::ozCaelumAddTime( lua_State* l )
{
  ARG( 1 );

  orbis.caelum.time += tofloat( 1 );
  return 0;
}

/*
 * Structure
 */

int Lua::ozStrBindIndex( lua_State* l )
{
  ARG( 1 );

  int index = toint( 1 );
  if( uint( index ) >= uint( orbis.structs.length() ) ) {
    ERROR( "invalid structure index" );
  }
  lua.str = orbis.structs[index];
  return 0;
}

int Lua::ozStrBindNext( lua_State* l )
{
  ARG( 0 );

  if( lua.strIndex < lua.structs.length() ) {
    lua.str = lua.structs[lua.strIndex];
    ++lua.strIndex;
    pushbool( true );
  }
  else {
    pushbool( false );
  }
  return 1;
}

int Lua::ozStrIsNull( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.str == null );
  return 1;
}

int Lua::ozStrGetIndex( lua_State* l )
{
  ARG( 0 );

  if( lua.str == null ) {
    pushint( -1 );
  }
  else {
    pushint( lua.str->index );
  }
  return 1;
}

int Lua::ozStrGetBounds( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  pushfloat( lua.str->mins.x );
  pushfloat( lua.str->mins.y );
  pushfloat( lua.str->mins.z );
  pushfloat( lua.str->maxs.x );
  pushfloat( lua.str->maxs.y );
  pushfloat( lua.str->maxs.z );
  return 6;
}

int Lua::ozStrGetPos( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  pushfloat( lua.str->p.x );
  pushfloat( lua.str->p.y );
  pushfloat( lua.str->p.z );
  return 3;
}

int Lua::ozStrGetBSP( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  pushint( lua.str->id );
  return 1;
}

int Lua::ozStrGetHeading( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  pushint( lua.str->heading );
  return 1;
}

int Lua::ozStrGetLife( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  pushfloat( lua.str->life );
  return 1;
}

int Lua::ozStrSetLife( lua_State* l )
{
  ARG( 1 );
  STR_NOT_NULL();

  lua.str->life = tofloat( 1 );
  return 0;
}

int Lua::ozStrAddLife( lua_State* l )
{
  ARG( 1 );
  STR_NOT_NULL();

  lua.str->life += tofloat( 1 );
  return 0;
}

int Lua::ozStrDamage( lua_State* l )
{
  ARG( 1 );
  STR_NOT_NULL();

  lua.str->damage( tofloat( 1 ) );
  return 0;
}

int Lua::ozStrDestroy( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  lua.str->destroy();
  return 0;
}

int Lua::ozStrRemove( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  synapse.remove( lua.str );
  lua.str = null;
  return 0;
}

int Lua::ozStrVectorFromSelf( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  Vec3 vec = lua.str->p - lua.self->p;
  pushfloat( vec.x );
  pushfloat( vec.y );
  pushfloat( vec.z );
  return 3;
}

int Lua::ozStrVectorFromSelfEye( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );

  Vec3 vec = lua.str->p - eye;
  pushfloat( vec.x );
  pushfloat( vec.y );
  pushfloat( vec.z );
  return 3;
}

int Lua::ozStrDirectionFromSelf( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  Vec3 dir = ~( lua.str->p - lua.self->p );
  pushfloat( dir.x );
  pushfloat( dir.y );
  pushfloat( dir.z );
  return 3;
}

int Lua::ozStrDirectionFromSelfEye( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );

  Vec3 dir = ~( lua.str->p - eye );
  pushfloat( dir.x );
  pushfloat( dir.y );
  pushfloat( dir.z );
  return 3;
}

int Lua::ozStrDistanceFromSelf( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  pushfloat( !( lua.str->p - lua.self->p ) );
  return 1;
}

int Lua::ozStrDistanceFromSelfEye( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );

  pushfloat( !( lua.str->p - eye ) );
  return 1;
}

int Lua::ozStrRelativeHeadingFromSelf( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  float dx = lua.str->p.x - lua.self->p.x;
  float dy = lua.str->p.y - lua.self->p.y;
  float angle = Math::mod( Math::deg( Math::atan2( -dx, dy ) ) + 360.0f, 360.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozStrHeadingFromSelf( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();
  SELF_BOT();

  float dx = lua.str->p.x - self->p.x;
  float dy = lua.str->p.y - self->p.y;
  float angle = Math::mod( Math::deg( Math::atan2( -dx, dy ) - self->h ) + 720.0f, 360.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozStrPitchFromSelf( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();

  float dx = lua.str->p.x - lua.self->p.x;
  float dy = lua.str->p.y - lua.self->p.y;
  float dz = lua.str->p.z - lua.self->p.z;
  float angle = Math::deg( Math::atan2( dz, Math::sqrt( dx*dx + dy*dy ) ) + Math::TAU / 4.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozStrPitchFromSelfEye( lua_State* l )
{
  ARG( 0 );
  STR_NOT_NULL();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );

  float dx = lua.str->p.x - eye.x;
  float dy = lua.str->p.y - eye.y;
  float dz = lua.str->p.z - eye.z;
  float angle = Math::deg( Math::atan2( dz, Math::sqrt( dx*dx + dy*dy ) ) + Math::TAU / 4.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozStrBindAllOverlaps( lua_State* l )
{
  ARG( 3 );
  STR_NOT_NULL();

  AABB aabb = AABB( lua.str->p,
                    Vec3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ) );

  lua.objects.clear();
  lua.structs.clear();
  collider.getOverlaps( aabb, &lua.objects, &lua.structs );
  lua.objIndex = 0;
  lua.strIndex = 0;
  return 0;
}

int Lua::ozStrBindStrOverlaps( lua_State* l )
{
  ARG( 3 );
  STR_NOT_NULL();

  AABB aabb = AABB( lua.str->p,
                    Vec3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ) );

  lua.structs.clear();
  collider.getOverlaps( aabb, null, &lua.structs );
  lua.strIndex = 0;
  return 0;
}

int Lua::ozStrBindObjOverlaps( lua_State* l )
{
  ARG( 3 );
  STR_NOT_NULL();

  AABB aabb = AABB( lua.str->p,
                    Vec3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ) );

  lua.objects.clear();
  collider.getOverlaps( aabb, &lua.objects, null );
  lua.objIndex = 0;
  return 0;
}

/*
 * Object
 */

int Lua::ozEventBindNext( lua_State* l )
{
  ARG( 0 );

  if( lua.isFirstEvent ) {
    lua.isFirstEvent = false;
    pushbool( true );
  }
  else if( lua.event.isValid() ) {
    ++lua.event;
    pushbool( true );
  }
  else {
    pushbool( false );
  }
  return 1;
}

int Lua::ozEventGet( lua_State* l )
{
  ARG( 0 );
  EVENT_NOT_NULL();

  if( !lua.event.isValid() ) {
    ERROR( "event is null" );
  }
  pushint( lua.event->id );
  pushfloat( lua.event->intensity );
  return 2;
}

int Lua::ozObjBindIndex( lua_State* l )
{
  ARG( 1 );

  int index = toint( 1 );
  if( uint( index ) >= uint( orbis.objects.length() ) ) {
    ERROR( "invalid object index" );
  }
  lua.obj = orbis.objects[index];
  return 0;
}

int Lua::ozObjBindPilot( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  lua.obj = vehicle->pilot == -1 ? null : orbis.objects[vehicle->pilot];
  return 0;
}

int Lua::ozObjBindSelf( lua_State* l )
{
  ARG( 0 );

  lua.obj = lua.self;
  return 0;
}

int Lua::ozObjBindUser( lua_State* l )
{
  ARG( 0 );

  lua.obj = lua.user;
  return 0;
}

int Lua::ozObjBindNext( lua_State* l )
{
  ARG( 0 );

  if( lua.objIndex < lua.objects.length() ) {
    lua.obj = lua.objects[lua.objIndex];
    ++lua.objIndex;
    pushbool( true );
  }
  else {
    pushbool( false );
  }
  return 1;
}

int Lua::ozObjIsNull( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj == null );
  return 1;
}

int Lua::ozObjIsSelf( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj == lua.self );
  return 1;
}

int Lua::ozObjIsUser( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj == lua.user );
  return 1;
}

int Lua::ozObjIsCut( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj != null && lua.obj->cell == null );
  return 1;
}

int Lua::ozObjIsBrowsable( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj != null && ( lua.obj->flags & Object::BROWSABLE_BIT ) );
  return 1;
}

int Lua::ozObjIsDynamic( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj != null && ( lua.obj->flags & Object::DYNAMIC_BIT ) );
  return 1;
}

int Lua::ozObjIsItem( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj != null && ( lua.obj->flags & Object::ITEM_BIT ) );
  return 1;
}

int Lua::ozObjIsWeapon( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj != null && ( lua.obj->flags & Object::WEAPON_BIT ) );
  return 1;
}

int Lua::ozObjIsBot( lua_State* l )
{
  ARG( 0 );

  const Bot* bot = static_cast<const Bot*>( lua.obj );
  pushbool( bot != null && ( bot->flags & Object::BOT_BIT ) && !( bot->state & Bot::DEAD_BIT ) );
  return 1;
}

int Lua::ozObjIsVehicle( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.obj != null && ( lua.obj->flags & Object::VEHICLE_BIT ) );
  return 1;
}

int Lua::ozObjGetIndex( lua_State* l )
{
  ARG( 0 );

  if( lua.obj == null ) {
    pushint( -1 );
  }
  else {
    pushint( lua.obj->index );
  }
  return 1;
}

int Lua::ozObjGetPos( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  if( lua.obj->cell == null ) {
    hard_assert( lua.obj->flags & Object::DYNAMIC_BIT );

    const Dynamic* dyn = static_cast<const Dynamic*>( lua.obj );

    if( dyn->parent != -1 ) {
      Object* parent = orbis.objects[dyn->parent];

      if( parent != null ) {
        pushfloat( parent->p.x );
        pushfloat( parent->p.y );
        pushfloat( parent->p.z );
        return 3;
      }
    }
  }

  pushfloat( lua.obj->p.x );
  pushfloat( lua.obj->p.y );
  pushfloat( lua.obj->p.z );
  return 3;
}

int Lua::ozObjSetPos( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();

  lua.obj->p.x = tofloat( 1 );
  lua.obj->p.y = tofloat( 2 );
  lua.obj->p.z = tofloat( 3 );
  return 0;
}

int Lua::ozObjAddPos( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();

  lua.obj->p.x += tofloat( 1 );
  lua.obj->p.y += tofloat( 2 );
  lua.obj->p.z += tofloat( 3 );
  return 0;
}

int Lua::ozObjGetDim( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  pushfloat( lua.obj->dim.x );
  pushfloat( lua.obj->dim.y );
  pushfloat( lua.obj->dim.z );
  return 3;
}

int Lua::ozObjGetFlags( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  int mask = toint( 1 );
  pushbool( ( lua.obj->flags & mask ) != 0 );
  return 1;
}

int Lua::ozObjGetHeading( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  pushint( lua.obj->flags & Object::HEADING_MASK );
  return 1;
}

int Lua::ozObjGetClassName( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  pushstring( lua.obj->clazz->name );
  return 1;
}

int Lua::ozObjGetLife( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  pushfloat( lua.obj->life );
  return 1;
}

int Lua::ozObjSetLife( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  lua.obj->life = clamp( tofloat( 1 ), 0.0f, lua.obj->clazz->life );
  return 0;
}

int Lua::ozObjAddLife( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  lua.obj->life = clamp( lua.obj->life + tofloat( 1 ),
                         0.0f,
                         lua.obj->clazz->life );
  return 0;
}

int Lua::ozObjAddEvent( lua_State* l )
{
  ARG( 2 );
  OBJ_NOT_NULL();

  int   id        = toint( 1 );
  float intensity = tofloat( 2 );

  if( id >= 0 && intensity < 0.0f ) {
    ERROR( "event intensity for sounds (id >= 0) has to be > 0.0" );
  }
  lua.obj->addEvent( id, intensity );
  return 0;
}

int Lua::ozObjAddItem( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  if( lua.obj->items.length() == lua.obj->clazz->nItems ) {
    pushbool( false );
    return 1;
  }

  int index = toint( 1 );
  if( uint( index ) >= uint( orbis.objects.length() ) ) {
    ERROR( "invalid object index" );
  }

  Dynamic* obj = static_cast<Dynamic*>( orbis.objects[index] );
  if( !( obj->flags & Object::ITEM_BIT ) ) {
    ERROR( "object is not an item" );
  }
  if( obj->cell == null ) {
    ERROR( "object is already cut" );
  }

  synapse.cut( obj );
  lua.obj->items.add( obj->index );
  obj->parent = lua.obj->index;

  pushbool( true );
  return 0;
}

int Lua::ozObjRemoveItem( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  int item = toint( 1 );
  if( uint( item ) >= uint( lua.obj->items.length() ) ) {
    ERROR( "invalid item number" );
  }

  int index = lua.obj->items[item];
  Dynamic* dyn = static_cast<Dynamic*>( orbis.objects[index] );

  if( dyn != null ) {
    synapse.remove( dyn );
    lua.obj->items.remove( item );
  }
  return 0;
}

int Lua::ozObjRemoveAllItems( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  foreach( item, lua.obj->items.citer() ) {
    Dynamic* dyn = static_cast<Dynamic*>( orbis.objects[*item] );

    if( dyn != null ) {
      synapse.remove( dyn );
    }
  }

  lua.obj->items.clear();
  return 0;
}

int Lua::ozObjEnableUpdate( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  if( tobool( 1 ) ) {
    lua.obj->flags |= Object::UPDATE_FUNC_BIT;
  }
  else {
    lua.obj->flags &= ~Object::UPDATE_FUNC_BIT;
  }
  return 0;
}

int Lua::ozObjDamage( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();

  lua.obj->damage( tofloat( 1 ) );
  return 0;
}

int Lua::ozObjDestroy( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  lua.obj->life = 0.0f;
  return 0;
}

int Lua::ozObjQuietDestroy( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  lua.obj->flags |= Object::DESTROYED_BIT;
  return 0;
}

int Lua::ozObjRemove( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  if( lua.obj->cell == null ) {
    Dynamic* dyn = static_cast<Dynamic*>( lua.obj );

    hard_assert( dyn->flags & Object::DYNAMIC_BIT );

    synapse.remove( dyn );
  }
  else {
    synapse.remove( lua.obj );
  }

  lua.self = lua.self == lua.obj ? null : lua.self;
  lua.user = lua.user == lua.obj ? null : lua.user;
  lua.obj  = null;
  return 0;
}

int Lua::ozObjVectorFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();

  Vec3 vec = lua.obj->p - lua.self->p;

  pushfloat( vec.x );
  pushfloat( vec.y );
  pushfloat( vec.z );
  return 3;
}

int Lua::ozObjVectorFromSelfEye( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );
  Vec3   vec = lua.obj->p - eye;

  pushfloat( vec.x );
  pushfloat( vec.y );
  pushfloat( vec.z );
  return 3;
}

int Lua::ozObjDirectionFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();

  Vec3 dir = ~( lua.obj->p - lua.self->p );

  pushfloat( dir.x );
  pushfloat( dir.y );
  pushfloat( dir.z );
  return 3;
}

int Lua::ozObjDirectionFromSelfEye( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );
  Vec3   dir = ~( lua.obj->p - eye );

  pushfloat( dir.x );
  pushfloat( dir.y );
  pushfloat( dir.z );
  return 3;
}

int Lua::ozObjDistanceFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();

  pushfloat( !( lua.obj->p - lua.self->p ) );
  return 1;
}

int Lua::ozObjDistanceFromSelfEye( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );

  pushfloat( !( lua.obj->p - eye ) );
  return 1;
}

int Lua::ozObjHeadingFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();

  float dx = lua.obj->p.x - lua.self->p.x;
  float dy = lua.obj->p.y - lua.self->p.y;
  float angle = Math::mod( Math::deg( Math::atan2( -dx, dy ) ) + 360.0f, 360.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozObjRelativeHeadingFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  SELF_BOT();

  float dx = lua.obj->p.x - self->p.x;
  float dy = lua.obj->p.y - self->p.y;
  float angle = Math::mod( Math::deg( Math::atan2( -dx, dy ) - self->h ) + 720.0f, 360.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozObjPitchFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();

  float dx = lua.obj->p.x - lua.self->p.x;
  float dy = lua.obj->p.y - lua.self->p.y;
  float dz = lua.obj->p.z - lua.self->p.z;
  float angle = Math::deg( Math::atan2( dz, Math::sqrt( dx*dx + dy*dy ) ) + Math::TAU / 4.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozObjPitchFromSelfEye( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  SELF_BOT();

  Point3 eye = Point3( self->p.x, self->p.y, self->p.z + self->camZ );

  float dx = lua.obj->p.x - eye.x;
  float dy = lua.obj->p.y - eye.y;
  float dz = lua.obj->p.z - eye.z;
  float angle = Math::deg( Math::atan2( dz, Math::sqrt( dx*dx + dy*dy ) ) + Math::TAU / 4.0f );

  pushfloat( angle );
  return 1;
}

int Lua::ozObjIsVisibleFromSelf( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();

  Point3 eye    = Point3( lua.self->p.x, lua.self->p.y, lua.self->p.z );
  Vec3   vector = lua.obj->p - eye;

  collider.translate( eye, vector, lua.obj );
  pushbool( collider.hit.ratio == 1.0f );
  return 1;
}

int Lua::ozObjIsVisibleFromSelfEye( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  SELF_BOT();

  Point3 eye    = Point3( self->p.x, self->p.y, self->p.z + self->camZ );
  Vec3   vector = lua.obj->p - eye;

  collider.translate( eye, vector, lua.obj );
  pushbool( collider.hit.ratio == 1.0f );
  return 1;
}

int Lua::ozObjBindEvents( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  lua.event = lua.obj->events.citer();
  lua.isFirstEvent = true;
  return 0;
}

int Lua::ozObjBindItems( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();

  lua.objects.clear();
  foreach( item, lua.obj->items.citer() ) {
    lua.objects.add( orbis.objects[*item] );
  }
  lua.objIndex = 0;
  return 0;
}

int Lua::ozObjBindAllOverlaps( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();

  AABB aabb = AABB( lua.obj->p,
                    Vec3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ) );

  lua.objects.clear();
  lua.structs.clear();
  collider.getOverlaps( aabb, &lua.objects, &lua.structs );
  lua.objIndex = 0;
  lua.strIndex = 0;
  return 0;
}

int Lua::ozObjBindStrOverlaps( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();

  AABB aabb = AABB( lua.obj->p,
                    Vec3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ) );

  lua.structs.clear();
  collider.getOverlaps( aabb, null, &lua.structs );
  lua.strIndex = 0;
  return 0;
}

int Lua::ozObjBindObjOverlaps( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();

  AABB aabb = AABB( lua.obj->p,
                    Vec3( tofloat( 1 ), tofloat( 2 ), tofloat( 3 ) ) );

  lua.objects.clear();
  collider.getOverlaps( aabb, &lua.objects, null );
  lua.objIndex = 0;
  return 0;
}

/*
 * Dynamic object
 */

int Lua::ozDynBindParent( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  if( dyn->parent != -1 && orbis.objects[dyn->parent] != null ) {
    lua.obj = orbis.objects[dyn->parent];
    pushbool( true );
  }
  else {
    pushbool( false );
  }
  return 1;
}

int Lua::ozDynGetVelocity( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  pushfloat( dyn->velocity.x );
  pushfloat( dyn->velocity.y );
  pushfloat( dyn->velocity.z );
  return 3;
}

int Lua::ozDynSetVelocity( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  dyn->flags &= ~Object::DISABLED_BIT;
  dyn->velocity.x = tofloat( 1 );
  dyn->velocity.y = tofloat( 2 );
  dyn->velocity.z = tofloat( 3 );
  return 0;
}

int Lua::ozDynAddVelocity( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  dyn->flags &= ~Object::DISABLED_BIT;
  dyn->velocity.x += tofloat( 1 );
  dyn->velocity.y += tofloat( 2 );
  dyn->velocity.z += tofloat( 3 );
  return 0;
}

int Lua::ozDynGetMomentum( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  pushfloat( dyn->momentum.x );
  pushfloat( dyn->momentum.y );
  pushfloat( dyn->momentum.z );
  return 3;
}

int Lua::ozDynSetMomentum( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  dyn->flags &= ~Object::DISABLED_BIT;
  dyn->momentum.x = tofloat( 1 );
  dyn->momentum.y = tofloat( 2 );
  dyn->momentum.z = tofloat( 3 );
  return 0;
}

int Lua::ozDynAddMomentum( lua_State* l )
{
  ARG( 3 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  dyn->flags &= ~Object::DISABLED_BIT;
  dyn->momentum.x += tofloat( 1 );
  dyn->momentum.y += tofloat( 2 );
  dyn->momentum.z += tofloat( 3 );
  return 0;
}

int Lua::ozDynGetMass( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  pushfloat( dyn->mass );
  return 1;
}

int Lua::ozDynGetLift( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_DYNAMIC();

  pushfloat( dyn->lift );
  return 1;
}

/*
 * Weapon
 */

int Lua::ozWeaponGetDefaultRounds( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_WEAPON();

  const WeaponClass* weaponClazz = static_cast<const WeaponClass*>( weapon->clazz );

  pushint( weaponClazz->nRounds );
  return 1;
}

int Lua::ozWeaponGetRounds( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_WEAPON();

  pushint( weapon->nRounds );
  return 1;
}

int Lua::ozWeaponSetRounds( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_WEAPON();

  const WeaponClass* weaponClazz = static_cast<const WeaponClass*>( weapon->clazz );

  weapon->nRounds = clamp( toint( 1 ), -1, weaponClazz->nRounds );
  return 1;
}

int Lua::ozWeaponAddRounds( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_WEAPON();

  const WeaponClass* weaponClazz = static_cast<const WeaponClass*>( weapon->clazz );

  if( weapon->nRounds != -1 ) {
    weapon->nRounds = min( weapon->nRounds + toint( 1 ), weaponClazz->nRounds );
  }
  return 1;
}

int Lua::ozWeaponReload( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_WEAPON();

  const WeaponClass* weaponClazz = static_cast<const WeaponClass*>( weapon->clazz );

  weapon->nRounds = weaponClazz->nRounds;
  return 1;
}

/*
 * Bot
 */

int Lua::ozBotBindPilot( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  lua.obj = vehicle->pilot == -1 ? null : orbis.objects[vehicle->pilot];
  return 0;
}

int Lua::ozBotGetName( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushstring( bot->name );
  return 1;
}

int Lua::ozBotSetName( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->name = tostring( 1 );
  return 0;
}

int Lua::ozBotGetMindFunc( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushstring( bot->mindFunc );
  return 1;
}

int Lua::ozBotSetMindFunc( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->mindFunc = tostring( 1 );
  return 0;
}

int Lua::ozBotGetState( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  int mask = toint( 1 );
  pushbool( ( bot->state & mask ) != 0 );
  return 1;
}

int Lua::ozBotGetEyePos( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushfloat( bot->p.x );
  pushfloat( bot->p.y );
  pushfloat( bot->p.z + bot->camZ );
  return 3;
}

int Lua::ozBotGetH( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushfloat( Math::deg( bot->h ) );
  return 1;
}

int Lua::ozBotSetH( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->h = Math::rad( tofloat( 1 ) );
  bot->h = Math::mod( bot->h + Math::TAU, Math::TAU );
  return 0;
}

int Lua::ozBotAddH( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->h += Math::rad( tofloat( 1 ) );
  bot->h = Math::mod( bot->h + Math::TAU, Math::TAU );
  return 0;
}

int Lua::ozBotGetV( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushfloat( Math::deg( bot->v ) );
  return 1;
}

int Lua::ozBotSetV( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->v = Math::rad( tofloat( 1 ) );
  bot->v = clamp( bot->v, 0.0f, Math::TAU / 2.0f );
  return 0;
}

int Lua::ozBotAddV( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->v += Math::rad( tofloat( 1 ) );
  bot->v = clamp( bot->v, 0.0f, Math::TAU / 2.0f );
  return 0;
}

int Lua::ozBotGetDir( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  // { hsine, hcosine, vsine, vcosine, vsine * hsine, vsine * hcosine }
  float hvsc[6];

  Math::sincos( bot->h, &hvsc[0], &hvsc[1] );
  Math::sincos( bot->v, &hvsc[2], &hvsc[3] );

  hvsc[4] = hvsc[2] * hvsc[0];
  hvsc[5] = hvsc[2] * hvsc[1];

  pushfloat( -hvsc[4] );
  pushfloat(  hvsc[5] );
  pushfloat( -hvsc[3] );

  return 3;
}

int Lua::ozBotGetStamina( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushfloat( bot->stamina );
  return 1;
}

int Lua::ozBotSetStamina( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  const BotClass* clazz = static_cast<const BotClass*>( bot->clazz );

  bot->stamina = clamp( tofloat( 1 ), 0.0f, clazz->stamina );
  return 0;
}

int Lua::ozBotAddStamina( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  const BotClass* clazz = static_cast<const BotClass*>( bot->clazz );

  bot->stamina = clamp( bot->stamina + tofloat( 1 ), 0.0f, clazz->stamina );
  return 0;
}

int Lua::ozBotActionForward( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_FORWARD;
  return 0;
}

int Lua::ozBotActionBackward( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_BACKWARD;
  return 0;
}

int Lua::ozBotActionRight( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_RIGHT;
  return 0;
}

int Lua::ozBotActionLeft( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_LEFT;
  return 0;
}

int Lua::ozBotActionJump( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_JUMP;
  return 0;
}

int Lua::ozBotActionCrouch( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_CROUCH;
  return 0;
}

int Lua::ozBotActionUse( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_USE;
  return 0;
}

int Lua::ozBotActionTake( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_TAKE;
  return 0;
}

int Lua::ozBotActionGrab( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_GRAB;
  return 0;
}

int Lua::ozBotActionThrow( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_THROW;
  return 0;
}

int Lua::ozBotActionAttack( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_ATTACK;
  return 0;
}

int Lua::ozBotActionExit( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_EXIT;
  return 0;
}

int Lua::ozBotActionEject( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_EJECT;
  return 0;
}

int Lua::ozBotActionSuicide( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->actions |= Bot::ACTION_SUICIDE;
  return 0;
}

int Lua::ozBotIsRunning( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  pushbool( bot->state & Bot::RUNNING_BIT );
  return 1;
}

int Lua::ozBotSetRunning( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  if( tobool( 1 ) ) {
    bot->state |= Bot::RUNNING_BIT;
  }
  else {
    bot->state &= ~Bot::RUNNING_BIT;
  }
  return 0;
}

int Lua::ozBotToggleRunning( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->state ^= Bot::RUNNING_BIT;
  return 0;
}

int Lua::ozBotSetGesture( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->state &= ~( Bot::GESTURE0_BIT | Bot::GESTURE1_BIT | Bot::GESTURE2_BIT | Bot::GESTURE4_BIT );
  bot->state |= toint( 1 );
  return 0;
}

int Lua::ozBotSetWeaponItem( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  int item = toint( 1 );
  if( item == -1 ) {
    bot->weapon = -1;
  }
  else {
    if( uint( item ) >= uint( lua.obj->items.length() ) ) {
      ERROR( "invalid item number" );
    }

    int index = lua.obj->items[item];
    Weapon* weapon = static_cast<Weapon*>( orbis.objects[index] );

    if( weapon == null ) {
      pushbool( false );
      return 1;
    }

    if( !( weapon->flags & Object::WEAPON_BIT ) ) {
      ERROR( "object is not a weapon" );
    }

    const WeaponClass* clazz = static_cast<const WeaponClass*>( weapon->clazz );
    if( clazz->allowedUsers.contains( bot->clazz ) ) {
      bot->weapon = index;
    }
  }

  pushbool( true );
  return 1;
}

int Lua::ozBotHeal( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->heal();
  return 0;
}

int Lua::ozBotRearm( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->rearm();
  return 0;
}

int Lua::ozBotKill( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_BOT();

  bot->kill();
  return 0;
}

int Lua::ozBotIsVisibleFromSelfEyeToEye( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_NOT_SELF();
  OBJ_BOT();
  SELF_BOT();

  Point3 eye    = Point3( self->p.x, self->p.y, self->p.z + self->camZ );
  Vec3   vector = Point3( bot->p.x, bot->p.y, bot->p.z + bot->camZ ) - eye;

  collider.translate( eye, vector, lua.obj );
  pushbool( collider.hit.ratio == 1.0f );
  return 1;
}

/*
 * Vehicle
 */

int Lua::ozVehicleGetH( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  pushfloat( Math::deg( vehicle->h ) );
  return 1;
}

int Lua::ozVehicleSetH( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  vehicle->h = Math::rad( tofloat( 1 ) );
  vehicle->h = Math::mod( vehicle->h + Math::TAU, Math::TAU );

  vehicle->rot = Quat::rotZYX( vehicle->h, 0.0f, vehicle->v - Math::TAU / 4.0f );
  return 0;
}

int Lua::ozVehicleAddH( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  vehicle->h += Math::rad( tofloat( 1 ) );
  vehicle->h = Math::mod( vehicle->h + Math::TAU, Math::TAU );

  vehicle->rot = Quat::rotZYX( vehicle->h, 0.0f, vehicle->v - Math::TAU / 4.0f );
  return 0;
}

int Lua::ozVehicleGetV( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  pushfloat( Math::deg( vehicle->v ) );
  return 1;
}

int Lua::ozVehicleSetV( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  vehicle->v = Math::rad( tofloat( 1 ) );
  vehicle->v = clamp( vehicle->v, 0.0f, Math::TAU / 2.0f );

  vehicle->rot = Quat::rotZYX( vehicle->h, 0.0f, vehicle->v - Math::TAU / 4.0f );
  return 0;
}

int Lua::ozVehicleAddV( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  vehicle->v += Math::rad( tofloat( 1 ) );
  vehicle->v = clamp( vehicle->v, 0.0f, Math::TAU / 2.0f );

  vehicle->rot = Quat::rotZYX( vehicle->h, 0.0f, vehicle->v - Math::TAU / 4.0f );
  return 0;
}

int Lua::ozVehicleGetDir( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  // { hsine, hcosine, vsine, vcosine, vsine * hsine, vsine * hcosine }
  float hvsc[6];

  Math::sincos( vehicle->h, &hvsc[0], &hvsc[1] );
  Math::sincos( vehicle->v, &hvsc[2], &hvsc[3] );

  hvsc[4] = hvsc[2] * hvsc[0];
  hvsc[5] = hvsc[2] * hvsc[1];

  pushfloat( -hvsc[4] );
  pushfloat(  hvsc[5] );
  pushfloat( -hvsc[3] );

  return 3;
}

int Lua::ozVehicleEmbarkPilot( lua_State* l )
{
  ARG( 1 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  if( vehicle->pilot != -1 ) {
    ERROR( "vehicle already has a pilot" );
  }

  int index = toint( 1 );
  if( uint( index ) >= uint( orbis.objects.length() ) ) {
    ERROR( "invalid bot index" );
  }

  Bot* bot = static_cast<Bot*>( orbis.objects[index] );
  if( !( bot->flags & Object::BOT_BIT ) ) {
    ERROR( "object is not a bot" );
  }
  if( bot->cell == null ) {
    ERROR( "bot is already cut" );
  }

  vehicle->pilot = index;
  bot->enter( vehicle->index );
  return 0;
}

int Lua::ozVehicleService( lua_State* l )
{
  ARG( 0 );
  OBJ_NOT_NULL();
  OBJ_VEHICLE();

  vehicle->service();
  return 0;
}

/*
 * Particle
 */

int Lua::ozPartBindIndex( lua_State* l )
{
  ARG( 1 );

  int index = toint( 1 );
  if( uint( index ) >= uint( orbis.parts.length() ) ) {
    ERROR( "invalid particle index" );
  }
  lua.part = orbis.parts[index];
  return 0;
}

int Lua::ozPartIsNull( lua_State* l )
{
  ARG( 0 );

  pushbool( lua.part == null );
  return 1;
}

int Lua::ozPartGetIndex( lua_State* l )
{
  ARG( 0 );

  if( lua.part == null ) {
    pushint( -1 );
  }
  else {
    pushint( lua.part->index );
  }
  return 1;
}

int Lua::ozPartGetPos( lua_State* l )
{
  ARG( 0 );
  PART_NOT_NULL();

  pushfloat( lua.part->p.x );
  pushfloat( lua.part->p.y );
  pushfloat( lua.part->p.z );
  return 3;
}

int Lua::ozPartSetPos( lua_State* l )
{
  ARG( 3 );
  PART_NOT_NULL();

  lua.part->p.x = tofloat( 1 );
  lua.part->p.y = tofloat( 2 );
  lua.part->p.z = tofloat( 3 );
  return 0;
}

int Lua::ozPartAddPos( lua_State* l )
{
  ARG( 3 );
  PART_NOT_NULL();

  lua.part->p.x += tofloat( 1 );
  lua.part->p.y += tofloat( 2 );
  lua.part->p.z += tofloat( 3 );
  return 0;
}

int Lua::ozPartGetVelocity( lua_State* l )
{
  ARG( 0 );
  PART_NOT_NULL();

  pushfloat( lua.part->velocity.x );
  pushfloat( lua.part->velocity.y );
  pushfloat( lua.part->velocity.z );
  return 3;
}

int Lua::ozPartSetVelocity( lua_State* l )
{
  ARG( 3 );
  PART_NOT_NULL();

  lua.part->velocity.x = tofloat( 1 );
  lua.part->velocity.y = tofloat( 2 );
  lua.part->velocity.z = tofloat( 3 );
  return 0;
}

int Lua::ozPartAddVelocity( lua_State* l )
{
  ARG( 3 );
  PART_NOT_NULL();

  lua.part->velocity.x += tofloat( 1 );
  lua.part->velocity.y += tofloat( 2 );
  lua.part->velocity.z += tofloat( 3 );
  return 0;
}

int Lua::ozPartGetLife( lua_State* l )
{
  ARG( 0 );
  PART_NOT_NULL();

  pushfloat( lua.part->lifeTime );
  return 1;
}

int Lua::ozPartSetLife( lua_State* l )
{
  ARG( 1 );
  PART_NOT_NULL();

  lua.part->lifeTime = tofloat( 1 );
  return 0;
}

int Lua::ozPartAddLife( lua_State* l )
{
  ARG( 1 );
  PART_NOT_NULL();

  lua.part->lifeTime += tofloat( 1 );
  return 0;
}

int Lua::ozPartRemove( lua_State* l )
{
  ARG( 0 );
  PART_NOT_NULL();

  synapse.remove( lua.part );
  lua.part = null;
  return 0;
}

}
}
