/*
 * liboz - OpenZone core library.
 *
 * Copyright (C) 2002-2011  Davorin Učakar
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file oz/oz.hh
 *
 * Common include file for liboz.
 *
 * @defgroup oz liboz - OpenZone core library
 */

#pragma once

/*
 * Base definitions and templates.
 */
#include "common.hh"

/*
 * Iterators and utilities for iterable containers and arrays.
 */
#include "iterables.hh"
#include "arrays.hh"

/*
 * Simple containers.
 */
#include "Pair.hh"
#include "List.hh"
#include "DList.hh"
#include "Array.hh"
#include "DArray.hh"
#include "SVector.hh"
#include "Vector.hh"
#include "Map.hh"

/*
 * Memory management.
 */
#include "Alloc.hh"
#include "Pool.hh"

/*
 * String.
 */
#include "String.hh"

/*
 * Advanced containers.
 */
#include "HashIndex.hh"
#include "HashString.hh"

/*
 * Bitset.
 */
#include "Bitset.hh"
#include "SBitset.hh"

/*
 * Math.
 */
#include "Math.hh"
#include "Simd.hh"
#include "Vec4.hh"
#include "Vec3.hh"
#include "Point3.hh"
#include "Plane.hh"
#include "Quat.hh"
#include "Mat44.hh"

/*
 * IO.
 */
#include "Endian.hh"
#include "InputStream.hh"
#include "OutputStream.hh"
#include "BufferStream.hh"
#include "Buffer.hh"
#include "File.hh"

/*
 * Error handling.
 */
#include "StackTrace.hh"
#include "Exception.hh"
#include "System.hh"

/*
 * Utilities.
 */
#include "Log.hh"
#include "Config.hh"
