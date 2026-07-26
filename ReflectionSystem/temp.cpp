#include <type_traits>
#include <utility>
#include  CollectionTraits.h
#include Reflection.h

static_assert(CollectionReflectionType<char[5]>);
