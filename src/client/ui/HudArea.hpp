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
 * @file client/ui/HudArea.hpp
 */

#pragma once

#include "matrix/Vehicle.hpp"

#include "client/ui/Area.hpp"
#include "client/ui/Label.hpp"

namespace oz
{
namespace client
{
namespace ui
{

class HudArea : public Area
{
  friend class UI;

  private:

    static const int   ICON_SIZE    = 32;
    static const int   VEHICLE_SIZE = 180;
    static const float VEHICLE_DIMF;

    Label title;
    Label weaponName;
    Label weaponRounds;
    Label vehicleWeaponNames[Vehicle::WEAPONS_MAX];
    Label vehicleWeaponRounds[Vehicle::WEAPONS_MAX];

    int   lastTaggedId;
    int   lastWeaponId;
    int   lastWeaponRounds;
    int   lastVehicleId;
    int   lastVehicleWeaponRounds[Vehicle::WEAPONS_MAX];

    uint  crossTexId;
    uint  useTexId;
    uint  deviceTexId;
    uint  equipTexId;
    uint  mountTexId;
    uint  takeTexId;
    uint  browseTexId;
    uint  liftTexId;
    uint  grabTexId;

    int   descTextX;
    int   descTextY;
    int   healthBarX;
    int   healthBarY;
    int   crossIconX;
    int   crossIconY;
    int   leftIconX;
    int   leftIconY;
    int   rightIconX;
    int   rightIconY;
    int   bottomIconX;
    int   bottomIconY;

    void drawBotCrosshair();
    void drawBotStatus();
    void drawVehicleStatus();

  protected:

    virtual void onUpdate();
    virtual void onDraw();

  public:

    HudArea();
    virtual ~HudArea();

};

}
}
}
