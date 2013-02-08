--[[
 * OpenZone - simple cross-platform FPS/RTS game engine.
 * Copyright (C) 2002-2012  Davorin Učakar
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
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
]]--

--[[
 * mission/test.lua
 *
 * Test world.
]]--

function onUpdate()
  ozUIShowBuild( true )
end

function onCreate()
  ozTerraLoad( "terra" )

  ozCaelumLoad( "earth" )
  ozCaelumSetHeading( math.pi / 8 )
  ozCaelumSetPeriod( 3600 )
  ozCaelumSetTime( 450 )

  oz_me = ozOrbisAddObj( OZ_FORCE, "beast", -1005.96, 603.80, 51.10, true )
  ozBotSetH( 20 )
  ozBotSetName( ozProfileGetName() )
  ozBotSetMind( "" )

  ozOrbisAddStr( OZ_FORCE, "beastBunker", -1014.0, 620.0, 52.0, OZ_NORTH )
  ozOrbisAddStr( OZ_FORCE, "beastBunker", -1029.0, 610.0, 51.5, OZ_WEST )
  ozOrbisAddStr( OZ_FORCE, "beastBunker", -1010.0, 595.0, 53.0, OZ_SOUTH )

  ozOrbisAddObj( OZ_FORCE, "smallCrate", -1022.30, 596.59, 51.54 )
  ozOrbisAddObj( OZ_FORCE, "smallCrate", -1021.96, 597.80, 51.45 )
  ozOrbisAddObj( OZ_FORCE, "smallCrate", -1023.41, 596.91, 50.29 )
  ozOrbisAddObj( OZ_FORCE, "bigCrate", -1022.48, 596.59, 50.63 )
  ozOrbisAddObj( OZ_FORCE, "bigCrate", -1023.38, 597.93, 50.52 )
  ozOrbisAddObj( OZ_FORCE, "metalCrate", -1021.95, 597.90, 50.54 )
  ozObjAddItem( "musicPlayer" )
  ozObjAddItem( "musicPlayer" )
  ozObjAddItem( "beast$plasmagun" )
  ozObjAddItem( "beast$plasmagun" )
  ozObjAddItem( "nvGoggles" )
  ozObjAddItem( "galileo" )
  ozObjAddItem( "firstAid" )
  ozObjAddItem( "firstAid" )
  ozObjAddItem( "firstAid" )
  ozObjAddItem( "firstAid" )
  ozObjAddItem( "bomb" )
  ozObjAddItem( "bomb" )

  ozFloraSeed()

  local bounds = { -1100, -900, 500, 700 }

  for i = 1, 200 do
    local x = -1050 + 100 * math.random()
    local y = 550 + 100 * math.random()
    local z = ozTerraHeight( x, y )

    if z > 0 then
      ozOrbisAddObj( OZ_TRY, "zombie", x, y, z + 1.10 )
    end
  end

  ozCameraAddSwitchableUnit( oz_me )
  ozCameraSwitchTo( oz_me )
--   ozCameraAllowReincarnation( false )
end