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

/**
 * @file ozCore/Arrays.hh
 *
 * Iterators and utility functions for arrays and array-like containers.
 */

#pragma once

#include "System.hh"

namespace oz
{

/**
 * Wrapper class for array iterators and utility functions.
 */
class Arrays
{
private:

  /**
   * Array iterator.
   *
   * It should not be used directly but appropriate iterator types should be typedef'd to it.
   */
  template <typename Elem>
  class ArrayIterator : public detail::IteratorBase<Elem>
  {
  protected:

    using detail::IteratorBase<Elem>::elem_;

    Elem* past_ = nullptr; ///< Pointer that points just past the last element.

  public:

    /**
     * Create an invalid iterator.
     */
    ArrayIterator() = default;

    /**
     * Array iterator.
     *
     * @param first first array element.
     * @param past successor of the last element.
     */
    OZ_ALWAYS_INLINE
    explicit ArrayIterator(Elem* first, Elem* past)
      : detail::IteratorBase<Elem>(first), past_(past)
    {}

    /**
     * True as long as iterator has not passed all array elements.
     */
    OZ_ALWAYS_INLINE
    bool isValid() const
    {
      return elem_ != past_;
    }

    /**
     * Advance to the next element.
     */
    OZ_ALWAYS_INLINE
    ArrayIterator& operator++()
    {
      OZ_ASSERT(elem_ != past_);

      ++elem_;
      return *this;
    }

    /**
     * STL-style begin iterator.
     */
    OZ_ALWAYS_INLINE
    Elem* begin() const
    {
      return elem_;
    }

    /**
     * STL-style end iterator.
     */
    OZ_ALWAYS_INLINE
    Elem* end() const
    {
      return past_;
    }

  };

public:

  /**
   * Array iterator with constant access to elements.
   */
  template <typename Elem>
  using CIterator = ArrayIterator<const Elem>;

  /**
   * Array iterator with non-constant access to elements.
   */
  template <typename Elem>
  using Iterator = ArrayIterator<Elem>;

private:

  /**
   * Helper function for `sort()`.
   *
   * @note
   * `Elem` type must have `operator<(const Elem&)` defined.
   *
   * Quicksort algorithm is used which takes last element in a partition as a pivot so sorting a
   * sorted or nearly sorted array will take O(n^2) time instead of O(n log n) as in average case.
   * When a partition has at most 11 elements, selection sort is used.
   *
   * @param first pointer to first element in the array to be sorted.
   * @param last pointer to last element in the array.
   */
  template <typename Elem, class LessFunc = Less<Elem>>
  static void quicksort(Elem* first, Elem* last)
  {
    // 8-14 seem as optimal thresholds for switching to selection sort.
    if (last - first > 11) {
      // Quicksort (the last element is the pivot).
      Elem* top    = first;
      Elem* bottom = last - 1;

      do {
        for (; !LessFunc()(*last, *top) && top <= bottom; ++top);
        for (; LessFunc()(*last, *bottom) && top < bottom; --bottom);

        if (top >= bottom) {
          break;
        }

        swap<Elem>(*top, *bottom);
      }
      while (true);

      if (top != last) {
        swap<Elem>(*top, *last);
        quicksort<Elem, LessFunc>(top + 1, last);
      }
      quicksort<Elem, LessFunc>(first, top - 1);
    }
    else {
      // Selection sort.
      for (Elem* i = first; i < last;) {
        Elem* pivot = i;
        Elem* min   = i;
        ++i;

        for (Elem* j = i; j <= last; ++j) {
          if (LessFunc()(*j, *min)) {
            min = j;
          }
        }
        if (min != pivot) {
          swap<Elem>(*min, *pivot);
        }
      }
    }
  }

public:

  /**
   * Static class.
   */
  Arrays() = delete;

  /**
   * True iff respective elements are equal.
   */
  template <typename Elem>
  static bool equals(const Elem* arrayA, int size, const Elem* arrayB)
  {
    const Elem* endA = arrayA + size;

    while (arrayA < endA) {
      if (!(*arrayA++ == *arrayB++)) {
        return false;
      }
    }
    return true;
  }

  /**
   * Length of a static array.
   */
  template <typename Elem, int SIZE>
  OZ_ALWAYS_INLINE
  static constexpr int size(const Elem (&)[SIZE])
  {
    return SIZE;
  }

  /**
   * Index of the first occurrence of the value or -1 if not found.
   */
  template <typename Elem, typename Value>
  static int index(const Elem* array, int size, const Value& value)
  {
    const Elem* begin = array;
    const Elem* end   = array + size;

    for (; begin < end; ++begin) {
      if (*begin == value) {
        return int(begin - array);
      }
    }
    return -1;
  }

  /**
   * Index of the last occurrence of the value or -1 if not found.
   */
  template <typename Elem, typename Value>
  static int lastIndex(const Elem* array, int size, const Value& value)
  {
    const Elem* end = array + size - 1;

    for (; end >= array; --end) {
      if (*end == value) {
        break;
      }
    }
    return int(end - array);
  }

  /**
   * True iff a given value is found in an array.
   */
  template <typename Elem, typename Value>
  static bool contains(const Elem* array, int size, const Value& value)
  {
    const Elem* end = array + size;

    for (; array < end; ++array) {
      if (*array == value) {
        return true;
      }
    }
    return false;
  }

  /**
   * Copy array elements from the first to the last.
   */
  template <typename Elem>
  static void copy(const Elem* srcArray, int size, Elem* destArray)
  {
    const Elem* srcEnd = srcArray + size;

    while (srcArray < srcEnd) {
      *destArray++ = *srcArray++;
    }
  }

