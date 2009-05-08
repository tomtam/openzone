/*
 *  precompiled.h
 *
 *  Precompiled header.
 *  It should be included in all ".cpp" source files in the project. Do not include it via another
 *  header, it will not work in GCC and VC++.
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "src/base/base.h"

// include SDL, OpenGL and OpenAL as those are commonly used
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_opengl.h>
#include <AL/alut.h>
