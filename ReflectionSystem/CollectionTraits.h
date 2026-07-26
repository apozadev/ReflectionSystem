#pragma once

#include <type_traits>
#include <assert.h>

template<typename T, typename = void>
struct CollectionTraits;

// Any STL-like container
template<typename T>
struct CollectionTraits<T, std::void_t<typename T::value_type>>
{
  using Element = typename T::value_type;

  typedef std::add_pointer_t<Element>                   ElementPtr;
  typedef std::add_pointer_t<std::add_const_t<Element>> ElementConstPtr;

  static int GetSize(const T* collection)
  {
    return collection->size();
  }

  static void Resize(T* collection, size_t newSize)
  {
    collection->size(newSize);
  }

  static ElementConstPtr GetElement(const T* collection, int index)
  {
    return &(*collection)[index];
  }

  static void SetElement(T* collection, int index, ElementConstPtr value)
  {
    (*collection)[index] = value;
  }
};

// C arrays
template<typename T, size_t N>
struct CollectionTraits<T[N], void>
{
  using Element = T;

  typedef std::add_pointer_t<Element>                   ElementPtr;
  typedef std::add_pointer_t<std::add_const_t<Element>> ElementConstPtr;

  using Array = T[N];

  static int GetSize(const Array*)
  {
    return N;
  }

  static void Resize(T(*)[N], size_t)
  {
    assert(false);
  }

  static ElementConstPtr GetElement(const Array* collection, int index)
  {
    return &(*collection)[index];
  }

  static void SetElement(Array* collection, int index, ElementConstPtr value)
  {
    (*collection)[index] = *value;
  }
};