  /**
   * Move array elements from the last to the first.
   */
  template <typename Elem>
  static void copyBackward(const Elem* srcArray, int size, Elem* destArray)
  {
    const Elem* srcEnd  = srcArray + size;
    Elem*       destEnd = destArray + size;

    while (srcEnd > srcArray) {
      *--destEnd = *--srcEnd;
    }
  }

  /**
   * Move array elements from the first to the last.
   */
  template <typename Elem>
  static void move(Elem* srcArray, int size, Elem* destArray)
  {
    const Elem* srcEnd = srcArray + size;

    while (srcArray < srcEnd) {
      *destArray++ = static_cast<Elem&&>(*srcArray++);
    }
  }

  /**
   * Move array elements from the last to the first.
   */
  template <typename Elem>
  static void moveBackward(Elem* srcArray, int size, Elem* destArray)
  {
    Elem* srcEnd  = srcArray + size;
    Elem* destEnd = destArray + size;

    while (srcEnd > srcArray) {
      *--destEnd = static_cast<Elem&&>(*--srcEnd);
    }
  }

  /**
   * Assign array elements to a given value.
   */
  template <typename Elem, typename Value>
  static void fill(Elem* array, int size, const Value& value)
  {
    const Elem* end = array + size;

    while (array < end) {
      *array++ = value;
    }
  }

  /**
   * Assign each array element to the default value.
   */
  template <typename Elem>
  static void clear(Elem* array, int size)
  {
    const Elem* end = array + size;

    while (array < end) {
      *array++ = Elem();
    }
  }

  /**
   * Delete objects referenced by elements and assign all elements to null.
   *
   * Array elements must be pointers.
   */
  template <typename Elem>
  static void free(Elem* array, int size)
  {
    const Elem* end = array + size;

    while (array < end) {
      delete *array;
      *array++ = Elem();
    }
  }

  /**
   * Reverse the order of array elements.
   */
  template <typename Elem>
  static void reverse(Elem* array, int size)
  {
    Elem* bottom = array;
    Elem* top    = array + size - 1;

    while (bottom < top) {
      swap<Elem>(*bottom++, *top--);
    }
  }

  /**
   * Sort array using `detail::quicksort()`.
   */
  template <typename Elem, class LessFunc = Less<Elem>>
  static void sort(Elem* array, int size)
  {
    int last = size - 1;

    if (last > 0) {
      quicksort<Elem, LessFunc>(array, &array[last]);
    }
  }

  /**
   * Find index in a sorted array such that `array[index - 1] < key && key <= array[index]`.
   *
   * @note
   * `Elem` type must have `bool operator<(const Elem&, const Key&) const` defined.
   *
   * If all elements are lesser return `count` and if all elements are greater return 0.
   *
   * @param array array of elements.
   * @param size number of elements.
   * @param key the key we are looking for.
   * @return Index of the first element >= `key`, `count` otherwise.
   */
  template <typename Elem, typename Key, class LessFunc = Less<Elem>>
  static int bisection(const Elem* array, int size, const Key& key)
  {
    int a = -1;
    int b = size;

    // The algorithm ensures that (a == -1 or array[a] <= key) and (b == count or key < array[b]),
    // so the key may only lie on position a or nowhere.
    while (b - a > 1) {
      int c = (a + b) / 2;

      if (LessFunc()(array[c], key)) {
        a = c;
      }
      else {
        b = c;
      }
    }
    return b;
  }

  /**
   * Reallocate an array to a different size, moving its elements.
   *
   * Allocate a new zero-initialised array of `newCount` elements, move first `min(count, newCount)`
   * elements of the source array to the newly created one and free the source array. Similar to
   * `realloc()`, the given array is deleted and `nullptr` is returned if `newCount` is 0.
   *
   * @return Newly allocated array.
   */
  template <typename Elem>
  static Elem* reallocate(Elem* array, int size, int newCount)
  {
    Elem* newArray = nullptr;

    if (newCount != 0) {
      newArray = new Elem[newCount] {};
      move<Elem>(array, min<int>(size, newCount), newArray);
    }
    delete[] array;

    return newArray;
  }

};

/**
 * Create array iterator with constant element access.
 */
template <typename Elem>
OZ_ALWAYS_INLINE
inline Arrays::CIterator<Elem> citerator(const Elem* array, int size)
{
  return Arrays::CIterator<Elem>(array, array + size);
}

/**
 * Create array iterator with non-constant element access.
 */
template <typename Elem>
OZ_ALWAYS_INLINE
inline Arrays::Iterator<Elem> iterator(Elem* array, int size)
{
  return Arrays::Iterator<Elem>(array, array + size);
}

/**
 * Create static array iterator with element constant access.
 */
template <typename Elem, int SIZE>
OZ_ALWAYS_INLINE
inline Arrays::CIterator<Elem> citerator(const Elem (& array)[SIZE])
{
  return Arrays::CIterator<Elem>(array, array + SIZE);
}

/**
 * Create static array iterator with non-constant element access.
 */
template <typename Elem, int SIZE>
OZ_ALWAYS_INLINE
inline Arrays::Iterator<Elem> iterator(Elem (& array)[SIZE])
{
  return Arrays::Iterator<Elem>(array, array + SIZE);
}

/**
 * Create initialiser list iterator with element constant access.
 */
template <typename Elem>
OZ_ALWAYS_INLINE
inline Arrays::CIterator<Elem> citerator(InitialiserList<Elem> li)
{
  return Arrays::CIterator<Elem>(li.begin(), li.begin() + li.size());
}

}
