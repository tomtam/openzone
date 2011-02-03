/*
 *  Sparse.hpp
 *
 *  Sparse vector
 *  Similar to Vector, but it can have holes in the middle. When a new elements are added it first
 *  tries to occupy all free slots, new element is added to the end only if there is no holes in
 *  the middle.
 *  Type should provide int nextSlot[INDEX] field.
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "arrays.hpp"

namespace oz
{

  template <class Type, int INDEX = 0, int GRANULARITY = 8>
  class Sparse
  {
    static_assert( GRANULARITY > 0, "GRANULARITY must be at least 1" );

    public:

      /**
       * Constant Sparse iterator.
       */
      class CIterator : public oz::CIterator<Type>
      {
        private:

          typedef oz::CIterator<Type> B;

        public:

          /**
           * Default constructor returns an invalid iterator
           */
          OZ_ALWAYS_INLINE
          explicit CIterator() : B( null, null )
          {}

          /**
           * Make iterator for given Sparse. After creation it points to first element.
           * @param s
           */
          OZ_ALWAYS_INLINE
          explicit CIterator( const Sparse& s ) : B( s.data, s.data + s.size )
          {}

          /**
           * Advance to next element.
           * @return
           */
          CIterator& operator ++ ()
          {
            do {
              ++B::elem;
            }
            while( B::elem != B::past && B::elem->nextSlot[INDEX] != -1 );

            return *this;
          }

      };

      /**
       * Sparse iterator.
       */
      class Iterator : public oz::Iterator<Type>
      {
        private:

          typedef oz::Iterator<Type> B;

        public:

          /**
           * Default constructor returns an invalid iterator
           */
          OZ_ALWAYS_INLINE
          explicit Iterator() : B( null, null )
          {}

          /**
           * Make iterator for given Sparse. After creation it points to first element.
           * @param s
           */
          OZ_ALWAYS_INLINE
          explicit Iterator( const Sparse& s ) : B( s.data, s.data + s.size )
          {}

          /**
           * Advance to next element.
           * @return
           */
          Iterator& operator ++ ()
          {
            do {
              ++B::elem;
            }
            while( B::elem != B::past && B::elem->nextSlot[INDEX] != -1 );

            return *this;
          }

      };

    private:

      // Pointer to data array
      Type* data;
      // Size of data array
      int   size;
      // Number of used slots in the sparse sparse vector
      int   count;
      // List of free slots (by indices in data array, not by pointers), freeSlot == size if out of
      // free slots
      int   freeSlot;

      /**
       * Enlarge capacity by two times if there's not enough space to add another element.
       */
      void ensureCapacity()
      {
        assert( ( count == size ) == ( freeSlot == size ) );

        if( freeSlot == size ) {
          if( size == 0 ) {
            size = GRANULARITY;
            data = new Type[size];
          }
          else {
            size *= 2;
            data = aRealloc( data, count, size );
          }

          freeSlot = count;
          for( int i = count; i < size; ++i ) {
            data[i].nextSlot[INDEX] = i + 1;
          }
        }
      }

    public:

      /**
       * Create empty sparse vector with initial capacity 8.
       */
      explicit Sparse() : data( null ), size( 0 ), count( 0 ), freeSlot( 0 )
      {}

      /**
       * Create empty sparse vector with given initial capacity.
       * @param initSize
       */
      explicit Sparse( int initSize ) : data( initSize == 0 ? null : new Type[initSize] ),
          size( initSize ), count( 0 ), freeSlot( 0 )
      {
        for( int i = 0; i < size; ++i ) {
          data[i].nextSlot[INDEX] = i + 1;
        }
      }

      /**
       * Copy constructor.
       * @param s
       */
      Sparse( const Sparse& s ) : data( s.size == 0 ? null : new Type[s.size] ),
          size( s.size ), count( s.count ), freeSlot( s.freeSlot )
      {
        aCopy( data, s.data, size );
      }

      /**
       * Destructor.
       */
      ~Sparse()
      {
        delete[] data;
      }

      /**
       * Copy operator.
       * @param s
       * @return
       */
      Sparse& operator = ( const Sparse& s )
      {
        assert( &s != this );

        // create new data array of the new data doesn't fit, keep the old one otherwise
        if( size < s.size ) {
          delete[] data;

          data = new Type[s.size];
          size = s.size;
        }

        aCopy( data, s.data, s.size );
        count = s.count;
        freeSlot = s.freeSlot;
        return *this;
      }

      /**
       * Equality operator. Capacity of sparse vectors doesn't matter.
       * @param s
       * @return true if all elements in both sparse vectors are equal
       */
      bool operator == ( const Sparse& s ) const
      {
        if( count != s.count ) {
          return false;
        }

        int i = 0;
        while( i < size && ( data[i].nextSlot[INDEX] > 0 || data[i] == s.data[i] ) ) {
          ++i;
        }
        return i == size;
      }

      /**
       * Inequality operator. Capacity of sparse vectors doesn't matter.
       * @param s
       * @return false if all elements in both sparse vectors are equal
       */
      bool operator != ( const Sparse& s ) const
      {
        if( count != s.count ) {
          return true;
        }

        int i = 0;
        while( i < size && ( data[i].nextSlot[INDEX] > 0 || data[i] == s.data[i] ) ) {
          ++i;
        }
        return i != size;
      }

      /**
       * @return constant iterator for this vector
       */
      OZ_ALWAYS_INLINE
      CIterator citer() const
      {
        return CIterator( *this );
      }

      /**
       * @return iterator for this vector
       */
      OZ_ALWAYS_INLINE
      Iterator iter() const
      {
        return Iterator( *this );
      }

      /**
       * Get pointer to <code>data</code> array. Use with caution, since you can easily make buffer
       * overflows if you don't check the size of <code>data</code> array.
       * @return constant pointer to data array
       */
      OZ_ALWAYS_INLINE
      operator const Type* () const
      {
        return data;
      }

      /**
       * Get pointer to <code>data</code> array. Use with caution, since you can easily make buffer
       * overflows if you don't check the size of <code>data</code> array.
       * @return non-constant pointer to data array
       */
      OZ_ALWAYS_INLINE
      operator Type* ()
      {
        return data;
      }

      /**
       * @return number of elements (occupied slots) in the sparse vector
       */
      OZ_ALWAYS_INLINE
      int length() const
      {
        return count;
      }

      /**
       * @return true if sparse vector has no elements
       */
      OZ_ALWAYS_INLINE
      bool isEmpty() const
      {
        return count == 0;
      }

      /**
       * @return capacity of the sparse vector
       */
      OZ_ALWAYS_INLINE
      int capacity() const
      {
        return size;
      }

      /**
       * @param i
       * @return true if slot at given index in use
       */
      bool hasIndex( int i ) const
      {
        assert( uint( i ) < uint( size ) );

        return data[i].nextSlot[INDEX] != -1;
      }

      /**
       * @param e
       * @return true if the element is found in the sparse vector
       */
      bool contains( const Type& e ) const
      {
        for( int i = 0; i < size; ++i ) {
          if( data[i].nextSlot[INDEX] == -1 && data[i] == e ) {
            return true;
          }
        }
        return false;
      }

      /**
       * @param i
       * @return constant reference i-th element
       */
      OZ_ALWAYS_INLINE
      const Type& operator [] ( int i ) const
      {
        assert( 0 <= i && i < size );

        return data[i];
      }

      /**
       * @param i
       * @return reference i-th element
       */
      OZ_ALWAYS_INLINE
      Type& operator [] ( int i )
      {
        assert( 0 <= i && i < size );

        return data[i];
      }

      /**
       * Find the first occurrence of an element.
       * @param e
       * @return index of first occurrence, -1 if not found
       */
      int index( const Type& e ) const
      {
        for( int i = 0; i < size; ++i ) {
          if( data[i].nextSlot[INDEX] == -1 && data[i] == e ) {
            return i;
          }
        }
        return -1;
      }

      /**
       * Find the last occurrence of an element.
       * @param e
       * @return index of last occurrence, -1 if not found
       */
      int lastIndex( const Type& e ) const
      {
        for( int i = size - 1; i >= 0; --i ) {
          if( data[i].nextSlot[INDEX] == -1 && data[i] == e ) {
            return i;
          }
        }
        return -1;
      }

      /**
       * Create slot for a new element.
       * @return index at which the slot was created
       */
      int add()
      {
        ensureCapacity();

        int i = freeSlot;

        freeSlot = data[i].nextSlot[INDEX];
        data[i].nextSlot[INDEX] = -1;
        ++count;

        return i;
      }

      /**
       * Add an element.
       * @param e
       * @return index at which the element was inserted
       */
      int add( const Type& e )
      {
        ensureCapacity();

        int i = freeSlot;

        freeSlot = data[i].nextSlot[INDEX];
        data[i] = e;
        data[i].nextSlot[INDEX] = -1;
        ++count;

        return i;
      }

      /**
       * Remove the element at given position. A gap will remain there.
       * @param i
       */
      void remove( int i )
      {
        assert( uint( i ) < uint( size ) );

        data[i].nextSlot[INDEX] = freeSlot;
        freeSlot = i;
        --count;
      }

      /**
       * Empty the sparse vector but don't delete the elements.
       */
      void clear()
      {
        for( int i = 0; i < size; ++i ) {
          data[i].nextSlot[INDEX] = i + 1;
        }

        count = 0;
        freeSlot = 0;
      }

      /**
       * Allocates capacity for initSize elements. It analoguous to Sparse( initSize ) constructor
       * if one want to reserving size on construction cannot be done.
       * Sparse vector must be empty for this function to work.
       * @param initSize
       */
      void alloc( int initSize )
      {
        assert( size == 0 && initSize > 0 );

        data = new Type[initSize];
        size = initSize;

        for( int i = 0; i < size; ++i ) {
          data[i].nextSlot[INDEX] = i + 1;
        }
      }

      /**
       * Deallocate rosources.
       * Container must be empty for this function to work.
       */
      void dealloc()
      {
        assert( count == 0 );

        delete[] data;

        data = null;
        size = 0;
        count = 0;
        freeSlot = 0;
      }

  };

}
