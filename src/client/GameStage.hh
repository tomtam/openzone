/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 * @file client/GameStage.hh
 */

#pragma once

#include "client/Stage.hh"
#include "client/Proxy.hh"
#include "client/Module.hh"

namespace oz
{
namespace client
{

class GameStage : public Stage
{
  public:

    enum State
    {
      GAME,
      MENU
    };

    static String AUTOSAVE_FILE;
    static String QUICKSAVE_FILE;

  private:

    SDL_Thread*     auxThread;
    SDL_sem*        mainSemaphore;
    SDL_sem*        auxSemaphore;
    volatile bool   isAlive;

    bool            isLoaded;

    uint            loadingMillis;
    uint            uiMillis;
    uint            loaderMillis;
    uint            soundMillis;
    uint            renderMillis;
    uint            matrixMillis;
    uint            nirvanaMillis;

    Vector<Module*> modules;

  public:

    State  state;
    Proxy* proxy;

    String stateFile;
    String mission;

  private:

    bool read( const char* path );
    void write( const char* path ) const;
    void reload();

    static int auxMain( void* );

    void auxRun();

  public:

    virtual bool update();
    virtual void present( bool full );

    virtual void load();
    virtual void unload();

    virtual void init();
    virtual void free();

};

extern GameStage gameStage;

}
}
