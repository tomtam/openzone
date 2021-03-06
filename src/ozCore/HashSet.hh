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
 * @file ozCore/HashSet.hh
 *
 * `HashSet` class template.
 */

#pragma once

#include "System.hh"
#include "Pool.hh"

namespace oz
{

/**
 * Chaining hashtable implementation, containing only keys without values.
 *
 * Memory is allocated when the first element is added. The number of buckets is doubled when the
 * number of elements surpasses it.
 *
 * @sa `oz::HashMap`, `oz::Set`, `oz::Heap`
 */
template <typename Elem, class HashFunc = Hash<Elem>>
class HashSet
{
protected:

  /// Granularity for bucket array and pool block sizes.
  static const int GRANULARITY = 256;

protected:

  /**
   * Bucket.
   */
  struct Entry
  {
    Entry* next; ///< Next entry in a bucket.
    int    hash; ///< Cached hash.
    Elem   elem; ///< Element (or key-value pair).

    OZ_PLACEMENT_POOL_ALLOC(Entry)
  };

  /**
   * Hashtable iterator.
   */
  template <class EntryType, typename ElemType>
  class HashIterator : public detail::IteratorBase<EntryType>
  {
  private:

    using detail::IteratorBase<EntryType>::elem_;

    const HashSet* table_ = nullptr; ///< Hashtable that is being iterated.
    int            index_ = 0;       ///< Index of the next bucket.

  public:

    /**
     * Create an invalid iterator.
     */
    HashIterator() = default;

    /**
     * Create hashtable iterator, initially pointing to the first hashtable element.
     */
    explicit HashIterator(const HashSet& table)
      : detail::IteratorBase<EntryType>(nullptr), table_(&table), index_(0)
    {
      while (elem_ == nullptr && index_ < table_->capacity_) {
        elem_ = table_->data_[index_];
        ++index_;
      }
    }

    /**
     * Pointer to the current element.
     */
    OZ_ALWAYS_INLINE
    operator ElemType*() const
    {
      return &elem_->elem;
    }

    /**
     * Reference to the current element.
     */
    OZ_ALWAYS_INLINE
    ElemType& operator*() const
    {
      return elem_->elem;
    }

    /**
     * Access to the current element's member.
     */
    OZ_ALWAYS_INLINE
    ElemType* operator->() const
    {
      return &elem_->elem;
    }

    /**
     * Advance to the next element.
     */
    HashIterator& operator++()
    {
      OZ_ASSERT(elem_ != nullptr);

      if (elem_->next != nullptr) {
        elem_ = elem_->next;
      }
      else if (index_ == table_->capacity_) {
        elem_ = nullptr;
      }
      else {
        do {
          elem_ = table_->data_[index_];
          ++index_;
        }
        while (elem_ == nullptr && index_ < table_->capacity_);
      }
      return *this;
    }

    /**
     * STL-style begin iterator.
     */
    OZ_ALWAYS_INLINE
    HashIterator begin() const
    {
      return *this;
    }

    /**
     * STL-style end iterator.
     */
    OZ_ALWAYS_INLINE
    HashIterator end() const
    {
      return HashIterator();
    }

  };

public:

  /**
   * %Iterator with constant access to elements.
   */
  typedef HashIterator<const Entry, const Elem> CIterator;

  /**
   * %Iterator with non-constant access to elements.
   */
  typedef HashIterator<Entry, Elem> Iterator;

protected:

  Pool<Entry> pool_     = Pool<Entry>(GRANULARITY); ///< Memory pool for entries.
  Entry**     data_     = nullptr;                  ///< Array of buckets (linked lists of entries).
  int         capacity_ = 0;                        ///< Number of buckets.

protected:

  /**
   * Resize bucket array and rebuild hashtable.
   */
  void resize(int newCapacity)
  {
    if (newCapacity < 0) {
      OZ_ERROR("oz::HashSet: Negative capacity (overflow?)");
    }

    Entry** newData = nullptr;

    if (newCapacity != 0) {
      newData = new Entry*[newCapacity] {};

      // Rebuild hashtable.
      for (int i = 0; i < capacity_; ++i) {
        Entry* chain = data_[i];
        Entry* next  = nullptr;

        while (chain != nullptr) {
          uint index = uint(chain->hash) % uint(newCapacity);

          next = chain->next;
          chain->next = newData[index];
          newData[index] = chain;

          chain = next;
        }
      }
    }
    delete[] data_;

    data_     = newData;
    capacity_ = newCapacity;
  }

