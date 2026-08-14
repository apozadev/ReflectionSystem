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

template<typename T>              // <-- single, unconditional primary template
struct CollectionTraits
{
    using Element         = typename CollectionElement<T>::type;
    using ElementPtr      = std::add_pointer_t<Element>;
    using ElementConstPtr = std::add_pointer_t<std::add_const_t<Element>>;

    static constexpr bool isResizable = Resizable<T>;

    static int GetSize(const T* collection)
    {
        if constexpr (std::is_array_v<T>)
        {
            (void)collection;
            return static_cast<int>(std::extent_v<T>);
        }
        else
        {
            return static_cast<int>(collection->size());
        }
    }

    static void Resize(T* collection, size_t newSize)
    {
        if constexpr (isResizable)
        {
            collection->resize(newSize);
        }
        else
        {
            (void)collection;
            (void)newSize;
            assert(false);
        }
    }

    static ElementConstPtr GetElement(const T* collection, int index)
    {
        return &(*collection)[index];
    }

    static void SetElement(T* collection, int index, ElementConstPtr value)
    {
        (*collection)[index] = *value;
    }
};