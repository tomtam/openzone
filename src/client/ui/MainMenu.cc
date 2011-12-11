/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
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
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
 */

/**
 * @file client/ui/MainMenu.cc
 */

#include "stable.hh"

#include "client/ui/MainMenu.hh"

#include "client/Shader.hh"
#include "client/Shape.hh"
#include "client/GameStage.hh"
#include "client/MenuStage.hh"
#include "client/OpenGL.hh"

#include "client/ui/SettingsFrame.hh"
#include "client/ui/UI.hh"

namespace oz
{
namespace client
{
namespace ui
{

String MainMenu::autosaveFile;
String MainMenu::quicksaveFile;

static void continueAutosaved( Button* sender )
{
  ui.root->remove( sender->parent );

  Stage::nextStage = &gameStage;
  gameStage.stateFile = GameStage::AUTOSAVE_FILE;
}

static void continueQuicksaved( Button* sender )
{
  ui.root->remove( sender->parent );

  Stage::nextStage = &gameStage;
  gameStage.stateFile = GameStage::QUICKSAVE_FILE;
}

static void loadTutorial( Button* sender )
{
  ui.root->remove( sender->parent );

  Stage::nextStage = &gameStage;
  gameStage.stateFile = "";
  gameStage.missionFile = "tutorial";
}

static void loadTest( Button* sender )
{
  ui.root->remove( sender->parent );

  Stage::nextStage = &gameStage;
  gameStage.stateFile = "";
  gameStage.missionFile = "test";
}

static void loadCvicek( Button* sender )
{
  ui.root->remove( sender->parent );

  Stage::nextStage = &gameStage;
  gameStage.stateFile = "";
  gameStage.missionFile = "cvicek";
}

static void settings( Button* sender )
{
  ui.root->remove( sender->parent );
  ui.root->add( new SettingsFrame() );
}

static void exit( Button* sender )
{
  ui.root->remove( sender->parent );
  menuStage.doExit = true;
}

MainMenu::MainMenu() : Frame( 0, 0, 400, 450, OZ_APPLICATION_TITLE " v" OZ_APPLICATION_VERSION ),
    copyright( 10, 10, 380, 9, Font::SANS )
{
  x = ( Area::uiWidth  - width ) / 2;
  y = ( Area::uiHeight - height ) / 2;

  copyright.setText( "OpenZone  Copyright © 2002-2011  Davorin Učakar\n\n%s",
      gettext( "This program comes with ABSOLUTELY NO WARRANTY. "
               "This is free software, and you are welcome to redistribute it "
               "under certain conditions; See COPYING file for details.\n\n"
               "Data files come form different sources. See respective README and COPYING "
               "files for details about copyrights and licences." ) );

  File autosaveFile( String::str( "%s/autosave.ozState", config.get( "dir.rc", "" ) ) );
  File quicksaveFile( String::str( "%s/quicksave.ozState", config.get( "dir.rc", "" ) ) );

  if( autosaveFile.getType() == File::REGULAR ) {
    MainMenu::autosaveFile = autosaveFile.path();
    add( new Button( gettext( "Continue" ), continueAutosaved, 300, 20 ), 50, -60 );
  }
  if( quicksaveFile.getType() == File::REGULAR ) {
    MainMenu::quicksaveFile = quicksaveFile.path();
    add( new Button( gettext( "Load Quicksave" ), continueQuicksaved, 300, 20 ), 50, -90 );
  }

  add( new Button( gettext( "Tutorial" ), loadTutorial, 300, 20 ), 50, -150 );
  add( new Button( gettext( "Test World" ), loadTest, 300, 20 ), 50, -180 );
  add( new Button( gettext( "Mission 1: Cvicek" ), loadCvicek, 300, 20 ), 50, -210 );

  add( new Button( gettext( "Settings" ), settings, 300, 20 ), 50, -270 );
  add( new Button( gettext( "Exit" ), exit, 300, 20 ), 50, -300 );
}

MainMenu::~MainMenu()
{
  autosaveFile  = "";
  quicksaveFile = "";
}

void MainMenu::onDraw()
{
  glUniform4f( param.oz_Colour, 0.1f, 0.1f, 0.1f, 1.0f );
  shape.fill( 0, 0, Area::uiWidth, Area::uiHeight );

  Frame::onDraw();

  copyright.draw( this );
}

}
}
}