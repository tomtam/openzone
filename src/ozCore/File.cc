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
 * @file ozCore/File.cc
 */

#include "File.hh"

#if defined( __native_client__ )
# include <ppapi/c/pp_file_info.h>
# include <ppapi/c/ppb_file_io.h>
# include <ppapi/cpp/completion_callback.h>
# include <ppapi/cpp/core.h>
# include <ppapi/cpp/file_io.h>
# include <ppapi/cpp/file_ref.h>
# include <ppapi/cpp/file_system.h>
# include <pthread.h>
#elif defined( _WIN32 )
# include <windows.h>
#else
# include <dirent.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <unistd.h>
#endif

#include <physfs.h>

#if PHYSFS_VER_MAJOR < 2
# error PhysicsFS version must be at least 2.0.
#elif PHYSFS_VER_MAJOR == 2 && PHYSFS_VER_MINOR == 0
# define PHYSFS_readBytes( handle, buffer, len )  PHYSFS_read( handle, buffer, 1, uint( len ) )
# define PHYSFS_writeBytes( handle, buffer, len ) PHYSFS_write( handle, buffer, 1, uint( len ) )
#endif

#ifdef __native_client__
extern "C"
int PHYSFS_NACL_init( PP_Instance instance, PPB_GetInterface getInterface,
                      PP_FileSystemType fileSystemType, PHYSFS_sint64 size );
#endif

