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
 * @file client/Client.hpp
 *
 * Game initialisation and main loop.
 */

#pragma once

#include "client/Stage.hpp"

namespace oz
{
namespace client
{

class Client
{
  private:

    static const int INIT_CONFIG       = 0x0001;
    static const int INIT_SDL          = 0x0002;
    static const int INIT_SDL_VIDEO    = 0x0004;
    static const int INIT_LIBRARY      = 0x0010;
    static const int INIT_STAGE_INIT   = 0x0100;
    static const int INIT_STAGE_LOAD   = 0x0200;
    static const int INIT_RENDER_INIT  = 0x1000;
    static const int INIT_CONTEXT_INIT = 0x2000;
    static const int INIT_AUDIO        = 0x4000;
    static const int INIT_MAIN_LOOP    = 0x8000;

    Stage* stage;
    int    initFlags;

    float  allTime;
    uint   inactiveMillis;
    uint   droppedMillis;
    float  loadingTime;

  public:

    void shutdown();
    void printUsage();

    int  main( int argc, char* argv[] );

};

extern Client client;

}
}