  /**
   * Ensure a given size for the array of buckets.
   *
   * The capacity is increased if necessary with growth factor 1.5 or to (at least) 8 slots as the
   * initial allocation.
   */
  void ensureCapacity(int requestedCapacity)
  {
    if (requestedCapacity < 0) {
      OZ_ERROR("oz::HashSet: Capacity overflow");
    }
    else if (capacity_ < requestedCapacity) {
      int newCapacity = capacity_ == 0 ? 8 : capacity_ + capacity_ / 2;
      newCapacity = max<int>(capacity_, requestedCapacity);

      resize(newCapacity);
    }
  }

  /**
   * Insert an element, optionally overwriting an existing one.
   *
   * This is a helper function to reduce code duplication between `add()` and `include()`.
   */
  template <typename Elem_>
  Elem& insert(Elem_&& elem, bool overwrite)
  {
    ensureCapacity(pool_.size() + 1);

    int    h     = HashFunc()(elem);
    uint   index = uint(h) % uint(capacity_);
    Entry* entry = data_[index];

    while (entry != nullptr) {
      if (entry->elem == elem) {
        if (overwrite) {
          entry->elem = static_cast<Elem_&&>(elem);
        }
        return entry->elem;
      }
      entry = entry->next;
    }

    data_[index] = new(pool_) Entry{data_[index], h, static_cast<Elem_&&>(elem)};
    return *data_[index];
  }

public:

  /**
   * Create an empty hashtable.
   */
  HashSet() = default;

  /**
   * Create an empty hashtable with a given number of pre-allocated buckets.
   */
  explicit HashSet(int capacity)
  {
    resize(capacity);
  }

  /**
   * Initialise from an initialiser list.
   */
  HashSet(InitialiserList<Elem> il)
    : HashSet(int(il.size()) * 4 / 3)
  {
    for (const Elem& e : il) {
      add(e);
    }
  }

  /**
   * Destructor.
   */
  ~HashSet()
  {
    clear();
    delete[] data_;
  }

  /**
   * Copy constructor, copies elements but does not preserve bucket array length.
   */
  HashSet(const HashSet& ht)
    : HashSet(ht.pool_.size() * 4 / 3)
  {
    for (const Elem& e : ht) {
      add(e);
    }
  }

  /**
   * Move constructor, moves storage.
   */
  HashSet(HashSet&& other) noexcept
    : pool_(static_cast<Pool<Entry>&&>(other.pool_)), data_(other.data_), capacity_(other.capacity_)
  {
    other.data_     = nullptr;
    other.capacity_ = 0;
  }

  /**
   * Copy operator, copies elements but does not preserve bucket array length.
   */
  HashSet& operator=(const HashSet& other)
  {
    if (&other != this) {
      clear();
      ensureCapacity(other.pool_.size() * 4 / 3);

      for (const Elem& e : other) {
        add(e);
      }
    }
    return *this;
  }

  /**
   * Move operator, moves storage.
   */
  HashSet& operator=(HashSet&& other) noexcept
  {
    if (&other != this) {
      clear();
      delete[] data_;

      pool_     = static_cast<Pool<Entry>&&>(other.pool_);
      data_     = other.data_;
      capacity_ = other.capacity_;

      other.data_     = nullptr;
      other.capacity_ = 0;
    }
    return *this;
  }

  /**
   * Assign from an initialiser list.
   */
  HashSet& operator=(InitialiserList<Elem> il)
  {
    clear();
    ensureCapacity(int(il.size()) * 4 / 3);

    for (const Elem& e : il) {
      add(e);
    }
    return *this;
  }

