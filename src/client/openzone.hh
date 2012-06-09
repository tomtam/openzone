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
 * @file client/openozne.hh
 *
 * Platform-specific main function implementation.
 */

#pragma once

#ifdef __native_client__

#include "oz/oz.hh"

#include "client/NaClGLContext.hh"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/fullscreen.h>
#include <ppapi/cpp/mouse_lock.h>

int ozMain( int argc, char** argv );

namespace oz
{
namespace client
{

class MainInstance : public pp::Instance, public pp::MouseLock
{
  private:

    pp::Fullscreen fullscreen;
    bool           isContextBound;
    bool           isMouseLocked;
    pthread_t      mainThread;

    static void* mainThreadMain( void* );

  public:

    explicit MainInstance( PP_Instance instance );
    virtual ~MainInstance();

    virtual bool Init( uint32_t argc, const char** argn, const char** argv );
    virtual void DidChangeView( const pp::View& view );
    virtual void DidChangeView( const pp::Rect& position, const pp::Rect& clip );
    virtual bool HandleInputEvent( const pp::InputEvent& event );
    virtual void MouseLockLost();

    static void Empty( void*, int );
    static void DidMouseLock( void* data, int result );

};

class MainModule : public pp::Module
{
  public:

    virtual pp::Instance* CreateInstance( PP_Instance instance );

};

}
}

namespace pp
{

pp::Module* CreateModule();

}

#endif // __native_client__