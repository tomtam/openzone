/*
 *  Render.h
 *
 *  Graphics render engine
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#pragma once

#include "matrix/World.h"

#include "ui/ui.h"

#include "BSP.h"
#include "Terrain.h"
#include "OBJ.h"
#include "MD2.h"
#include "Model.h"

namespace oz
{
namespace client
{

  class Render
  {
    private:

      static const float RELEASED_CULL_FACTOR = 6.0f;

      static const float BLACK_COLOR[];
      static const float WHITE_COLOR[];

      static const float TAG_COLOR[];
      static const float AABB_COLOR[];

      static const float GLOBAL_AMBIENT_COLOR[];

      static const float NIGHT_FOG_COEFF;
      static const float NIGHT_FOG_DIST;
      static const float WATER_VISIBILITY;

      static const float STAR_SIZE;

      static const int   DELAYED_LISTS_MAX = 256;

      // cleanup interval (remove unused models)
      static const int   CLEAR_INTERVAL = 303 * 1000;

      struct DelayedList
      {
        float distance;
        uint  list;
      };

    private:

      Terrain                 terra;
      Vector<BSP*>            bsps;
      Bitset                  drawnStructures;

      HashIndex<Model*, 1021> models;
      int                     clearCount;

      Vector<Structure*>      structures;
      Vector<Object*>         objects;
      Vector<Object*>         blendedObjects;
      Vector<Particle*>       particles;

      Vector<Structure*>      waterStructures;

      ui::CrosshairArea       *crosshairArea;

      int                     screenX;
      int                     screenY;

      double                  perspectiveAngle;
      double                  perspectiveAspect;
      double                  perspectiveMin;
      double                  perspectiveMax;

      float                   dayVisibility;
      float                   nightVisibility;
      float                   waterDayVisibility;
      float                   waterNightVisibility;

      float                   particleRadius;
      bool                    drawAABBs;
      bool                    showAim;

      bool                    isUnderWater;
      bool                    wasUnderWater;
      float                   visibility;
      int                     taggedObjIndex;

      void drawObject( Object *obj );
      void scheduleCell( int cellX, int cellY );

    public:

      bool                    doScreenshot;

      void sync();
      void update();

      void init();
      void free();

      void load();
      void unload();
  };

  extern Render render;

}
}
