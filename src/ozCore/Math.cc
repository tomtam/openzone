/*
 * ozCore - OpenZone Core Library.
 *
 * Copyright © 2002-2016 Davorin Učakar
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

#include "Math.hh"

#include <cstdlib>

namespace oz
{

constexpr float Math::FLOAT_EPS;
constexpr float Math::NaN;
constexpr float Math::INF;
constexpr float Math::E;
constexpr float Math::TAU;

void Math::seed(int n)
{
  srand(n);
}

int Math::rand(int max)
{
  return ::rand() % max;
}

float Math::rand()
{
  return float(::rand()) / float(RAND_MAX);
}

float Math::centralRand()
{
  return float(::rand() - RAND_MAX / 2) / float(RAND_MAX / 2);
}

float Math::normalRand()
{
  return tan(centralRand() * TAU / 8.0f);
}

}
