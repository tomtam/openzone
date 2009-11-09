/*
 *  Lua.cpp
 *
 *  Lua scripting engine for Matrix
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#include "precompiled.h"

#include "Lua.h"

#include "Bot.h"
#include "BotClass.h"
#include "Synapse.h"
#include "Collider.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#define OZ_LUA_ERROR( message ) luaL_error( l, "[%s] %s", __FUNCTION__, message );
#define OZ_LUA_REGISTER( func ) lua_register( l, #func, func )

namespace oz
{

  Lua lua;

  const char *Lua::HANDLERS_FILE = "lua/handlers.luac";

  static int ozPrintln( lua_State *l )
  {
    log.println( "M> %s", lua_tostring( l, 1 ) );
    return 0;
  }

  static int ozException( lua_State *l )
  {
    const char *message = lua_tostring( l, 1 );
    throw Exception( message );
  }

  static int ozBindSelf( lua_State* )
  {
    lua.obj = &lua.self;
    return 0;
  }

  static int ozBindUser( lua_State* )
  {
    lua.obj = &lua.user;
    return 0;
  }

  static int ozBindCreated( lua_State* )
  {
    lua.obj = &lua.created;
    return 0;
  }

  static int ozIsNull( lua_State *l )
  {
    lua_pushboolean( l, *lua.obj == null );
    return 1;
  }

  static int ozIsPut( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }
    lua_pushboolean( l, ( *lua.obj )->cell != null && ( ~( *lua.obj )->flags & Object::CUT_BIT ) );
    return 1;
  }

  static int ozIsDynObj( lua_State *l )
  {
    lua_pushboolean( l, *lua.obj != null && ( ( *lua.obj )->flags & Object::DYNAMIC_BIT ) );
    return 1;
  }

  static int ozIsBot( lua_State *l )
  {
    lua_pushboolean( l, *lua.obj != null && ( ( *lua.obj )->flags & Object::BOT_BIT ) );
    return 1;
  }

  static int ozGetPos( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    lua_pushnumber( l, ( *lua.obj )->p.x );
    lua_pushnumber( l, ( *lua.obj )->p.y );
    lua_pushnumber( l, ( *lua.obj )->p.z );
    return 3;
  }

  static int ozSetPos( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    ( *lua.obj )->p.x = lua_tonumber( l, 1 );
    ( *lua.obj )->p.y = lua_tonumber( l, 2 );
    ( *lua.obj )->p.z = lua_tonumber( l, 3 );
    return 0;
  }

  static int ozAddPos( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    ( *lua.obj )->p.x += lua_tonumber( l, 1 );
    ( *lua.obj )->p.y += lua_tonumber( l, 2 );
    ( *lua.obj )->p.z += lua_tonumber( l, 3 );
    return 0;
  }

  static int ozGetDim( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    lua_pushnumber( l, ( *lua.obj )->dim.x );
    lua_pushnumber( l, ( *lua.obj )->dim.y );
    lua_pushnumber( l, ( *lua.obj )->dim.z );
    return 3;
  }

  static int ozGetLife( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    lua_pushnumber( l, ( *lua.obj )->life );
    return 1;
  }

  static int ozSetLife( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    ( *lua.obj )->life = bound( lua_tonumber( l, 1 ), 0, ( *lua.obj )->type->life );
    return 0;
  }

  static int ozAddLife( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    ( *lua.obj )->life = bound( ( *lua.obj )->life + lua_tonumber( l, 1 ),
                                0,
                                ( *lua.obj )->type->life );
    return 0;
  }

  static int ozGetStamina( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }
    if( ~( *lua.obj )->flags & Object::BOT_BIT ) {
      OZ_LUA_ERROR( "selected object is not a Bot" );
    }

    Bot *bot = static_cast<Bot*>( *lua.obj );
    lua_pushnumber( l, bot->stamina );
    return 1;
  }

  static int ozSetStamina( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }

    if( ~( *lua.obj )->flags & Object::BOT_BIT ) {
      OZ_LUA_ERROR( "selected object is not a Bot" );
    }

    Bot *bot = static_cast<Bot*>( *lua.obj );
    BotClass *clazz = static_cast<BotClass*>( bot->type );
    bot->stamina = bound( lua_tonumber( l, 1 ), 0, clazz->stamina );
    return 0;
  }

  static int ozAddStamina( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }
    if( ~( *lua.obj )->flags & Object::BOT_BIT ) {
      OZ_LUA_ERROR( "selected object is not a Bot" );
    }

    Bot *bot = static_cast<Bot*>( *lua.obj );
    BotClass *clazz = static_cast<BotClass*>( bot->type );
    bot->stamina = bound( bot->stamina + lua_tonumber( l, 1 ), 0, clazz->stamina );
    return 0;
  }

  static int ozCreateObj( lua_State *l )
  {
    if( lua.created->cell == null ) {
      delete lua.created;
    }

    Vec3 pos = Vec3( lua_tonumber( l, 2 ), lua_tonumber( l, 3 ), lua_tonumber( l, 4 ) );
    lua.created = translator.createObject( lua_tostring( l, 1 ), pos );
    lua.obj = &lua.created;
    return 0;
  }

  static int ozDeleteCreatedObj( lua_State *l )
  {
    if( lua.created == null ) {
      OZ_LUA_ERROR( "created object is null" );
    }
    if( lua.created->cell != null ) {
      OZ_LUA_ERROR( "cannot delete created object once it has been put into world" );
    }

    delete lua.created;
    lua.created = null;
    return 0;
  }

  static int ozPutObj( lua_State *l )
  {
    if( lua.created == null ) {
      OZ_LUA_ERROR( "created object is null" );
    }
    if( lua.created->cell != null ) {
      OZ_LUA_ERROR( "created object has already been put into world" );
    }

    lua.created->p = Vec3( lua_tonumber( l, 1 ), lua_tonumber( l, 2 ), lua_tonumber( l, 3 ) );

    if( collider.test( *lua.created ) ) {
      synapse.put( lua.created );
      lua_pushboolean( l, true );
    }
    else {
      lua_pushboolean( l, false );
    }
    return 1;
  }

  static int ozForcePutObj( lua_State *l )
  {
    if( lua.created == null ) {
      OZ_LUA_ERROR( "created object is null" );
    }
    if( lua.created->cell != null ) {
      OZ_LUA_ERROR( "created object has already been put into the world" );
    }

    lua.created->p = Vec3( lua_tonumber( l, 1 ), lua_tonumber( l, 2 ), lua_tonumber( l, 3 ) );
    synapse.put( lua.created );
    return 0;
  }

  static int ozRemoveObj( lua_State *l )
  {
    if( *lua.obj == null ) {
      OZ_LUA_ERROR( "selected object is null" );
    }
    if( ( *lua.obj )->cell == null || ( ( *lua.obj )->flags & Object::CUT_BIT ) ) {
      OZ_LUA_ERROR( "selected object isn't put into the world" );
    }

    synapse.remove( *lua.obj );
    *lua.obj = null;
    return 0;
  }

  void Lua::callFunc( const char *functionName )
  {
    assert( self != null );

    obj     = &self;
    created = null;

    lua_getglobal( l, functionName );
    lua_pcall( l, 0, 0, 0 );

    if( lua_gettop( l ) != 0 ) {
      if( lua_isstring( l, 1 ) ) {
        log.println( "M! %s", lua_tostring( l, 1 ) );
      }
      lua_settop( l, 0 );
    }

    if( created != null ) {
      delete created;
    }
  }

  void Lua::init()
  {
    log.println( "Initializing Matrix Lua {" );
    log.indent();

    l = lua_open();
    luaL_openlibs( l );

    OZ_LUA_REGISTER( ozPrintln );
    OZ_LUA_REGISTER( ozException );

    OZ_LUA_REGISTER( ozBindSelf );
    OZ_LUA_REGISTER( ozBindUser );
    OZ_LUA_REGISTER( ozBindCreated );
    OZ_LUA_REGISTER( ozIsNull );
    OZ_LUA_REGISTER( ozIsPut );
    OZ_LUA_REGISTER( ozIsDynObj );
    OZ_LUA_REGISTER( ozIsBot );

    OZ_LUA_REGISTER( ozGetPos );
    OZ_LUA_REGISTER( ozSetPos );
    OZ_LUA_REGISTER( ozAddPos );
    OZ_LUA_REGISTER( ozGetDim );

    OZ_LUA_REGISTER( ozGetLife );
    OZ_LUA_REGISTER( ozSetLife );
    OZ_LUA_REGISTER( ozAddLife );

    OZ_LUA_REGISTER( ozGetStamina );
    OZ_LUA_REGISTER( ozSetStamina );
    OZ_LUA_REGISTER( ozAddStamina );

    OZ_LUA_REGISTER( ozCreateObj );
    OZ_LUA_REGISTER( ozDeleteCreatedObj );

    OZ_LUA_REGISTER( ozPutObj );
    OZ_LUA_REGISTER( ozForcePutObj );
    OZ_LUA_REGISTER( ozRemoveObj );

    for( int i = 0; i < translator.matrixScripts.length(); i++ ) {
      const Translator::Resource &res = translator.matrixScripts[i];

      log.print( "Processing '%s' ...", res.path.cstr() );

      if( luaL_dofile( l, res.path ) != 0 ) {
        log.printEnd( " Failed" );
      }
      else {
        log.printEnd( " OK" );
      }
    }

    log.unindent();
    log.println( "}" );
  }

  void Lua::free()
  {
    log.print( "Freeing Matrix Lua ..." );
    lua_close( l );
    log.printEnd( " OK" );
  }

}
