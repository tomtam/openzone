/*
 *  Stage.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING for details.
 */

#pragma once

namespace oz
{
namespace client
{

  struct Stage
  {
    public:

      virtual ~Stage() {}

      virtual bool update() = 0;
      virtual void render() = 0;

      virtual void load()   = 0;
      virtual void unload() = 0;

  };

}
}
