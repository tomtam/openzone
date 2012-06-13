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

/**
 * @file nirvana/luaapi.hh
 *
 * Nirvana Lua API implementation.
 */

#pragma once

#include "matrix/luaapi.hh"

#include "nirvana/Memo.hh"
#include "nirvana/Nirvana.hh"

namespace oz
{
namespace nirvana
{

struct NirvanaLuaState
{
  Bot* self;

  bool forceUpdate;
};

static NirvanaLuaState ns;

/// @addtogroup luaapi
/// @{

/*
 * General functions
 */

static int ozForceUpdate( lua_State* l )
{
  ARG( 0 );

  ns.forceUpdate = true;
  return 0;
}

/*
 * Mind's bot
 */

static int ozSelfIsCut( lua_State* l )
{
  ARG( 0 );

  l_pushbool( ns.self->cell == null );
  return 1;
}

static int ozSelfGetIndex( lua_State* l )
{
  ARG( 0 );

  l_pushint( ns.self->index );
  return 1;
}

static int ozSelfGetPos( lua_State* l )
{
  ARG( 0 );

  if( ns.self->cell == null ) {
    if( ns.self->parent != -1 ) {
      Object* parent = orbis.objects[ns.self->parent];

      if( parent != null ) {
        l_pushfloat( parent->p.x );
        l_pushfloat( parent->p.y );
        l_pushfloat( parent->p.z );
        return 3;
      }
    }
  }

  l_pushfloat( ns.self->p.x );
  l_pushfloat( ns.self->p.y );
  l_pushfloat( ns.self->p.z );
  return 3;
}

static int ozSelfGetDim( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->dim.x );
  l_pushfloat( ns.self->dim.y );
  l_pushfloat( ns.self->dim.z );
  return 3;
}

static int ozSelfHasFlag( lua_State* l )
{
  ARG( 1 );

  l_pushbool( ns.self->flags & l_toint( 1 ) );
  return 1;
}

static int ozSelfGetHeading( lua_State* l )
{
  ARG( 0 );

  l_pushint( ns.self->flags & Object::HEADING_MASK );
  return 1;
}

static int ozSelfGetClassName( lua_State* l )
{
  ARG( 0 );

  l_pushstring( ns.self->clazz->name );
  return 1;
}

static int ozSelfMaxLife( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->clazz->life );
  return 1;
}

static int ozSelfGetLife( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->life );
  return 1;
}

static int ozSelfDefaultResistance( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->clazz->resistance );
  return 1;
}

static int ozSelfGetResistance( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->resistance );
  return 1;
}

static int ozSelfGetParent( lua_State* l )
{
  ARG( 0 );

  int parent = ns.self->parent;
  l_pushint( parent != -1 && orbis.objects[parent] == null ? -1 : parent );
  return 1;
}

static int ozSelfGetVelocity( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->velocity.x );
  l_pushfloat( ns.self->velocity.y );
  l_pushfloat( ns.self->velocity.z );
  return 3;
}

static int ozSelfGetMomentum( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->momentum.x );
  l_pushfloat( ns.self->momentum.y );
  l_pushfloat( ns.self->momentum.z );
  return 3;
}

static int ozSelfGetMass( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->mass );
  return 1;
}

static int ozSelfGetLift( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->lift );
  return 1;
}

static int ozSelfGetName( lua_State* l )
{
  ARG( 0 );

  l_pushstring( ns.self->name );
  return 1;
}

static int ozSelfGetMind( lua_State* l )
{
  ARG( 0 );

  l_pushstring( ns.self->mindFunc );
  return 1;
}

static int ozSelfHasState( lua_State* l )
{
  ARG( 1 );

  l_pushbool( ns.self->state & l_toint( 1 ) );
  return 1;
}

static int ozSelfGetEyePos( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->p.x );
  l_pushfloat( ns.self->p.y );
  l_pushfloat( ns.self->p.z + ns.self->camZ );
  return 3;
}

static int ozSelfGetH( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( Math::deg( ns.self->h ) );
  return 1;
}