  /**
   * True iff contained elements are equal.
   */
  bool operator==(const HashSet& other) const
  {
    if (pool_.size() != other.pool_.size()) {
      return false;
    }

    for (int i = 0; i < capacity_; ++i) {
      for (Entry* entry = data_[i]; entry != nullptr; entry = entry->next) {
        if (!other.contains(entry->elem)) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * False iff contained elements are equal.
   */
  bool operator!=(const HashSet& other) const
  {
    return !operator==(other);
  }

  /**
   * %Iterator with constant access, initially points to the first element.
   */
  OZ_ALWAYS_INLINE
  CIterator citerator() const
  {
    return CIterator(*this);
  }

  /**
   * %Iterator with non-constant access, initially points to the first element.
   */
  OZ_ALWAYS_INLINE
  Iterator iterator()
  {
    return Iterator(*this);
  }

  /**
   * STL-style constant begin iterator.
   */
  OZ_ALWAYS_INLINE
  CIterator begin() const
  {
    return CIterator(*this);
  }

  /**
   * STL-style begin iterator.
   */
  OZ_ALWAYS_INLINE
  Iterator begin()
  {
    return Iterator(*this);
  }

  /**
   * STL-style constant end iterator.
   */
  OZ_ALWAYS_INLINE
  CIterator end() const
  {
    return CIterator();
  }

  /**
   * STL-style end iterator.
   */
  OZ_ALWAYS_INLINE
  Iterator end()
  {
    return Iterator();
  }

  /**
   * Number of elements.
   */
  OZ_ALWAYS_INLINE
  int size() const
  {
    return pool_.size();
  }

  /**
   * True iff empty.
   */
  OZ_ALWAYS_INLINE
  bool isEmpty() const
  {
    return pool_.isEmpty();
  }

  /**
   * Length of bucket array.
   */
  OZ_ALWAYS_INLINE
  int capacity() const
  {
    return capacity_;
  }

  /**
   * Size of memory pool for elements.
   */
  OZ_ALWAYS_INLINE
  int poolCapacity() const
  {
    return pool_.capacity();
  }

  /**
   * True iff an element matching a given key is found in the hashtable.
   */
  template <typename Key>
  bool contains(const Key& key) const
  {
    if (capacity_ == 0) {
      return false;
    }

    int    h     = HashFunc()(key);
    uint   index = uint(h) % uint(capacity_);
    Entry* entry = data_[index];

    while (entry != nullptr) {
      if (entry->elem == key) {
        return true;
      }

      entry = entry->next;
    }
    return false;
  }

  /**
   * Add a new element, if the element already exists in the hashtable overwrite the existing one.
   */
  template <typename Elem_>
  Elem& add(Elem_&& elem)
  {
    return insert(static_cast<Elem_&&>(elem), true);
  }

  /**
   * Add a new element if it does not exist in the hashtable.
   */
  template <typename Elem_>
  Elem& include(Elem_&& elem)
  {
    return insert(static_cast<Elem_&&>(elem), false);
  }

  /**
   * Remove the element that matches a given key.
   *
   * @return True iff the element was found (and removed).
   */
  template <typename Key>
  bool exclude(const Key& key)
  {
    if (capacity_ == 0) {
      return false;
    }

    int     h     = HashFunc()(key);
    uint    index = uint(h) % uint(capacity_);
    Entry*  entry = data_[index];
    Entry** prev  = &data_[index];

    while (entry != nullptr) {
      if (entry->elem == key) {
        *prev = entry->next;

        entry->~Entry();
        pool_.deallocate(entry);

        return true;
      }

      prev = &entry->next;
      entry = entry->next;
    }
    return false;
  }

  /**
   * Trim the bucket array size to 4/3 of the current number of elements.
   *
   * In case the hastable contains no entries all its storage gets deallocated.
   */
  void trim()
  {
    int newCapacity = pool_.size() * 4 / 3;

    if (newCapacity < capacity_) {
      resize(newCapacity);

      if (newCapacity == 0) {
        pool_.free();
      }
    }
  }

  /**
   * Clear the hashtable.
   */
  void clear()
  {
    for (int i = 0; i < capacity_; ++i) {
      Entry* entry = data_[i];

      while (entry != nullptr) {
        Entry* next = entry->next;

        entry->~Entry();
        pool_.deallocate(entry);

        entry = next;
      }

      data_[i] = nullptr;
    }
  }

};

}
