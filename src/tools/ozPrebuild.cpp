/*
 *  ozPrebuildData.cpp
 *
 *  Prebuild data
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "Build.hpp"

#include "matrix/Library.hpp"
#include "matrix/QBSP.hpp"

#include "client/Context.hpp"
#include "client/Compiler.hpp"
#include "client/Caelum.hpp"
#include "client/Terra.hpp"
#include "client/BSP.hpp"
#include "client/OBJ.hpp"
#include "client/MD2.hpp"
#include "client/Render.hpp"
#include "client/Module.hpp"

#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <SDL/SDL_main.h>

using namespace oz;

bool Alloc::isLocked = true;

static const char* const CREATE_DIRS[] = {
  "bsp",
  "caelum",
  "class",
  "glsl",
  "lua",
  "lua/matrix",
  "lua/mission",
  "lua/nirvana",
  "mdl",
  "music",
  "name",
  "snd",
  "terra",
  "ui",
  "ui/cur",
  "ui/font",
  "ui/icon"
};

static bool forceRebuild = false;

static void printUsage()
{
  log.println( "Usage:" );
  log.indent();
  log.println( "ozPrebuild [--help] [-f | --force] <prefix>" );
  log.println();
  log.println( "--help" );
  log.println( "\tPrints that help message." );
  log.println();
  log.println( "-f, --force" );
  log.println( "\tForce rebuild of all resources." );
  log.println();
  log.println( "-C, --use-S3TC" );
  log.println( "\tUse S3 texture compression" );
  log.println();
  log.println( "<prefix>" );
  log.println( "\tSets data directory to <prefix>/share/openzone." );
  log.println();
  log.unindent();
}

static void createDirs()
{
  log.println( "Creating directory structure {" );
  log.indent();

  for( int i = 0; i < aLength( CREATE_DIRS ); ++i ) {
    log.print( "%-11s ...", CREATE_DIRS[i] );

    struct stat dirStat;
    if( stat( CREATE_DIRS[i], &dirStat ) == 0 ) {
      log.printEnd( " OK, exists" );
      continue;
    }

    if( !File::mkdir( CREATE_DIRS[i] ) ) {
      log.printEnd( " Failed" );
      throw Exception( "Failed to create directories" );
    }

    log.printEnd( " OK, created" );
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildTextures( const char* srcDir, const char* destDir,
                              bool wrap, int magFilter, int minFilter )
{
  log.println( "Prebuilding textures in '%s' {", srcDir );
  log.indent();

  String sSrcDir = srcDir;
  String sDestDir = destDir;
  File dir( sSrcDir );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + sSrcDir + "'" );
  }

  sSrcDir  = sSrcDir + "/";
  sDestDir = sDestDir + "/";

  foreach( file, dirList.citer() ) {
    if( !file->hasExtension( "png" ) && !file->hasExtension( "jpg" ) ) {
      continue;
    }

    String srcPath = file->path();
    String destPath = sDestDir + file->baseName() + ".ozcTex";

    struct stat srcInfo;
    struct stat destInfo;

    if( stat( srcPath, &srcInfo ) != 0 ) {
      throw Exception( "Source texture '" + srcPath + "' stat error" );
    }
    if( !forceRebuild && stat( destPath, &destInfo ) == 0 &&
        destInfo.st_mtime > srcInfo.st_mtime )
    {
      continue;
    }

    log.println( "Prebuilding texture '%s' {", srcPath.cstr() );
    log.indent();

    uint id = client::context.loadRawTexture( srcPath, wrap, magFilter, minFilter );

    hard_assert( id != 0 );

    Buffer buffer( 4 * 1024 * 1024 );
    OutputStream os = buffer.outputStream();

    log.println( "Compiling into '%s'", destPath.cstr() );
    client::context.writeTexture( id, &os );

    if( !buffer.write( destPath, os.length() ) ) {
      throw Exception( "Texture writing failed" );
    }

    log.unindent();
    log.println( "}" );
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildBSPTextures()
{
  log.println( "Prebuilding BSP textures {" );
  log.indent();

  for( int i = 0; i < library.textures.length(); ++i ) {
    if( !library.usedTextures.get( i ) ) {
      continue;
    }

    String srcPath = library.textures[i].path;
    String destPath = "bsp/" + library.textures[i].name + ".ozcTex";

    struct stat srcInfo;
    struct stat destInfo;

    if( stat( srcPath, &srcInfo ) != 0 ) {
      throw Exception( "Source texture '" + srcPath + "' stat error" );
    }
    if( !forceRebuild && stat( destPath, &destInfo ) == 0 &&
        destInfo.st_mtime > srcInfo.st_mtime )
    {
      continue;
    }

    log.println( "Prebuilding texture '%s' {", srcPath.cstr() );
    log.indent();

    uint id = client::context.loadRawTexture( srcPath );

    hard_assert( id != 0 );

    Buffer buffer( 4 * 1024 * 1024 );
    OutputStream os = buffer.outputStream();

    log.println( "Compiling into '%s'", destPath.cstr() );
    client::context.writeTexture( id, &os );

    int slash = destPath.lastIndex( '/' );
    hard_assert( slash != -1 );
    String dir = destPath.substring( 0, slash );

    File::mkdir( dir );

    if( !buffer.write( destPath, os.length() ) ) {
      throw Exception( "Texture writing failed" );
    }

    log.unindent();
    log.println( "}" );
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildModels()
{
  log.println( "Prebuilding models {" );
  log.indent();

  String dirName = "mdl";
  File dir( dirName );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + dirName + "'" );
  }

  dirName = dirName + "/";

  foreach( file, dirList.citer() ) {
    struct stat srcInfo0;
    struct stat srcInfo1;
    struct stat configInfo;
    struct stat destInfo;

    String name = file->name();
    String path = file->path();

    if( stat( path + "/data.obj", &srcInfo0 ) == 0 ) {
      if( stat( path + "/data.mtl", &srcInfo1 ) != 0 ||
          stat( path + "/config.rc", &configInfo ) != 0 )
      {
        throw Exception( "OBJ model '" + name + "' source files missing" );
      }
      if( !forceRebuild && stat( dirName + name + ".ozcSMM", &destInfo ) == 0 &&
          configInfo.st_mtime < destInfo.st_mtime &&
          srcInfo0.st_mtime < destInfo.st_mtime && srcInfo1.st_mtime < destInfo.st_mtime )
      {
        continue;
      }

      client::OBJ::prebuild( path );
    }
    else if( stat( path + "/tris.md2", &srcInfo0 ) == 0 ) {
      if( stat( path + "/skin.png", &srcInfo1 ) != 0 ||
          stat( path + "/config.rc", &configInfo ) != 0 )
      {
        throw Exception( "MD2 model '" + name + "' source files missing" );
      }
      if( !forceRebuild && ( stat( dirName + name + ".ozcSMM", &destInfo ) == 0 ||
          stat( dirName + name + ".ozcMD2", &destInfo ) == 0 ) &&
          configInfo.st_mtime < destInfo.st_mtime &&
          srcInfo0.st_mtime < destInfo.st_mtime && srcInfo1.st_mtime < destInfo.st_mtime )
      {
        continue;
      }

      client::MD2::prebuild( path );
    }
  }

  log.unindent();
  log.println( "}" );
}

static void compileBSPs()
{
  log.println( "Compiling BSPs {" );
  log.indent();

  String dirName = "data/maps";
  File dir( dirName );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + dirName + "'" );
  }

  dirName = dirName + "/";

  foreach( file, dirList.citer() ) {
    if( !file->hasExtension( "map" ) ) {
      continue;
    }

    const char* dot = String::findLast( file->baseName(), '.' );

    if( dot != null && String::equals( dot + 1, "autosave" ) ) {
      continue;
    }

    String srcPath = file->path();
    String destPath = dirName + file->baseName() + ".bsp";

    struct stat srcInfo;
    struct stat destInfo;

    if( stat( srcPath, &srcInfo ) != 0 ) {
      throw Exception( "Source map stat error" );
    }
    if( !forceRebuild && stat( destPath, &destInfo ) == 0 &&
        destInfo.st_mtime > srcInfo.st_mtime )
    {
      continue;
    }

    String cmdLine = "q3map2 -fs_basepath . -fs_game data " + String( file->path() );

    log.println( "%s", cmdLine.cstr() );
    log.println();
    log.println( "========== q3map2 OUTPUT BEGIN %s ==========", file->baseName().cstr() );
    log.println();
    if( system( cmdLine ) != 0 ) {
      throw Exception( "BSP map compilation failed" );
    }
    log.println();
    log.println( "========== q3map2 OUTPUT END %s ==========", file->baseName().cstr() );
    log.println();
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildBSPs()
{
  log.println( "Prebuilding BSPs {" );
  log.indent();

  String srcDir = "data/maps";
  String destDir = "bsp";
  File dir( srcDir );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + srcDir + "'" );
  }

  srcDir = srcDir + "/";
  destDir = destDir + "/";

  foreach( file, dirList.citer() ) {
    if( !file->hasExtension( "rc" ) ) {
      continue;
    }

    String srcPath0 = file->path();
    String srcPath1 = srcDir + file->baseName() + ".bsp";
    String destPath0 = destDir + file->baseName() + ".ozBSP";
    String destPath1 = destDir + file->baseName() + ".ozcBSP";

    struct stat srcInfo0;
    struct stat srcInfo1;
    struct stat destInfo0;
    struct stat destInfo1;

    if( stat( srcPath0, &srcInfo0 ) != 0 || stat( srcPath1, &srcInfo1 ) != 0 ) {
      throw Exception( "Source BSP stat error" );
    }
    if( !forceRebuild && stat( destPath0, &destInfo0 ) == 0 && stat( destPath1, &destInfo1 ) == 0 &&
        destInfo0.st_mtime > srcInfo0.st_mtime && destInfo0.st_mtime > srcInfo1.st_mtime &&
        destInfo1.st_mtime >= destInfo0.st_mtime )
    {
      continue;
    }

    String name = file->baseName();

    QBSP::prebuild( name );
    client::BSP::prebuild( name );
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildTerras()
{
  log.println( "Prebuilding Terras {" );
  log.indent();

  String srcDir = "terra";
  File dir( srcDir );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + srcDir + "'" );
  }

  srcDir = srcDir + "/";

  foreach( file, dirList.citer() ) {
    if( !file->hasExtension( "rc" ) ) {
      continue;
    }

    String srcPath = file->path();
    String destPath0 = srcDir + file->baseName() + ".ozTerra";
    String destPath1 = srcDir + file->baseName() + ".ozcTerra";

    struct stat srcInfo;
    struct stat destInfo0;
    struct stat destInfo1;

    if( stat( srcPath, &srcInfo ) != 0 ) {
      throw Exception( "Terra .rc stat error" );
    }
    if( !forceRebuild && stat( destPath0, &destInfo0 ) == 0 && stat( destPath1, &destInfo1 ) == 0 &&
        destInfo0.st_mtime > srcInfo.st_mtime && destInfo1.st_mtime >= destInfo0.st_mtime )
    {
      continue;
    }

    String name = file->baseName();

    orbis.terra.prebuild( name );
    client::terra.prebuild( name );
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildCaela()
{
  log.println( "Prebuilding Caela {" );
  log.indent();

  String srcDir = "caelum";
  File dir( srcDir );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + srcDir + "'" );
  }

  srcDir = srcDir + "/";

  foreach( file, dirList.citer() ) {
    if( !file->hasExtension( "rc" ) ) {
      continue;
    }

    String srcPath = file->path();
    String destPath = srcDir + file->baseName() + ".ozcCaelum";

    struct stat srcInfo;
    struct stat destInfo;

    if( stat( srcPath, &srcInfo ) != 0 ) {
      throw Exception( "Caelum .rc stat error" );
    }
    if( !forceRebuild && stat( destPath, &destInfo ) == 0 &&
        destInfo.st_mtime > srcInfo.st_mtime )
    {
      continue;
    }

    String name = file->baseName();

    client::caelum.prebuild( name );
  }

  log.unindent();
  log.println( "}" );
}

static void prebuildModules()
{
  log.println( "Prebuilding Modules {" );
  log.indent();

  Vector<client::Module*> modules;
  client::Module::listModules( &modules );

  for( int i = 0; i < modules.length(); ++i ) {
    modules[i]->prebuild();
  }

  log.unindent();
  log.println( "}" );
}

static void checkLua( const char* path )
{
  log.println( "Checking Lua scripts '%s' {", path );
  log.indent();

  String srcDir = path + String( "/" );
  File dir( path );
  DArray<File> dirList;

  if( !dir.ls( &dirList ) ) {
    throw Exception( "Cannot open directory '" + srcDir + "'" );
  }

  String sources;

  foreach( file, dirList.citer() ) {
    if( !file->hasExtension( "lua" ) ) {
      continue;
    }

    sources = sources + " " + file->path();
  }

  log.println( "luac -p%s", sources.cstr() );
  if( system( "luac -p" + sources ) != 0 ) {
    throw Exception( "Lua syntax check failed" );
  }

  log.unindent();
  log.println( "}" );
}

int main( int argc, char** argv )
{
  System::catchSignals();
//   System::enableHalt( true );

  Alloc::isLocked = false;

  int exitCode = 0;

  printf( "OpenZone  Copyright (C) 2002-2011  Davorin Učakar\n"
      "This program comes with ABSOLUTELY NO WARRANTY.\n"
      "This is free software, and you are welcome to redistribute it\n"
      "under certain conditions; See COPYING file for details.\n\n" );

  try {
    for( int i = 1; i < argc; ++i ) {
      if( String::equals( argv[i], "--help" ) ) {
        printUsage();
        return -1;
      }
      else if( String::equals( argv[i], "--force" ) || String::equals( argv[i], "-f" ) ) {
        forceRebuild = true;
      }
      else if( String::equals( argv[i], "--use-S3TC" ) || String::equals( argv[i], "-C" ) ) {
        client::context.useS3TC = true;
      }
      else if( argv[i][0] != '-' && !config.contains( "dir.prefix" ) ) {
        config.add( "dir.prefix", argv[i] );
      }
      else {
        log.println( "Invalid command-line option '%s'", argv[i] );
        log.println();
        printUsage();
        return -1;
      }
    }
    if( !config.contains( "dir.prefix" ) ) {
      log.println( "Missing data directory as parameter" );
      log.println();
      printUsage();
      return -1;
    }

    log.printlnETD( OZ_APPLICATION_TITLE " Prebuild started at" );

    log.println( "Build details {" );
    log.indent();

    log.println( "Date:            %s", Build::TIME );
    log.println( "Host system:     %s", Build::HOST_SYSTEM );
    log.println( "Target system:   %s", Build::TARGET_SYSTEM );
    log.println( "Build type:      %s", Build::BUILD_TYPE );
    log.println( "Compiler:        %s", Build::COMPILER );
    log.println( "Compiler flags:  %s", Build::CXX_FLAGS );
    log.println( "Linker flags:    %s", Build::EXE_LINKER_FLAGS );

    log.unindent();
    log.println( "}" );

    String prefixDir = config.get( "dir.prefix", OZ_INSTALL_PREFIX );
    String dataDir   = prefixDir + "/share/" OZ_APPLICATION_NAME;

    log.print( "Setting working directory to data directory '%s' ...", dataDir.cstr() );
    if( chdir( dataDir ) != 0 ) {
      log.printEnd( " Failed" );
      return -1;
    }
    log.printEnd( " OK" );

    SDL_Init( SDL_INIT_VIDEO );

    uint startTime = SDL_GetTicks();

    library.init();

    config.add( "screen.width", "400" );
    config.add( "screen.height", "40" );
    config.add( "screen.bpp", "32" );
    config.add( "screen.full", "false" );
    client::render.init();
    SDL_WM_SetCaption( OZ_APPLICATION_TITLE " :: Prebuilding data ...", null );

    if( !config.get( "context.enableS3TC", false ) && client::context.useS3TC ) {
      throw Exception( "S3 texture compression enable but not supported" );
    }

    createDirs();

    compileBSPs();

    client::ui::Mouse::prebuild();

    prebuildTextures( "ui/icon", "ui/icon", true, GL_LINEAR, GL_LINEAR );
    prebuildTextures( "ui/galileo", "ui/galileo", true, GL_LINEAR, GL_LINEAR );

    prebuildTerras();
    prebuildCaela();

    prebuildBSPs();
    prebuildBSPTextures();

    prebuildModels();

    prebuildModules();

    checkLua( "lua/matrix" );
    checkLua( "lua/nirvana" );
    checkLua( "lua/mission" );

    uint endTime = SDL_GetTicks();

    log.println( "Build time: %.2f s", float( endTime - startTime ) / 1000.0f );

    SDL_Quit();
  }
  catch( const Exception& e ) {
    log.resetIndent();
    log.println();
    log.printException( e );
    log.println();

    if( log.isFile() ) {
      fprintf( stderr, "\nEXCEPTION: %s\n", e.what() );
      fprintf( stderr, "  in %s\n\n", e.function );
      fprintf( stderr, "  at %s:%d\n\n", e.file, e.line );
    }

    exitCode = -1;
  }
  catch( const std::exception& e ) {
    log.resetIndent();
    log.println();
    log.println( "EXCEPTION: %s", e.what() );
    log.println();

    if( log.isFile() ) {
      fprintf( stderr, "\nEXCEPTION: %s\n\n", e.what() );
    }

    exitCode = -1;
  }

  client::compiler.free();
  client::render.free();
  library.free();
  config.clear();

  Alloc::printStatistics();
  log.printlnETD( OZ_APPLICATION_TITLE " Prebuild finished at" );

//   Alloc::isLocked = true;
//   Alloc::printLeaks();
  return exitCode;
}
