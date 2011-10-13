--[[
 *  tutorial.lua
 *  Tutorial mission
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
]]--

function onUpdate()
end

function onCreate()
  ozTerraLoad( "terra" )

  ozCaelumLoad( "earth" )
  ozCaelumSetHeading( math.pi / 8 )
  ozCaelumSetPeriod( 3600 )
  ozCaelumSetTime( 450 )

  local me = ozOrbisAddObj( "beast", 78, -90, 65.55 )
  ozBotSetName( ozProfileGetPlayerName() )
  ozBotSetMindFunc( "" )
  ozObjRemoveAllItems()

  local oX, oY, oZ = 80, -80, 69
  ozOrbisAddStr( "tutorial_movement", oX, oY, oZ, OZ_STRUCT_R0 )

  local infoMove = ozOrbisAddObj( "datacube", oX - 2, oY - 9, oZ - 3 )
  local infoClimb = ozOrbisAddObj( "datacube", oX - 2.5, oY + 1, oZ - 2.5 )
  local infoLadder = ozOrbisAddObj( "datacube", oX - 4, oY + 3, oZ + 2 )
  local infoSwim = ozOrbisAddObj( "datacube", oX + 4, oY + 5, oZ + 2 )
  local infoStamina = ozOrbisAddObj( "datacube", oX + 2, oY - 7, oZ + 2 )

  ozNirvanaAddMemo( infoMove,
    ozGettext( "1. MOVEMENT\n\nUse W, A, S, D to move and mouse to look around.\n\nYou can jump with SPACEBAR, toggle crouch with LCTRL and toggle running with LSHIFT.\n\nIf you crouch you are a bit lower, so you can go under some obstacles, but you cannot run." )
  )

  ozNirvanaAddMemo( infoClimb,
    ozGettext( "2. LEDGE CLIMBING\n\nYou can climb on a ledge if you hold SPACEBAR and W simultaneously." )
  )

  ozNirvanaAddMemo( infoLadder,
    ozGettext( "3. LADDERS\n\nTo move up or down a ladder just look in the desired direction while you are touching the ladder.\n\nIf you move away from the ladder, you will fall from it.\n\nBe careful when you want to go down the ladder. Approach the ladder slowly or you may miss it and fall.\n\nNote that you cannot climb a ladder when you are carrying some object in your hands." )
  )

  ozNirvanaAddMemo( infoSwim,
    ozGettext( "4. SWIMMING\n\nFor swimming you use the same controls as for basic movement.\n\nTo dive, just look down and move forwards. You must only watch that you are running (and not crouching), as you must swim fast enough to dive." )
  )

  ozNirvanaAddMemo( infoStamina,
    ozGettext( "5. STAMINA\n\nYou probably noticed the blue bar in bottom left corner starts to drain if you are running, jumping, climbing, throwing objects or swimming under water. That is you stamina. If it runs out, you are not able to perform those actions any more.\n\nBe careful it doesn't run out while you are under water or you will start to drawn.\n\nYour stamina is restoring while you are still or walking." )
  )

  local oX, oY, oZ = 60, -92, 69
  ozOrbisAddStr( "tutorial_carrying", oX, oY, oZ, OZ_STRUCT_R90 )

  ozOrbisAddObj( "metalCrate", oX + 3.2, oY - 4.5, oZ - 3 )
  ozObjRemoveAllItems()

  ozOrbisAddObj( "bigCrate", oX + 1, oY - 3, oZ - 3 )
  ozOrbisAddObj( "bigCrate", oX, oY - 1, oZ - 3 )
  ozOrbisAddObj( "smallCrate", oX + 1.1, oY - 3, oZ - 2 )
  ozOrbisAddObj( "smallCrate", oX - 2.5, oY - 2, oZ - 3 )
  ozOrbisAddObj( "smallCrate", oX - 1, oY - 2.5, oZ - 3 )

  local infoPush = ozOrbisAddObj( "datacube", oX + 6, oY - 2, oZ - 2.5 )
  local infoCarry = ozOrbisAddObj( "datacube", oX - 4, oY - 2.5, oZ - 2.5 )
  local infoWaterPush = ozOrbisAddObj( "datacube", oX - 6, oY, oZ + 1.5 )

  ozNirvanaAddMemo( infoPush,
    ozGettext( "6. PUSHING\n\nYou can push around many object you see. It's also possible to push them to the side if you don't walk into them directly, but slightly to one side (diagonally)." )
  )

  ozNirvanaAddMemo( infoCarry,
    ozGettext( "7. CARRYING\n\nYou can also carry around objects that you are strong enough to lift. Press middle mouse button to lift/drop an object when you see the 'lift' or 'carrying' icon under your crosshair. To throw the object you are carrying roll your mouse weel up.\n\nThere are some limitations though. E.g. you cannot lift objects while swimming, climbing a ladder or holding a weapon. You also cannot lift somebody who is already carrying something and you cannot run while carrying an object.\n\nThe object you carry will fall from your hands if you jump of if it touches some other object above (so you cannot carry several objects stacked on one another).\n\nIf somebody else carries you, you can free yourself with jumping.\n\nNow use the available boxes to get to the next room." )
  )

  ozNirvanaAddMemo( infoWaterPush,
    ozGettext( "8. PUSHING IN WATER\n\nYou will need a create from the previous room for this one. Bring it here, throw it in the water and push it undr water to the other side. There you will need the crate to jump on it to climb on a ledge." )
  )

  local oX, oY, oZ = 82, -55, 69.2
  ozOrbisAddStr( "tutorial_items", oX, oY, oZ, OZ_STRUCT_R0 )

  ozOrbisAddObj( "firstAid", oX - 4.5, oY - 3.2, oZ - 1.5 )
  ozOrbisAddObj( "firstAid", oX - 4.5, oY - 2.5, oZ - 1.5 )
  ozOrbisAddObj( "cvicek", oX - 4.5, oY - 2, oZ - 1.5 )
  ozOrbisAddObj( "metalCrate", oX + 4.5, oY - 3, oZ - 1.5 )
  ozOrbisAddObj( "firstAid", oX - 4.5, oY + 2, oZ - 1.5 )
  ozOrbisAddObj( "firstAid", oX - 4.5, oY + 2, oZ - 1.1 )
  ozOrbisAddObj( "cvicek", oX - 4.5, oY + 2, oZ - 0.5 )
  ozOrbisAddObj( "musicPlayer", oX - 4.5, oY + 2.8, oZ - 0.5 )
  ozOrbisAddObj( "galileo", oX - 4.5, oY + 3.5, oZ - 0.5 )

  local infoUI = ozOrbisAddObj( "datacube", oX - 1, oY - 4, oZ - 1 )
  local infoItem = ozOrbisAddObj( "datacube", oX - 4, oY - 1, oZ - 1 )
  local infoExchange = ozOrbisAddObj( "datacube", oX + 4, oY - 1, oZ - 1 )
  local infoHealth = ozOrbisAddObj( "datacube", oX - 4, oY + 1, oZ - 1 )
  local infoSpecial = ozOrbisAddObj( "datacube", oX - 4, oY + 4, oZ - 1 )
  local infoSaving = ozOrbisAddObj( "datacube", oX + 4, oY + 3, oZ - 1 )

  ozNirvanaAddMemo( infoUI,
    ozGettext( "9. USER INTERFACE\n\nUse TAB key to toggle between freelook and UI mode. In UI mode you see several other windows (depending on situation) and manage your inventory.\n\nYou can still look around in UI mode by pushing mouse cursor into edges of the screen.\n\nWindows can be dragged around with left mouse button while holding the ALT key." )
  )

  ozNirvanaAddMemo( infoItem,
    ozGettext( "10. INVENTORY\n\nIf you see a blue hand icon right to your crosshair, it means you can activate the object with right click.\n\nPurple icon left to your crosshair means the item can be put into your inventory if you roll mouse wheel down. You can only put items into your inventory if you have enough room.\n\nBy left clicking an item in you inventory interface you will drop it and with middle click you will start to carry it like boxes in previous tutorial. For both actions you need enough room in front of you.\n\nYou can still activate items while they are in your inventory (blue hand is shown at the bottom of inventory window if the selected item can be activated)." )
  )

  ozNirvanaAddMemo( infoExchange,
    ozGettext( "11. TRANSFER BETWEEN INVENTORIES\n\nIf you see two arrows left of you crosshair, you can browse object in front of you and swap items between it and your inventory.\n\nIf you roll your mouse wheel down, both inventories will be shown. With left click you can transfer items between the inventories.\n\nSometimes inventory is to big for all slots to be shown. In such case you can scroll slots in the inventory windows with your mouse wheel. Arrows on top left and/or bottom left side are shown when the inventory can be scrolled up and/or down." )
  )

  ozNirvanaAddMemo( infoHealth,
    ozGettext( "12. HEALTH\n\nFirst aid kits are disposable, you can only use them once. If you are not seriously hurt it's probably best to spare first aid for a later time. There's also a bonus if you are a beast (which you currently are): your health regenerates slowly.\n\nOne first aid kit restores 100 units of health. Beasts like you have 150 units of health.\n\nYour health is show in a green bar above stamina (becomes red when your health drops). If you are heavily wounded, you are unable to run." )
  )

  ozNirvanaAddMemo( infoSpecial,
    ozGettext( "13. SPECIAL ITEMS\n\nThere are also some items that give you special abilities when in your inventory. Music player and Galileo satellite navigation are such examples. When you poses them a music player interface and a minimap windows will be shown." )
  )

  ozNirvanaAddMemo( infoSaving,
    ozGettext( "14. SAVING AND CAMERA\n\nThere are some dangers ahead. You can quick-save with F5 and quick-load with F7.\n\nEvery time you exit the level, it is auto-saved. To load last auto-saved state press F8.\n\nTo toggle to 3rd person view use NUMPAD ENTER and to toggle free camera rotation press NUMPAD *." )
  )

  ozOrbisAddObj( "bomb", oX + 25, oY + 17, oZ - 2 )
  ozOrbisAddObj( "bomb", oX + 25, oY + 18, oZ - 2 )

  local infoBombs = ozOrbisAddObj( "datacube", oX + 23, oY + 17.5, oZ - 2 )

  ozNirvanaAddMemo( infoBombs,
    ozGettext( "15. BOMBS\n\nSome objects like this bombs are dangerous. They will explode when destroyed or 5 s after activation (they can also be deactivated by right clicking them again)." )
  )

  local cvicek = ozOrbisAddObj( "cvicek", 0, 0, 0 )
  local firstAid = ozOrbisAddObj( "firstAid", 0, 0, 0 )

  ozOrbisAddObj( "beast_weapon.plasmagun", oX - 5, oY + 20, oZ - 2 )
  ozOrbisAddObj( "metalBarrel", oX - 10, oY + 30, oZ )
  ozOrbisAddObj( "metalBarrel", oX - 8, oY + 30, oZ )

  ozOrbisAddObj( "droid", oX - 5, oY + 30, oZ )
  ozObjRemoveAllItems()
  ozObjAddItem( cvicek )
  ozBotSetH( 180 )
  ozBotSetMindFunc( "" )

  ozOrbisAddObj( "droid.OOM-9", oX - 2.5, oY + 30, oZ )
  ozObjRemoveAllItems()
  ozObjAddItem( firstAid )
  ozBotSetH( 180 )
  ozBotSetMindFunc( "" )

  ozOrbisAddObj( "metalCrate", oX, oY + 30, oZ - 1 )
  ozObjRemoveAllItems()

  local infoWeapon = ozOrbisAddObj( "datacube", oX - 6, oY + 20, oZ - 0.5 )
  local infoCorpse = ozOrbisAddObj( "datacube", oX - 6, oY + 30, oZ )

  ozNirvanaAddMemo( infoWeapon,
    ozGettext( "16. WEAPONS\n\nTo use a weapon you first need to put in in your inventory. By right clicking it in you inventory interface you can arm (red fist icon) and disarm it (grey fist icon).\n\nThere's also a shortcut: when you right click a weapon laying on the floor you will both put it in your inventory and arm it at the same time.\n\nYou are only able to use weapons suitable for your bot class (e.g. if you are a beast you cannot use droid weapons and vice versa).\n\nTo fire your current weapon press left mouse button.\n\nTest that plasma gun on objects ahead and those two droids." )
  )

  ozNirvanaAddMemo( infoCorpse,
    ozGettext( "17. CORPSES\n\nCorpses can also be searched for items. Use them like the metal container before.\n\nBut don't wait too long. Corpses disappear some time after death." )
  )

  ozOrbisAddObj( "hoverTank", 55, -30, 72 )
  ozOrbisAddObj( "raptor", 45, -30, 73 )
  ozOrbisAddObj( "serviceStation", 35, -30, 74 )

  local infoVehicle = ozOrbisAddObj( "datacube", 58, -33, 71 )
  local infoRaptor = ozOrbisAddObj( "datacube", 50, -35, 72 )
  local infoService = ozOrbisAddObj( "datacube", 35, -35, 73 )

  ozNirvanaAddMemo( infoVehicle,
    ozGettext( "18. VEHICLES\n\nYou can use the same keys to control vehicle as for movement.\n\nTo switch weapons use right mouse button.\n\nTo exit the vehicle press X and to eject press ALT + X. On destruction the vehicle automatically ejects you." )
  )

  ozNirvanaAddMemo( infoRaptor,
    ozGettext( "19. TRUNK\n\nSome vehicles also have a trunk. In that case you can swap items between your inventory and the vehicle as you did for that metal crate. It also works once you are in the vehicle (just press TAB key to open inventory window)." )
  )

  ozNirvanaAddMemo( infoService,
    ozGettext( "20. SERVICE STATION\n\nYou can repair and rearm vehicles at a service station. Park a vehicle close enough, go out and use the service station.\n\nService station will also reload all weapons in your inventory. For mechanical units, it will also heal (repair) them and refill their stamina." )
  )

  ozOrbisAddObj( "goblin", 58, -52, 70 )
  ozBotSetMindFunc( "" )
  ozBotSetH( 270 )
  ozOrbisAddObj( "droid", 58, -48, 70 )
  ozBotSetMindFunc( "" )
  ozBotSetH( 270 )

  local infoReincarnation = ozOrbisAddObj( "datacube", 60, -50, 69 )
  local infoQuests = ozOrbisAddObj( "datacube", 60, -55, 68.5 )

  ozNirvanaAddMemo( infoReincarnation,
    ozGettext( "21. STRATEGIC MODE\n\n(Read to the end before you try it.)\n\nYou can exit the bot you are controlling by pressing I key. That puts you in strategic mode. You move the camera the same way you move your bot. By pressing NUMPAD ENTER you switch between strategic camera (top-down) and free camera. In both modes you can still use TAB key to toggle between user interface mode and freelook mode.\n\nTo incarnate into a bot select it by left clicking and press I.\n\nReincarnation and strategic mode are not always available. Depending on a mission, reincarnation can be limited to specific bots only or completely disabled (including strategic mode)." )
  )

  ozNirvanaAddMemo( infoQuests,
    ozGettext( "22. OBJECTIVES\n\nWhen in the user interface mode, you see your current objective on top of the screen. By clicking '+' you can open or close objective description and by clicking '<' and '>' you cycle between all available objectives for the current mission.\n\nIf you posses Galileo gadget, you also see your current objective position on your minimap." )
  )

  ozOrbisAddObj( "beast", 100, -100, 71 )
  ozBotSetH( 0 )
  ozOrbisAddObj( "beast", 100, -50, 70 )
  ozBotSetH( 90 )
  ozOrbisAddObj( "beast", 60, -80, 69 )
  ozBotSetH( 270 )

  local tutorialQuest = ozQuestAdd(
    ozGettext( "Tutorial" ),
    ozGettext( "Get familiar with all aspects of OpenZone game." ),
    60, -46, 69.5
  )
  ozQuestEnd( tutorialQuest, true )

  ozFloraSeed()

  ozObjBindIndex( me )
  ozCameraAllowReincarnation( true )
  ozCameraIncarnate( me )
  ozCameraWarpTo( ozBotGetEyePos() )
end