static int ozSelfSetH( lua_State* l )
{
  ARG( 1 );

  ns.self->h = Math::rad( l_tofloat( 1 ) );
  ns.self->h = angleWrap( ns.self->h );
  return 1;
}

static int ozSelfAddH( lua_State* l )
{
  ARG( 1 );

  ns.self->h += Math::rad( l_tofloat( 1 ) );
  ns.self->h  = angleWrap( ns.self->h );
  return 1;
}

static int ozSelfGetV( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( Math::deg( ns.self->v ) );
  return 1;
}

static int ozSelfSetV( lua_State* l )
{
  ARG( 1 );

  ns.self->v = Math::rad( l_tofloat( 1 ) );
  ns.self->v = clamp( ns.self->v, 0.0f, Math::TAU / 2.0f );
  return 1;
}

static int ozSelfAddV( lua_State* l )
{
  ARG( 1 );

  ns.self->v += Math::rad( l_tofloat( 1 ) );
  ns.self->v  = clamp( ns.self->v, 0.0f, Math::TAU / 2.0f );
  return 1;
}

static int ozSelfGetDir( lua_State* l )
{
  ARG( 0 );

  // { hsine, hcosine, vsine, vcosine, vsine * hsine, vsine * hcosine }
  float hvsc[6];

  Math::sincos( ns.self->h, &hvsc[0], &hvsc[1] );
  Math::sincos( ns.self->v, &hvsc[2], &hvsc[3] );

  hvsc[4] = hvsc[2] * hvsc[0];
  hvsc[5] = hvsc[2] * hvsc[1];

  l_pushfloat( -hvsc[4] );
  l_pushfloat(  hvsc[5] );
  l_pushfloat( -hvsc[3] );

  return 3;
}

static int ozSelfMaxStamina( lua_State* l )
{
  ARG( 0 );

  const BotClass* clazz = static_cast<const BotClass*>( ns.self->clazz );

  l_pushfloat( clazz->stamina );
  return 1;
}

static int ozSelfGetStamina( lua_State* l )
{
  ARG( 0 );

  l_pushfloat( ns.self->stamina );
  return 1;
}

static int ozSelfGetCargo( lua_State* l )
{
  ARG( 0 );

  int cargo = ns.self->cargo;
  l_pushint( cargo != -1 && orbis.objects[cargo] == null ? -1 : cargo );
  return 1;
}

static int ozSelfGetWeapon( lua_State* l )
{
  ARG( 0 );

  int weapon = ns.self->weapon;
  l_pushint( weapon != -1 && orbis.objects[weapon] == null ? -1 : weapon );
  return 1;
}

static int ozSelfSetWeaponItem( lua_State* l )
{
  ARG( 1 );

  int item = l_toint( 1 );
  if( item == -1 ) {
    ns.self->weapon = -1;
  }
  else {
    if( uint( item ) >= uint( ns.self->items.length() ) ) {
      ERROR( "Invalid item number (out of range)" );
    }

    int index = ns.self->items[item];
    Weapon* weapon = static_cast<Weapon*>( orbis.objects[index] );

    if( weapon == null ) {
      l_pushbool( false );
      return 1;
    }

    if( !( weapon->flags & Object::WEAPON_BIT ) ) {
      ERROR( "Invalid item number (not a weapon)" );
    }

    const WeaponClass* clazz = static_cast<const WeaponClass*>( weapon->clazz );
    if( ns.self->clazz->name.beginsWith( clazz->userBase ) ) {
      ns.self->weapon = index;
    }
  }

  l_pushbool( true );
  return 1;
}

static int ozSelfCanReachEntity( lua_State* l )
{
  ARG( 2 );
  STR_INDEX( l_toint( 1 ) );
  ENT_INDEX( l_toint( 2 ) );

  l_pushbool( ns.self->canReach( ent ) );
  return 1;
}

static int ozSelfCanReachObj( lua_State* l )
{
  ARG( 1 );
  OBJ_INDEX( l_toint( 1 ) );

  l_pushbool( ns.self->canReach( obj ) );
  return 1;
}

