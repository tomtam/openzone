/*
 *  Names.cc
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING file for details.
 */

#include "stable.hh"

#include "matrix/Names.hh"

namespace oz
{

  Names names;

  const String& Names::genName() const
  {
    assert( names.length() > 0 );

    if( names.length() == 1 ) {
      return names[0];
    }

    int index = Math::randn( names.length() - 1 ) + 1;
    return names[index];
  }

  void Names::init()
  {
    // a default entry, we need it if nothing is read
    names << "";

    log.print( "Reading character name database 'name/names' ..." );

    FILE* file = fopen( "name/names", "r" );
    if( file == null ) {
      log.printEnd( " Failed" );
      return;
    }

    char buffer[LINE_LENGTH];

    while( fgets( buffer, LINE_LENGTH, file ) != null ) {
      String name = buffer;
      name = name.trim();

      if( !name.isEmpty() ) {
        names << name;
      }
    }
    fclose( file );

    log.printEnd( " OK" );
  }

  void Names::free()
  {
    names.clear();
  }

}