namespace oz
{

#ifdef __native_client__
static pp::Core*      ppCore = nullptr;
static pp::FileSystem ppFileSystem;
#endif

static bool operator < ( const File& a, const File& b )
{
  return String::compare( a.path(), b.path() ) < 0;
}

OZ_HIDDEN
File::File( const String& path, File::Type type, int size, long64 time ) :
  filePath( path ), fileType( type ), fileSize( size ), fileTime( time ), data( nullptr )
{}

File::File( const char* path ) :
  filePath( path ), fileType( MISSING ), fileSize( -1 ), fileTime( 0 ), data( nullptr )
{
  // Avoid stat'ing obviously non-existent files.
  if( filePath.isEmpty() ) {
    return;
  }

  stat();
}

File::File( const String& path ) :
  filePath( path ), fileType( MISSING ), fileSize( -1 ), fileTime( 0 ), data( nullptr )
{
  // Avoid stat'ing obviously non-existent files.
  if( filePath.isEmpty() ) {
    return;
  }

  stat();
}

File::~File()
{
  unmap();
}

File::File( const File& file ) :
  filePath( file.filePath ), fileType( file.fileType ), fileSize( file.fileSize ),
  fileTime( file.fileTime ), data( nullptr )
{}

File::File( File&& file ) :
  filePath( static_cast<String&&>( file.filePath ) ), fileType( file.fileType ),
  fileSize( file.fileSize ), fileTime( file.fileTime ), data( file.data )
{
  file.filePath = "";
  file.fileType = MISSING;
  file.fileSize = -1;
  file.fileTime = 0;
  file.data     = nullptr;
}

File& File::operator = ( const File& file )
{
  if( &file == this ) {
    return *this;
  }

  unmap();

  filePath = file.filePath;
  fileType = file.fileType;
  fileSize = file.fileSize;
  fileTime = file.fileTime;

  return *this;
}

File& File::operator = ( File&& file )
{
  if( &file == this ) {
    return *this;
  }

  unmap();

  filePath = static_cast<String&&>( file.filePath );
  fileType = file.fileType;
  fileSize = file.fileSize;
  fileTime = file.fileTime;
  data     = file.data;

  file.filePath = "";
  file.fileType = MISSING;
  file.fileSize = -1;
  file.fileTime = 0;
  file.data     = nullptr;

  return *this;
}

bool File::stat()
{
  if( filePath.fileIsVirtual() ) {
#if PHYSFS_VER_MAJOR == 2 && PHYSFS_VER_MINOR == 0

    if( !PHYSFS_exists( &filePath[1] ) ) {
      fileType = MISSING;
      fileSize = -1;
      fileTime = 0;
    }
    else if( PHYSFS_isDirectory( &filePath[1] ) ) {
      fileType = DIRECTORY;
      fileSize = -1;
      fileTime = PHYSFS_getLastModTime( &filePath[1] );
    }
    else {
      PHYSFS_File* file = PHYSFS_openRead( &filePath[1] );

      if( file == nullptr ) {
        fileType = MISSING;
        fileSize = -1;
        fileTime = 0;
      }
      else {
        fileType = REGULAR;
        fileSize = int( PHYSFS_fileLength( file ) );
        fileTime = PHYSFS_getLastModTime( &filePath[1] );

        PHYSFS_close( file );
      }
    }

#else

    PHYSFS_Stat info;

    if( !PHYSFS_stat( &filePath[1], &info ) ) {
      fileType = MISSING;
      fileSize = -1;
      fileTime = 0;
    }
    else if( info.filetype == PHYSFS_FILETYPE_DIRECTORY ) {
      fileType = DIRECTORY;
      fileSize = -1;
      fileTime = max( info.createtime, info.modtime );
    }
    else if( info.filetype == PHYSFS_FILETYPE_REGULAR ) {
      fileType = REGULAR;
      fileSize = int( info.filesize );
      fileTime = max( info.createtime, info.modtime );
    }
    else {
      fileType = MISSING;
      fileSize = -1;
      fileTime = 0;
    }

#endif
  }
  else {
    fileType = MISSING;
    fileSize = -1;
    fileTime = 0;

#if defined( __native_client__ )

    if( String::equals( filePath, "/" ) ) {
      fileType = DIRECTORY;
      return;
    }

    pp::FileRef file( ppFileSystem, filePath );
    pp::FileIO  fio( System::instance );
    PP_FileInfo info;

    if( fio.Open( file, 0, pp::BlockUntilComplete() ) == PP_OK &&
        fio.Query( &info, pp::BlockUntilComplete() ) == PP_OK )
    {
      if( info.type == PP_FILETYPE_REGULAR ) {
        fileType = REGULAR;
        fileSize = int( info.size );
        fileTime = long64( max( info.creation_time, info.last_modified_time ) );
      }
      else if( info.type == PP_FILETYPE_DIRECTORY ) {
        fileType = DIRECTORY;
        fileSize = -1;
        fileTime = long64( max( info.creation_time, info.last_modified_time ) );
      }
    }

#elif defined( _WIN32 )

    WIN32_FILE_ATTRIBUTE_DATA info;
    if( GetFileAttributesEx( filePath, GetFileExInfoStandard, &info ) == 0 ) {
      return;
    }

    ULARGE_INTEGER creationTime = {
      { info.ftCreationTime.dwLowDateTime, info.ftCreationTime.dwHighDateTime }
    };
    ULARGE_INTEGER modificationTime = {
      { info.ftLastWriteTime.dwLowDateTime, info.ftLastWriteTime.dwHighDateTime }
    };

    long64 time = max( creationTime.QuadPart, modificationTime.QuadPart ) / 10000;

    if( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
      fileType = DIRECTORY;
      fileSize = -1;
      fileTime = time;
    }
    else {
      HANDLE handle = CreateFile( filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, nullptr );
      if( handle != nullptr ) {
        int size = int( GetFileSize( handle, nullptr ) );

        if( size != int( INVALID_FILE_SIZE ) ) {
          fileType = REGULAR;
          fileSize = size;
          fileTime = time;
        }

        CloseHandle( handle );
      }
    }

#else

    struct stat info;

    if( ::stat( filePath, &info ) == 0 ) {
      if( S_ISREG( info.st_mode ) ) {
        fileType = REGULAR;
        fileSize = int( info.st_size );
        fileTime = long64( max( info.st_ctime, info.st_mtime ) );
      }
      else if( S_ISDIR( info.st_mode ) ) {
        fileType = DIRECTORY;
        fileSize = -1;
        fileTime = long64( max( info.st_ctime, info.st_mtime ) );
      }
    }

#endif
  }

  return fileType != MISSING;
}

String File::realDirectory() const
{
  if( filePath.fileIsVirtual() ) {
    const char* realDir = PHYSFS_getRealDir( &filePath[1] );
    return realDir == nullptr ? "" : realDir;
  }
  else {
    return "";
  }
}

String File::realPath() const
{
  if( filePath.fileIsVirtual() ) {
    const char* realDir = PHYSFS_getRealDir( &filePath[1] );
    realDir = realDir == nullptr ? "" : realDir;

    return String::str( String::last( realDir ) == '/' ? "%s%s" : "%s/%s", realDir, &filePath[1] );
  }
  else {
    return filePath;
  }
}

bool File::read( char* buffer, int* size ) const
{
  if( fileSize <= 0 ) {
    *size = 0;
    return fileSize == 0;
  }

  if( filePath.fileIsVirtual() ) {
    PHYSFS_File* file = PHYSFS_openRead( &filePath[1] );
    if( file == nullptr ) {
      *size = 0;
      return false;
    }

    int result = int( PHYSFS_readBytes( file, buffer, ulong64( *size ) ) );
    PHYSFS_close( file );

    *size = result;
    return true;
  }
  else if( data != nullptr ) {
    *size = min( *size, fileSize );
    mCopy( buffer, data, size_t( *size ) );
    return true;
  }
  else {
#if defined( __native_client__ )

    pp::FileRef file( ppFileSystem, filePath );
    pp::FileIO  fio( System::instance );

    if( fio.Open( file, PP_FILEOPENFLAG_READ, pp::BlockUntilComplete() ) != PP_OK ) {
      *size = 0;
      return false;
    }

    int read = 0;
    int result;
    while( ( result = fio.Read( read, &buffer[read], *size - read,
                                pp::BlockUntilComplete() ) ) > 0 )
    {
      read += result;
    }

    if( result < 0 || read != *size ) {
      *size = read;
      return false;
    }
    return true;

#elif defined( _WIN32 )

    HANDLE file = CreateFile( filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr );
    if( file == nullptr ) {
      *size = 0;
      return false;
    }

    DWORD read = 0;
    BOOL result = ReadFile( file, buffer, DWORD( size ), &read, nullptr );
    CloseHandle( file );

    if( !result || int( read ) != *size ) {
      *size = int( read );
      return false;
    }
    return true;

#else

    int fd = open( filePath, O_RDONLY );
    if( fd < 0 ) {
      *size = 0;
      return false;
    }

    int read = int( ::read( fd, buffer, size_t( *size ) ) );
    close( fd );

    if( read != *size ) {
      *size = max( 0, read );
      return false;
    }
    return true;

#endif
  }
}

bool File::read( OutputStream* ostream ) const
{
  int   size   = fileSize;
  char* buffer = ostream->forward( fileSize );
  bool  result = read( buffer, &size );

  ostream->set( buffer + size );
  return result;
}

Buffer File::read() const
{
  Buffer buffer;

  if( fileSize <= 0 ) {
    return buffer;
  }

  int size = fileSize;
  buffer.allocate( size );

  read( buffer.begin(), &size );

  if( size != fileSize ) {
    buffer.resize( size );
  }
  return buffer;
}

String File::readString() const
{
  char*  buffer;
  int    size = fileSize;
  String s    = String::create( size, &buffer );

  read( buffer, &size );
  buffer[size] = '\0';
  return s;
}

bool File::write( const char* buffer, int size ) const
{
  if( filePath.fileIsVirtual() ) {
    PHYSFS_File* file = PHYSFS_openWrite( &filePath[1] );
    if( file == nullptr ) {
      return false;
    }

    int result = int( PHYSFS_writeBytes( file, data, ulong64( size ) ) );
    PHYSFS_close( file );

    return result == size;
  }
  else if( data != nullptr ) {
    return false;
  }
  else {
#if defined( __native_client__ )

    pp::FileRef file( ppFileSystem, filePath );
    pp::FileIO  fio( System::instance );

    if( fio.Open( file, PP_FILEOPENFLAG_WRITE | PP_FILEOPENFLAG_CREATE | PP_FILEOPENFLAG_TRUNCATE,
                  pp::BlockUntilComplete() ) != PP_OK )
    {
      return false;
    }

    int written = 0;
    int result;
    while( written != size &&
          ( result = fio.Write( written, &buffer[written], size - written,
                                pp::BlockUntilComplete() ) ) > 0 )
    {
      written += result;
    }

    return written == size;

#elif defined( _WIN32 )

    HANDLE file = CreateFile( filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr );
    if( file == nullptr ) {
      return false;
    }

    DWORD written;
    BOOL result = WriteFile( file, buffer, DWORD( size ), &written, nullptr );
    CloseHandle( file );

    return result && int( written ) == size;

#else

    int fd = open( filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
    if( fd < 0 ) {
      return false;
    }

    int result = int( ::write( fd, buffer, size_t( size ) ) );
    close( fd );

    return result == size;

#endif
  }
}

bool File::write( const Buffer& buffer ) const
{
  return write( buffer.begin(), buffer.length() );
}

bool File::writeString( const String& s ) const
{
  return write( s.cstr(), s.length() );
}

bool File::map()
{
  if( fileSize < 0 ) {
    return false;
  }
  if( data != nullptr ) {
    return true;
  }

  if( filePath.fileIsVirtual() ) {
    int size = fileSize;

    data = new char[size];
    return read( data, &size );
  }
  else {
#if defined( __native_client__ )

    // If we used `data` member here `read()` would copy it into itself.
    char* buffer = new char[size];

    if( !read( filePath, buffer, &size ) ) {
      delete[] buffer;
      return nullptr;
    }

    data = buffer;
    return true;

#elif defined( _WIN32 )

    HANDLE file = CreateFile( filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr );
    if( file == nullptr ) {
      return nullptr;
    }

    HANDLE mapping = CreateFileMapping( file, nullptr, PAGE_READONLY, 0, 0, nullptr );
    if( mapping == nullptr ) {
      CloseHandle( file );
      return nullptr;
    }

    data = static_cast<char*>( MapViewOfFile( mapping, FILE_MAP_READ, 0, 0, 0 ) );

    CloseHandle( mapping );
    CloseHandle( file );

    return data != nullptr;

#else

    int fd = open( filePath, O_RDONLY );
    if( fd < 0 ) {
      return nullptr;
    }

    data = static_cast<char*>( mmap( nullptr, size_t( fileSize ), PROT_READ, MAP_SHARED, fd, 0 ) );
    data = data == MAP_FAILED ? nullptr : data;

    close( fd );
    return data != nullptr;

#endif
  }
}

void File::unmap()
{
  if( data == nullptr ) {
    return;
  }

  if( filePath.fileIsVirtual() ) {
    delete[] data;
  }
  else {
#if defined( __native_client__ )
    delete[] data;
#elif defined( _WIN32 )
    UnmapViewOfFile( data );
#else
    munmap( data, size_t( fileSize ) );
#endif
  }
  data = nullptr;
}

InputStream File::inputStream( Endian::Order order ) const
{
  hard_assert( data != nullptr );

  return InputStream( data, data + fileSize, order );
}

DArray<File> File::ls() const
{
  DArray<File> array;

  if( fileType != DIRECTORY ) {
    return array;
  }

  if( filePath.fileIsVirtual() ) {
    char** list = PHYSFS_enumerateFiles( &filePath[1] );
    if( list == nullptr ) {
      return array;
    }

    // Count entries first.
    int count = 0;
    char** entity = list;
    while( *entity != nullptr ) {
      if( ( *entity )[0] != '.' ) {
        ++count;
      }
      ++entity;
    }

    if( count == 0 ) {
      PHYSFS_freeList( list );
      return array;
    }

    array.resize( count );

    String prefix = filePath[1] == '\0' ? filePath : filePath + "/";

    entity = list;
    for( int i = 0; i < count; ++entity ) {
      if( ( *entity )[0] != '.' ) {
        array[i] = File( prefix + *entity );
        ++i;
      }
    }

    PHYSFS_freeList( list );
  }
  else {
#if defined( __native_client__ )

    // TODO: Implement when pp::DirectoryReader gets into stable PPAPI.

#elif defined( _WIN32 )

    WIN32_FIND_DATA entity;

    HANDLE dir = FindFirstFile( filePath + "\\*.*", &entity );
    if( dir == nullptr ) {
      return array;
    }

    // Count entries first.
    int count = 0;
    while( FindNextFile( dir, &entity ) ) {
      if( entity.cFileName[0] != '.' ) {
        ++count;
      }
    }

    CloseHandle( dir );

    if( count == 0 ) {
      return array;
    }

    dir = FindFirstFile( filePath + "\\*.*", &entity );
    if( dir == nullptr ) {
      return array;
    }

    array.resize( count );

    int i = 0;

    if( entity.cFileName[i] != '.' ) {
      array[i].filePath = filePath + "/" + entity.cFileName;
      ++i;
    }

    while( i < count ) {
      if( FindNextFile( dir, &entity ) == 0 ) {
        CloseHandle( dir );
        array.clear();
        return array;
      }

      if( entity.cFileName[0] != '.' ) {
        array[i] = File( filePath + "/" + entity.cFileName );
        ++i;
      }
    }

    CloseHandle( dir );

#else

    DIR* directory = opendir( filePath );
    if( directory == nullptr ) {
      return array;
    }

    struct dirent* entity = readdir( directory );

    // Count entries first.
    int count = 0;
    while( entity != nullptr ) {
      if( entity->d_name[0] != '.' ) {
        ++count;
      }
      entity = readdir( directory );
    }

    if( count == 0 ) {
      closedir( directory );
      return array;
    }

    rewinddir( directory );

    array.resize( count );

    String prefix = filePath.isEmpty() || filePath.equals( "/" ) ? "" : filePath + "/";

    for( int i = 0; i < count; ) {
      entity = readdir( directory );

      if( entity == nullptr ) {
        closedir( directory );
        array.clear();
        return array;
      }

      if( entity->d_name[0] != '.' ) {
        array[i] = File( prefix + entity->d_name );
        ++i;
      }
    }

    closedir( directory );

#endif
  }

  array.sort();
  return array;
}

String File::cwd()
{
#if defined( __native_client__ )

  return "";

#elif defined( _WIN32 )

  char buffer[256];
  bool hasFailed = GetCurrentDirectory( 256, buffer ) == 0;
  return hasFailed ? "" : buffer;

#else

  char buffer[256];
  bool hasFailed = getcwd( buffer, 256 ) == nullptr;
  return hasFailed ? "" : buffer;

#endif
}

bool File::chdir( const char* path )
{
#if defined( __native_client__ )
  static_cast<void>( path );
  return false;
#elif defined( _WIN32 )
  return SetCurrentDirectory( path ) != 0;
#else
  return ::chdir( path ) == 0;
#endif
}

bool File::mkdir( const char* path )
{
  if( String::fileIsVirtual( path ) ) {
    return PHYSFS_mkdir( &path[1] ) != 0;
  }
  else {
#if defined( __native_client__ )
    pp::FileRef file( ppFileSystem, path );
    return file.MakeDirectory( pp::BlockUntilComplete() ) == PP_OK;
#elif defined( _WIN32 )
    return CreateDirectory( path, nullptr ) != 0;
#else
    return ::mkdir( path, 0755 ) == 0;
#endif
  }
}

bool File::rm( const char* path )
{
  if( String::fileIsVirtual( path ) ) {
    return PHYSFS_delete( &path[1] );
  }
  else {
#if defined( __native_client__ )

    pp::FileRef file( ppFileSystem, path );
    return file.Delete( pp::BlockUntilComplete() ) == PP_OK;

#elif defined( _WIN32 )

    if( File( path ).fileType == DIRECTORY ) {
      return RemoveDirectory( path ) != 0;
    }
    else {
      return DeleteFile( path ) != 0;
    }

#else

    if( File( path ).fileType == DIRECTORY ) {
      return rmdir( path ) == 0;
    }
    else {
      return unlink( path ) == 0;
    }

#endif
  }
}

bool File::mount( const char* path, const char* mountPoint, bool append )
{
  return PHYSFS_mount( path, mountPoint, append ) != 0;
}

bool File::mountLocal( const char* path )
{
  if( PHYSFS_setWriteDir( path ) == 0 ) {
    return false;
  }
  if( PHYSFS_mount( path, nullptr, false ) == 0 ) {
    PHYSFS_setWriteDir( nullptr );
    return false;
  }
  return true;
}

void File::init( NaClFileSystem naclFileSystem, int naclSize )
{
  static_cast<void>( naclFileSystem );
  static_cast<void>( naclSize );

#ifdef __native_client__

  ppCore = pp::Module::Get()->core();

  PP_FileSystemType naclType = naclFileSystem == PERSISTENT ? PP_FILESYSTEMTYPE_LOCALPERSISTENT :
                                                              PP_FILESYSTEMTYPE_LOCALTEMPORARY;

  ppFileSystem = pp::FileSystem( System::instance, naclType );

  if( ppFileSystem.Open( naclSize, pp::BlockUntilComplete() ) != PP_OK ) {
    OZ_ERROR( "Local NaCl file system open failed" );
  }
  if( System::instance == nullptr ) {
    OZ_ERROR( "System::instance must be set prior to NaCl file system initialisation" );
  }
  if( pp::Module::Get()->IsMainThread() ) {
    OZ_ERROR( "PhysicsFS cannot be initialised from the main NaCl thread" );
  }

  PHYSFS_NACL_init( System::instance->pp_instance(), pp::Module::Get()->get_browser_interface(),
                    naclType, naclSize );

#endif

  if( PHYSFS_init( nullptr ) == 0 ) {
    OZ_ERROR( "PhysicsFS initialisation failed: %s", PHYSFS_getLastError() );
  }
}

void File::destroy()
{
  PHYSFS_deinit();
}

}
