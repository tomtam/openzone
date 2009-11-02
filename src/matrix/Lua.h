/*
 *  Lua.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#pragma once

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace oz
{

  struct Lua
  {
    public:

      struct API
      {
        const char    *name;
        lua_CFunction func;
      };

    private:

      lua_State *state;

    public:

      operator lua_State* ()
      {
        return state;
      }

      void init();
      void free();

      void loadAPI( const API *apis );
      bool load( const char *file );

  };

}
