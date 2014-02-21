/*
 * ozCore - OpenZone Core Library.
 *
 * Copyright © 2002-2013 Davorin Učakar
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file ozCore/Pepper.cc
 */

#ifdef __native_client__

#include "Pepper.hh"

#include "List.hh"
#include "SpinLock.hh"
#include "Thread.hh"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/core.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/fullscreen.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/mouse_lock.h>

extern "C"
void SDL_NACL_SetInstance( PP_Instance, int, int );

extern "C"
void SDL_NACL_PushEvent( const pp::InputEvent& );

OZ_WEAK
void SDL_NACL_SetInstance( PP_Instance, int, int )
{}

OZ_WEAK
void SDL_NACL_PushEvent( const pp::InputEvent& )
{}

namespace oz
{

static SpinLock      messageLock;
static List<String>  messageQueue;
static Thread        mainThread;

static pp::Core*     ppCore     = nullptr;
static pp::Instance* ppInstance = nullptr;

class Pepper::Instance : public pp::Instance, public pp::MouseLock
{
private:

  pp::Fullscreen fullscreen;

private:

  static void mainThreadMain( void* );
  static void onMouseLocked( void*, int result );

  bool Init( uint32_t argc, const char** argn, const char** argv ) override;
  void DidChangeView( const pp::View& view ) override;
  void DidChangeView( const pp::Rect& position, const pp::Rect& clip ) override;
  void HandleMessage( const pp::Var& message ) override;
  bool HandleInputEvent( const pp::InputEvent& event ) override;
  void MouseLockLost() override;

public:

  explicit Instance( PP_Instance instance );
  ~Instance() override;

};

void Pepper::Instance::mainThreadMain( void* )
{
  char  argv0[] = "";
  char* argv[]  = { argv0, nullptr };

  naclMain( 1, argv );
}

void Pepper::Instance::onMouseLocked( void*, int result )
{
  hasFocus = result == PP_OK;
}

Pepper::Instance::Instance( PP_Instance instance_ ) :
  pp::Instance( instance_ ), pp::MouseLock( this ), fullscreen( this )
{
  MainCall::semaphore.init();

  ppCore     = pp::Module::Get()->core();
  ppInstance = this;

  RequestInputEvents( PP_INPUTEVENT_CLASS_KEYBOARD | PP_INPUTEVENT_CLASS_MOUSE |
                      PP_INPUTEVENT_CLASS_WHEEL );
}

Pepper::Instance::~Instance()
{
  if( mainThread.isValid() ) {
    mainThread.join();
  }

  messageQueue.clear();
  messageQueue.deallocate();

  ppInstance = nullptr;

  MainCall::semaphore.destroy();
}

bool Pepper::Instance::Init( uint32_t, const char**, const char** )
{
  return true;
}

void Pepper::Instance::DidChangeView( const pp::View& view )
{
  int newWidth  = view.GetRect().width();
  int newHeight = view.GetRect().height();

  if( newWidth == width && newHeight == height ) {
    return;
  }

  width  = newWidth;
  height = newHeight;

  if( !mainThread.isValid() ) {
    SDL_NACL_SetInstance( pp_instance(), width, height );

    mainThread.start( "main", Thread::JOINABLE, mainThreadMain, nullptr );
  }
}

void Pepper::Instance::DidChangeView( const pp::Rect&, const pp::Rect& )
{
  PP_NOTREACHED();
}

void Pepper::Instance::HandleMessage( const pp::Var& message )
{
  push( message.AsString().c_str() );
}

bool Pepper::Instance::HandleInputEvent( const pp::InputEvent& event )
{
  switch( event.GetType() ) {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN: {
      if( !Pepper::hasFocus ) {
        LockMouse( pp::CompletionCallback( onMouseLocked, this ) );
        return true;
      }
      break;
    }
    case PP_INPUTEVENT_TYPE_MOUSEMOVE: {
      pp::MouseInputEvent mouseEvent( event );
      pp::Point move = mouseEvent.GetMovement();

      moveX += float( move.x() );
      moveY += float( move.y() );
      break;
    }
    case PP_INPUTEVENT_TYPE_WHEEL: {
      pp::WheelInputEvent wheelEvent( event );
      pp::FloatPoint move = wheelEvent.GetDelta();

      moveZ += move.x();
      moveW += move.y();
      break;
    }
    case PP_INPUTEVENT_TYPE_KEYDOWN: {
      pp::KeyboardInputEvent keyEvent( event );

      if( ( keyEvent.GetKeyCode() == 122 || keyEvent.GetKeyCode() == 13 ) &&
          event.GetModifiers() == 0 )
      {
        fullscreen.SetFullscreen( !fullscreen.IsFullscreen() );
        return true;
      }
      break;
    }
    default: {
      break;
    }
  }

  SDL_NACL_PushEvent( event );
  return true;
}

void Pepper::Instance::MouseLockLost()
{
  hasFocus = false;
  fullscreen.SetFullscreen( false );
}

class Pepper::Module : public pp::Module
{
public:

  pp::Instance* CreateInstance( PP_Instance instance ) override;

};

pp::Instance* Pepper::Module::CreateInstance( PP_Instance instance )
{
  return new Instance( instance );
}

int   Pepper::width    = 0;
int   Pepper::height   = 0;
float Pepper::moveX    = 0.0f;
float Pepper::moveY    = 0.0f;
float Pepper::moveZ    = 0.0f;
float Pepper::moveW    = 0.0f;
bool  Pepper::hasFocus = false;

bool Pepper::isMainThread()
{
  return ppCore->IsMainThread();
}

void Pepper::mainCall( Callback* callback, void* data )
{
  ppCore->CallOnMainThread( 0, pp::CompletionCallback( callback, data ) );
}

pp::Instance* Pepper::instance()
{
  return ppInstance;
}

void Pepper::post( const char* message )
{
  pp::Var var( message );

  MainCall() << [&]()
  {
    ppInstance->PostMessage( var );
  };
}

String Pepper::pop()
{
  messageLock.lock();
  String s = messageQueue.isEmpty() ? String() : messageQueue.popFirst();
  messageLock.unlock();

  return s;
}

void Pepper::push( const char* message )
{
  messageLock.lock();
  messageQueue.pushLast( message );
  messageLock.unlock();
}

pp::Module* Pepper::createModule()
{
  return new Module();
}

Semaphore MainCall::semaphore;

}

#endif