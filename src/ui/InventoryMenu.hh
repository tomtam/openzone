/*
 *  InventoryMenu.hh
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING file for details.
 */

#pragma once

#include "ui/Frame.hh"

namespace oz
{
namespace client
{
namespace ui
{

  class InventoryMenu : public Frame
  {
    private:

      static const int   ICON_SIZE   = 32;
      static const int   SLOT_SIZE   = 64;
      static const float SLOT_DIMF;
      static const int   COLS        = 6;
      static const int   ROWS        = 2;
      static const int   HEADER_SIZE = 20;
      static const int   FOOTER_SIZE = 40;

      uint useTexId;
      int  taggedIndex;
      int  row;

    protected:

      virtual bool onMouseEvent();
      virtual void onDraw();

    public:

      explicit InventoryMenu();
      ~InventoryMenu();

  };

}
}
}