static int ozSelfAction( lua_State* l )
{
  VARG( 1, 3 );

  int action = l_toint( 1 );
  int arg1   = l_toint( 2 );
  int arg2   = l_toint( 3 );

  if( action & Bot::INSTRUMENT_ACTIONS ) {
    ns.self->actions   &= ~Bot::INSTRUMENT_ACTIONS;
    ns.self->actions   |= action;
    ns.self->instrument = arg1;
    ns.self->container  = arg2;
  }
  else {
    ns.self->actions |= action;
  }
  return 0;
}

static int ozSelfBindItems( lua_State* l )
{
  ARG( 0 );

  ms.objIndex = 0;
  ms.objects.clear();

  foreach( item, ns.self->items.citer() ) {
    hard_assert( *item != -1 );

    ms.objects.add( orbis.objects[*item] );
  }
  return 0;
}

static int ozSelfBindItem( lua_State* l )
{
  ARG( 1 );

  int index = l_toint( 1 );

  if( uint( index ) >= uint( ns.self->items.length() ) ) {
    ERROR( "Invalid inventory item index" );
  }

  ms.obj = orbis.objects[ ns.self->items[index] ];
  return 0;
}

static int ozSelfOverlaps( lua_State* l )
{
  ARG( 2 );

  int  flags = l_toint( 1 );
  AABB aabb  = AABB( *ns.self, l_tofloat( 2 ) );

  if( flags & COLLIDE_ALL_OBJECTS_BIT ) {
    collider.mask = ~0;
  }

  bool overlaps = collider.overlaps( aabb, ns.self );
  collider.mask = Object::SOLID_BIT;

  l_pushbool( overlaps );
  return 1;
}

static int ozSelfBindOverlaps( lua_State* l )
{
  ARG( 2 );

  int  flags = l_toint( 1 );
  AABB aabb  = AABB( *ns.self, l_tofloat( 2 ) );

  if( !( flags & ( COLLIDE_STRUCTS_BIT | COLLIDE_OBJECTS_BIT | COLLIDE_ALL_OBJECTS_BIT ) ) ) {
    ERROR( "At least one of OZ_STRUCTS_BIT, OZ_OBJECTS_BIT or OZ_ALL_OBJECTS_BIT must be given" );
  }

  Vector<Struct*>* structs = null;
  Vector<Object*>* objects = null;

  if( flags & COLLIDE_STRUCTS_BIT ) {
    structs = &ms.structs;

    ms.strIndex = 0;
    ms.structs.clear();
  }
  if( flags & ( COLLIDE_OBJECTS_BIT | COLLIDE_ALL_OBJECTS_BIT ) ) {
    objects = &ms.objects;

    ms.objIndex = 0;
    ms.objects.clear();
  }
  if( flags & COLLIDE_ALL_OBJECTS_BIT ) {
    collider.mask = ~0;
  }

  collider.getOverlaps( aabb, structs, objects );
  collider.mask = Object::SOLID_BIT;
  return 0;
}

/*
 * Nirvana
 */

static int ozNirvanaRemoveDevice( lua_State* l )
{
  ARG( 1 );

  int index = l_toint( 1 );
  const nirvana::Device* const* device = nirvana::nirvana.devices.find( index );

  if( device == null ) {
    l_pushbool( false );
  }
  else {
    delete *device;
    nirvana::nirvana.devices.exclude( index );
    l_pushbool( true );
  }
  return 1;
}

static int ozNirvanaAddMemo( lua_State* l )
{
  ARG( 2 );

  int index = l_toint( 1 );
  if( uint( index ) >= uint( orbis.objects.length() ) ) {
    ERROR( "invalid object index" );
  }
  if( orbis.objects[index] == null ) {
    ERROR( "object is null" );
  }

  if( nirvana::nirvana.devices.contains( index ) ) {
    ERROR( "object already has a device" );
  }

  nirvana::nirvana.devices.add( index, new nirvana::Memo( l_tostring( 2 ) ) );
  return 0;
}

/// @}

}
}
