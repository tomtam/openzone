/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 * @file builder/Class.hh
 */

#pragma once

#include <builder/common.hh>

#include <matrix/VehicleClass.hh>

namespace oz
{
namespace builder
{

class Class
{
  private:

    static const Mat44 INJURY_COLOUR;

    JSON               config;

    /*
     * Object
     */

    String             name;
    String             title;
    String             description;

    Vec3               dim;
    int                flags;
    float              life;
    float              resistance;

    int                attributes;
    int                key;
    int                cost;

    int                nItems;
    List<String>       defaultItems;

    int                nFrags;
    String             fragPool;

    String             deviceType;

    String             imagoType;
    String             imagoModel;

    String             audioType;
    String             audioSounds[ObjectClass::MAX_SOUNDS];

    String             onDestroy;
    String             onUse;
    String             onUpdate;

    /*
     * Dynamic
     */

    float              mass;
    float              lift;

    /*
     * Weapon
     */

    String             userBase;

    int                nRounds;
    float              shotInterval;

    String             onShot;

    /*
     * Bot
     */

    int                state;

    Vec3               crouchDim;
    Vec3               corpseDim;

    float              camZ;
    float              crouchCamZ;

    float              walkMomentum;
    float              runMomentum;
    float              jumpMomentum;

    float              airControl;
    float              ladderControl;
    float              waterControl;
    float              slickControl;

    float              stepWalkInc;
    float              stepRunInc;

    float              stairInc;
    float              stairMax;
    float              stairRateLimit;
    float              stairRateSupp;

    float              climbInc;
    float              climbMax;
    float              climbMomentum;

    float              stamina;
    float              staminaGain;
    float              staminaRunDrain;
    float              staminaWaterDrain;
    float              staminaClimbDrain;
    float              staminaJumpDrain;
    float              staminaThrowDrain;

    float              regeneration;

    float              reachDist;
    float              grabWeight;
    float              throwMomentum;

    int                weaponItem;
    float              meleeInterval;
    String             onMelee;

    String             nameList;

    String             mindFunc;

    float              bobRotation;
    float              bobAmplitude;
    float              bobSwimAmplitude;

    Mat44              baseColour;
    Mat44              nvColour;
    Mat44              injuryColour;

    /*
     * Vehicle
     */

    VehicleClass::Type type;

    Vec3               pilotPos;

    float              lookHMin;
    float              lookHMax;
    float              lookVMin;
    float              lookVMax;

    float              rotVelLimit;

    float              moveMomentum;

    float              hoverHeight;
    float              hoverHeightStiffness;
    float              hoverMomentumStiffness;

    float              enginePitchBias;
    float              enginePitchRatio;
    float              enginePitchLimit;

    float              fuel;
    float              fuelConsumption;

    int                nWeapons;
    String             weaponTitles[VehicleClass::MAX_WEAPONS];
    String             onWeaponShot[VehicleClass::MAX_WEAPONS];
    int                nWeaponRounds[VehicleClass::MAX_WEAPONS];
    float              weaponShotIntervals[VehicleClass::MAX_WEAPONS];

  public:

    List<String>       names;
    List<String>       bases;
    List<String>       devices;
    List<String>       imagines;
    List<String>       audios;

  private:

    void fillObject( const char* className );
    void fillDynamic( const char* className );
    void fillWeapon( const char* className );
    void fillBot( const char* className );
    void fillVehicle( const char* className );

    void writeObject( OutputStream* os );
    void writeDynamic( OutputStream* os );
    void writeWeapon( OutputStream* os );
    void writeBot( OutputStream* os );
    void writeVehicle( OutputStream* os );

  public:

    void build( OutputStream* os, const char* className );
    void destroy();

};

extern Class clazz;

}
}