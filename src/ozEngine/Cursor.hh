/*
 * ozEngine - OpenZone Engine Library.
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
 * @file ozEngine/Cursor.hh
 *
 * `Cursor` class.
 */

#pragma once

#include "common.hh"

namespace oz
{

/**
 * X11 mouse cursor loader.
 *
 * It all images for an animated cursor have the same size, the same hotspot and the same animation
 * delay.
 */
class Cursor
{
  private:

    /// Maximum number of images.
    static const int MAX_IMAGES = 16;

    /**
     * Cursor image.
     *
     * For animated cursors there may are multiple images, one for each animation frame.
     */
    struct Image
    {
      int  width;       ///< Image width.
      int  height;      ///< Image height.
      int  hotspotLeft; ///< Hotspot offset from the left.
      int  hotspotTop;  ///< Hotspot offset from the top.
      int  delay;       ///< Frame time in milliseconds.
      uint textureId;   ///< GL texture id.
    };

    Image images[MAX_IMAGES]; ///< Cursor images.
    int   nImages;            ///< Number of images.
    int   frame;              ///< Current animation frame.
    int   frameTime;          ///< Time in milliseconds of the current animation frame.

  public:

    /**
     * Create an empty instance.
     */
    explicit Cursor();

    /**
     * Create from file.
     */
    explicit Cursor( const File& file, int size = -1 );

    /**
     * Destructor, destroys textures if loaded.
     */
    ~Cursor();

    /**
     * Move constructor.
     */
    Cursor( Cursor&& c );

    /**
     * Move operator.
     */
    Cursor& operator = ( Cursor&& c );

    /**
     * True iff loaded.
     */
    bool isLoaded() const
    {
      return nImages != 0;
    }

    /**
     * Hotspot offset from the left.
     */
    int hotspotLeft() const
    {
      return images[frame].hotspotLeft;
    }

    /**
     * Hotspot offset from the top.
     */
    int hotspotTop() const
    {
      return images[frame].hotspotTop;
    }

    /**
     * Cursor image width.
     */
    int width() const
    {
      return images[frame].width;
    }

    /**
     * Cursor image height.
     */
    int height() const
    {
      return images[frame].height;
    }

    /**
     * GL texture id for the current frame.
     */
    uint textureId() const
    {
      return images[frame].textureId;
    }

    /**
     * Reset animation.
     */
    void reset();

    /**
     * Advance animation.
     */
    void advance( int millis );

    /**
     * Load from file.
     */
    bool load( const File& file, int size = -1 );

    /**
     * Destroy textures if loaded.
     */
    void destroy();

};

}