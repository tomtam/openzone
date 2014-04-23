/*
 * ozCore - OpenZone Core Library.
 *
 * Copyright © 2002-2014 Davorin Učakar
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
 * @file ozCore/Set.hh
 *
 * `Set` class template.
 */

#pragma once

#include "List.hh"

namespace oz
{

/**
 * Sorted array list.
 *
 * %Set is implemented as a sorted array list.
 * Better worst case performance than a hashtable; however, for large sets `HashSet` is preferred as
 * it is asymptotically faster in average case.
 *
 * Like in `List` all allocated elements are constructed all the time and a removed element's
 * destruction is still guaranteed.
 *
 * Memory is allocated when the first element is added.
 *
 * @sa `oz::Map`, `oz::HashSet`
 */
template <typename Elem>
class Set : protected List<Elem>
{
public:

  /**
   * %Iterator with constant access to elements.
   */
  typedef typename List<Elem>::CIterator CIterator;

  /**
   * %Iterator with non-constant access to elements.
   */
  typedef typename List<Elem>::Iterator Iterator;

protected:

  using List<Elem>::data;
  using List<Elem>::count;
  using List<Elem>::ensureCapacity;

public:

  using List<Elem>::citer;
  using List<Elem>::iter;
  using List<Elem>::begin;
  using List<Elem>::end;
  using List<Elem>::length;
  using List<Elem>::isEmpty;
  using List<Elem>::capacity;
  using List<Elem>::operator [];
  using List<Elem>::first;
  using List<Elem>::last;
  using List<Elem>::resize;
  using List<Elem>::trim;
  using List<Elem>::clear;
  using List<Elem>::free;
  using List<Elem>::allocate;
  using List<Elem>::deallocate;

  /**
   * Create an empty set with a given initial capacity.
   */
  explicit Set( int capacity = 0 ) :
    List<Elem>( capacity )
  {}

  /**
   * Initialise from an initialiser list.
   */
  Set( InitialiserList<Elem> l )
  {
    for( const Elem& e : l ) {
      add( e );
    }
  }

  /**
   * Copy constructor, copies elements.
   */
  Set( const Set& s ) :
    List<Elem>( s )
  {}

  /**
   * Move constructor, moves element storage.
   */
  Set( Set&& s ) :
    List<Elem>( static_cast<Set&&>( s ) )
  {}

  /**
   * Copy operator, copies elements.
   *
   * Existing storage is reused if it suffices.
   */
  Set& operator = ( const Set& s )
  {
    return static_cast<Set&>( List<Elem>::operator = ( s ) );
  }

  /**
   * Move operator, moves element storage.
   */
  Set& operator = ( Set&& s )
  {
    return static_cast<Set&>( List<Elem>::operator = ( static_cast<Set&&>( s ) ) );
  }

  /**
   * True iff respective elements are equal.
   */
  bool operator == ( const Set& s ) const
  {
    return List<Elem>::operator == ( s );
  }

  /**
   * False iff respective elements are equal.
   */
  bool operator != ( const Set& s ) const
  {
    return List<Elem>::operator != ( s );
  }

  /**
   * True iff a given key is found in the set.
   */
  template <typename Key = Elem>
  bool contains( const Key& key ) const
  {
    int i = aBisection<Elem, Key>( data, count, key );
    return i >= 0 && data[i] == key;
  }

  /**
   * Index of the element with a given value or -1 if not found.
   */
  template <typename Key = Elem>
  int index( const Key& key ) const
  {
    int i = aBisection<Elem, Key>( data, count, key );
    return i >= 0 && data[i] == key ? i : -1;
  }

  /**
   * Add an element overriding any existing equal element.
   *
   * @return Position of the inserted element.
   */
  template <typename Elem_ = Elem>
  int add( Elem_&& elem )
  {
    int i = aBisection<Elem, Elem>( data, count, elem );

    if( i >= 0 && data[i] == elem ) {
      data[i] = static_cast<Elem_&&>( elem );
      return i;
    }
    else {
      insert<Elem_>( i + 1, static_cast<Elem_&&>( elem ) );
      return i + 1;
    }
  }

  /**
   * Add an element if if there is no equal element in the set.
   *
   * @return Position of the inserted or the existing element.
   */
  template <typename Elem_ = Elem>
  int include( Elem_&& elem )
  {
    int i = aBisection<Elem, Elem>( data, count, elem );

    if( i >= 0 && data[i] == elem ) {
      return i;
    }
    else {
      insert<Elem_>( i + 1, static_cast<Elem_&&>( elem ) );
      return i + 1;
    }
  }

  /**
   * Insert an element at a given position.
   *
   * All later elements are shifted to make a gap.
   * Use only when you are sure you are inserting at the right position to preserve order of the
   * element.
   */
  template <typename Elem_ = Elem>
  void insert( int i, Elem_&& elem )
  {
    hard_assert( uint( i ) <= uint( count ) );

    ensureCapacity( count + 1 );

    aMoveBackward<Elem>( data + i, count - i, data + i + 1 );
    data[i] = static_cast<Elem_&&>( elem );

    ++count;
  }

  /**
   * Remove the element at a given position.
   *
   * All later elements are shifted to fill the gap.
   */
  void erase( int i )
  {
    hard_assert( uint( i ) < uint( count ) );

    --count;

    if( i == count ) {
      // When removing the last element, no shift is performed, so it is not implicitly destroyed by
      // the move operation.
      data[count] = Elem();
    }
    else {
      aMove<Elem>( data + i + 1, count - i, data + i );
    }
  }

  /**
   * Find and remove the element with a given value.
   *
   * @return Index of the removed element or -1 if not found.
   */
  int exclude( const Elem& elem )
  {
    int i = aBisection<Elem, Elem>( data, count, elem );

    if( i >= 0 && data[i] == elem ) {
      erase( i );
      return i;
    }
    return -1;
  }

};

}
