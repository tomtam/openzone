/*
 * liboz - OpenZone core library.
 *
 * Copyright © 2002-2011 Davorin Učakar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file oz/PhysFile.hh
 */

#pragma once

#include "BufferStream.hh"
#include "Buffer.hh"

namespace oz
{

/**
 * PHYSFS wrapper, similar to <code>File</code> class, but read-only.
 *
 * @ingroup oz
 */
class PhysFile
{
  public:

    /**
     * %File type classification.
     */
    enum Type
    {
      NONE,
      REGULAR,
      DIRECTORY,
      MISSING
    };

  private:

    String filePath; ///< %File path.
    Type   type;     ///< Cached file type.
    char*  data;     ///< Mapped memory.
    int    size;     ///< Mapped memory size.

  public:

    /**
     * Create an empty instance.
     */
    PhysFile();

    /**
     * Destructor.
     */
    ~PhysFile();

    /**
     * No copying.
     */
    PhysFile( const PhysFile& ) = delete;

    /**
     * Move constructor, transfers mapped region "ownership".
     */
    PhysFile( PhysFile&& file );

    /**
     * No copying.
     */
    PhysFile& operator = ( const PhysFile& ) = delete;

    /**
     * Move operator, transfers mapped region "ownership".
     */
    PhysFile& operator = ( PhysFile&& file );

    /**
     * Create an instance for the given path.
     */
    explicit PhysFile( const char* path );

    /**
     * Set a new file path.
     *
     * Cached file type is cleared to <tt>NONE</tt> and file is unmapped if it is currently mapped.
     */
    void setPath( const char* path );

    /**
     * Stat file to get its type.
     *
     * File type is cached until one changes the file path.
     */
    Type getType();

    /**
     * Virtual file path.
     */
    const String& path() const;

    /**
     * Real file path.
     */
    String realPath() const;

    /**
     * %File name.
     */
    const char* name() const;

    /**
     * Extension (part of base name after the last dot) or <tt>null</tt> if no dot in base name.
     */
    const char* extension() const;

    /**
     * Name without the extension (and the dot).
     */
    String baseName() const;

    /**
     * True iff the extension is equal to the given string.
     *
     * @return True iff extension exists.
     */
    bool hasExtension( const char* ext ) const;

    /**
     * %Map file into memory.
     *
     * One can use <tt>inputStream()</tt> afterwards to read the contents.
     */
    bool map();

    /**
     * Unmap mapped file.
     */
    void unmap();

    /**
     * Get <code>InputStream</code> for currently mapped file.
     */
    InputStream inputStream( Endian::Order order = Endian::NATIVE ) const;

    /**
     * Read file into a buffer.
     */
    Buffer read() const;

    /**
     * Generate a list of files in directory.
     *
     * Hidden files (in Unix means, so everything starting with '.') are skipped.
     * On error, empty array is returned.
     */
    DArray<PhysFile> ls();

};

}
