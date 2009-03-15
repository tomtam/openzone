/*
 *  M_Walker.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

#include "Mind.hpp"

namespace oz
{

  class M_Walker : public Mind
  {
    protected:

      void onUpdate();

    public:

      M_Walker( Bot *body );
  };

}
