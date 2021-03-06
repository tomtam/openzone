/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2016 Davorin Učakar
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
 * @file client/ui/Inventory.hh
 */

#pragma once

#include <client/ui/Bar.hh>
#include <client/ui/ModelField.hh>
#include <client/ui/Frame.hh>

namespace oz
{
namespace client
{
namespace ui
{

class Inventory : public Frame
{
private:

  static const int COLS          = 8;
  static const int FOOTER_SIZE   = 32;
  static const int ICON_SIZE     = 32;
  static const int SLOT_SIZE     = 64;
  static const int SLOT_OBJ_DIM  = SLOT_SIZE / 2;
  static const int SINGLE_HEIGHT = FOOTER_SIZE + SLOT_SIZE;

  const Bot*    owner;
  const Object* other;

  Bar           lifeBar;
  Bar           statusBar;

  ModelField*   ownerModels[COLS];
  ModelField*   otherModels[COLS];

  Text          itemDesc;

  int           taggedItemIndex;
  bool          taggedOwnerComponent;

  int           scrollOwner;
  int           scrollOther;

private:

  static void ownerItemCallback(ModelField* sender, bool isClicked);
  static void otherItemCallback(ModelField* sender, bool isClicked);

  void updateReferences();
  void handleScroll(const Object* container, int* scroll);
  void drawComponent(int height, const Object* container, const Dynamic* taggedItem, int scroll);

protected:

  void onVisibilityChange(bool doShow) override;
  bool onMouseEvent() override;
  void onUpdate() override;
  void onDraw() override;

public:

  Inventory();

};

}
}
}
