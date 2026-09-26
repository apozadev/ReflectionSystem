#pragma once
#include <type_traits>
#include <assert.h>

//Primary fallback template
template<typename T, typename = void>
struct CollectionElement
{
};

// Anything with ::value_type (vector, array, deque, string, ...)
template<typename T>
struct CollectionElement<T, std::void_t<typename T::value_type>>
{
  using type = typename T::value_type;
};
// C-style array specialization
template<typename T, size_t N>
struct CollectionElement<T[N]>
{
    using type = T;
};

template<typename T>
concept Resizable = requires(T t, size_t n)
{
    t.resize(n);
};

template<typename T>
struct CollectionTraits
{
    using Element         = typename CollectionElement<T>::type;
    using ElementPtr      = std::add_pointer_t<Element>;
    using ElementConstPtr = std::add_pointer_t<std::add_const_t<Element>>;    

    using CollectionPtr = std::add_pointer_t<T>;
    using CollectionConstPtr = std::add_pointer_t<std::add_const_t<T>>;

    // Resize is a function pointer instead of a method so it can be nullptr if the collection is not resizable. 
    // This allows for compile-time checking of whether a collection is resizable or not.
    inline static constexpr void(*resize)(void*, size_t) = []() constexpr -> void(*)(void*, size_t)
      {
        if constexpr (Resizable<T>)
        {
          return [](void* obj, size_t newSize)-> void
            {
              static_cast<CollectionPtr>(obj)->resize(newSize);
            };
        }
        else
        {
          return nullptr;
        }
      }();

    static constexpr size_t GetSize(const void* collection)
    {
        if constexpr (std::is_array_v<T>)
        {
            (void)collection;
            return std::extent_v<T>;
        }
        else
        {
            return static_cast<size_t>(static_cast<CollectionConstPtr>(collection)->size());
        }
    }

    static constexpr const void* GetElement(const void* collection, int index)
    {
        return &(*static_cast<CollectionConstPtr>(collection))[index];
    }

    static constexpr void SetElement(void* collection, int index, const void* value)
    {
        (*static_cast<CollectionPtr>(collection))[index] = *static_cast<ElementConstPtr>(value);
    }
};