/*
 *  DebugFrame.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING file for details.
 */

#pragma once

#include "ui/Frame.hpp"

namespace oz
{
namespace client
{
namespace ui
{

  class DebugFrame : public Frame
  {
    protected:

      virtual void onDraw();

    public:

      explicit DebugFrame();

  };

}
}
}