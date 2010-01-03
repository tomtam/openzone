/*
 *  Task.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#include "precompiled.h"

#include "Task.h"

namespace oz
{
namespace nirvana
{

  void Task::write( OutputStream* ostream ) const
  {
    foreach( child, children.iterator() ) {
      ostream->writeString( child->type() );
      child->write( ostream );
    }
    ostream->writeInt( flags );
  }

}
}
