#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <string_view>

#include "CollectionTraits.h"
#include "MacroUtils.h"
#include "TypeName.h"

struct TypeDesc_Base
{
  const char* name;
};

struct StructMember
{
  const char* name;
  size_t offset;
  const TypeDesc_Base* type;
};

struct TypeDesc_Struct : public TypeDesc_Base
{
  size_t size;
  const StructMember* members;
  const unsigned int memberCount;
  const TypeDesc_Struct* parentTypes;
  const unsigned int parentTypeCount;
};

struct TypeDesc_Primitive : public TypeDesc_Base
{
  size_t size;
};

struct TypeDesc_Collection : public TypeDesc_Base
{
  const TypeDesc_Base* elementType;

  void (*resize)(void* collection, size_t newSize);
  size_t(*getSize)(const void* collection);
  const void* (*getElement)(const void* collection, int index);
  void (*setElement)(void* collection, int index, const void* element);
};

// Reflection type concepts

template<typename T>
concept CollectionReflectionType =
  requires {
    typename CollectionTraits<std::remove_cvref_t<T>>::Element;
    { CollectionTraits<std::remove_cvref_t<T>>::GetSize(std::declval<const std::remove_cvref_t<T>*>()) } -> std::convertible_to<size_t>;
    { CollectionTraits<std::remove_cvref_t<T>>::Resize(std::declval<std::remove_cvref_t<T>*>(), size_t{}) };
    { CollectionTraits<std::remove_cvref_t<T>>::GetElement(std::declval<const std::remove_cvref_t<T>*>(), int{}) } -> std::convertible_to<typename CollectionTraits<T>::ElementConstPtr>;
    { CollectionTraits<std::remove_cvref_t<T>>::SetElement(std::declval<std::remove_cvref_t<T>*>(), int{}, std::declval<typename CollectionTraits<T>::ElementConstPtr>()) };
  };

template<typename T>
concept PrimitiveReflectionType =
std::is_arithmetic_v<std::remove_cvref_t<T>> ||
std::is_enum_v<std::remove_cvref_t<T>>;

template<typename T>
concept StructReflectionType =
std::is_class_v<std::remove_cvref_t<T>> &&
!PrimitiveReflectionType<T> && 
!CollectionReflectionType<T>;

template<typename T>
concept ReflectionType = PrimitiveReflectionType<T> || StructReflectionType<T> || CollectionReflectionType<T>;

template<class>
inline constexpr bool always_false = false;

// Fallback for types that don't have reflection data defined

template<typename T>
  requires PrimitiveReflectionType<T>
constexpr const TypeDesc_Primitive* GetTypeDesc() { static_assert(always_false<T>, "Primitive type not in reflection."); return nullptr; }

template<typename T>
  requires StructReflectionType<T>
constexpr const TypeDesc_Struct* GetTypeDesc() { static_assert(always_false<T>, "Struct type not in reflection."); return nullptr; }

template<typename T>
  requires CollectionReflectionType<T>
constexpr const TypeDesc_Collection* GetTypeDesc() { static_assert(always_false<T>, "Collection type not in reflection."); return nullptr; }

// Reflection data section boundaries

extern const TypeDesc_Struct StructReflectionStart;
extern const TypeDesc_Struct StructReflectionEnd;

extern const TypeDesc_Primitive PrimitiveReflectionStart;
extern const TypeDesc_Primitive PrimitiveReflectionEnd;

extern const TypeDesc_Collection CollectionReflectionStart;
extern const TypeDesc_Collection CollectionReflectionEnd;

#pragma section("reflection$b", read) // Struct reflection data section
#pragma section("reflection$e", read) // Primitive reflection data section
#pragma section("reflection$h", read) // Collection reflection data section

// Reflection types declaration macros

#define DECLARE_STRUCT_TYPE(T) \
    template<> constexpr const TypeDesc_Struct* GetTypeDesc<T>();

#define DECLARE_PRIMITIVE_TYPE(T) \
    template<> constexpr const TypeDesc_Primitive* GetTypeDesc<T>();

#define DECLARE_COLLECTION_TYPE(T) \
    template<> constexpr const TypeDesc_Collection* GetTypeDesc<T>();    

// Struct reflection implementation macros

#define IMPLEMENT_STRUCT_BASE(Base) \
    GetTypeDesc<Base>(),

#define IMPLEMENT_STRUCT_MEMBER(member) \
    {#member, offsetof(__Type, member), GetTypeDesc<decltype(__Type::member)>()},

#define __IMPLEMENT_STRUCT_TYPE_IMPL(T, ID, BASES, MEMBERS) \
    static_assert(StructReflectionType<T>, "Type must be a struct/class type.");  \
    namespace CAT(Refl_NS_, ID) { \
      using __Type = T; \
      static constexpr const TypeDesc_Struct* g_bases[] = {  \
          FOR_EACH(IMPLEMENT_STRUCT_BASE, REMOVE_PARENS(BASES)) \
          nullptr \
      }; \
      static constexpr StructMember g_members[] = { \
          FOR_EACH(IMPLEMENT_STRUCT_MEMBER, REMOVE_PARENS(MEMBERS)) \
      };  \
      __declspec(allocate("reflection$b")) \
      constexpr TypeDesc_Struct g_type = { \
          GetTypeName<T>(),  \
          sizeof(T),  \
          g_members,  \
          static_cast<unsigned int>(sizeof(g_members) / sizeof(StructMember)), \
          g_bases[0],  \
          static_cast<unsigned int>((sizeof(g_bases) / sizeof(TypeDesc_Struct*)) - 1)  \
      };  \
    } \
    template<>  \
    constexpr const TypeDesc_Struct* GetTypeDesc<T>() {  \
        return &CAT(Refl_NS_, ID)::g_type;  \
    }

#define IMPLEMENT_STRUCT_TYPE(T, BASES, MEMBERS) \
    __IMPLEMENT_STRUCT_TYPE_IMPL(T, __COUNTER__, BASES, MEMBERS)

// Primitive reflection implementation macro

#define _IMPLEMENT_PRIMITIVE_TYPE_IMPL(T, ID) \
    static_assert(PrimitiveReflectionType<T>, "Type must be a primitive type.");  \
    __declspec(allocate("reflection$e"))  \
    constexpr TypeDesc_Primitive CAT(g_type_, ID) = {  \
        GetTypeName<T>(),  \
        sizeof(T)  \
    };  \
    template<>  \
    constexpr const TypeDesc_Primitive* GetTypeDesc<T>() {  \
        return &CAT(g_type_, ID);  \
    }

#define IMPLEMENT_PRIMITIVE_TYPE(T) \
    _IMPLEMENT_PRIMITIVE_TYPE_IMPL(T, __COUNTER__)

// Collection reflection implementation macro

#define _IMPLEMENT_COLLECTION_TYPE_IMPL(T, ID) \
    static_assert(CollectionReflectionType<T>, "Type must be a collection type.");  \
    __declspec(allocate("reflection$h"))  \
    constexpr TypeDesc_Collection CAT(g_type_Coll_, ID) = {  \
        GetTypeName<T>(),  \
        GetTypeDesc<typename CollectionTraits<T>::Element>(),  \
        [](void* obj, size_t size) -> void {  \
          CollectionTraits<T>::Resize(static_cast<std::add_pointer_t<T>>(obj), size); \
        },  \
        [](const void* obj) -> size_t { \
          return CollectionTraits<T>::GetSize(static_cast<std::add_pointer_t<std::add_const_t<T>>>(obj)); \
        },  \
        [](const void* obj, int index) -> const void* { \
          return static_cast<const void*>(CollectionTraits<T>::GetElement(static_cast<std::add_pointer_t<std::add_const_t<T>>>(obj), index)); \
        },  \
        [](void* obj, int index, const void* elem) -> void { \
          CollectionTraits<T>::SetElement(static_cast<std::add_pointer_t<T>>(obj), index, static_cast<CollectionTraits<T>::ElementConstPtr>(elem)); \
        } \
    };  \
    template<>  \
    constexpr const TypeDesc_Collection* GetTypeDesc<T>() {  \
        return &CAT(g_type_Coll_, ID);  \
    }

#define IMPLEMENT_COLLECTION_TYPE(T) \
    _IMPLEMENT_COLLECTION_TYPE_IMPL(T, __COUNTER__)

// Type registry for looking up type descriptors by name

inline std::unordered_map<std::string_view, const TypeDesc_Base*>& GetTypeRegistry()
{
  static std::unordered_map<std::string_view, const TypeDesc_Base*> typeRegistry = []()
    {
      std::unordered_map<std::string_view, const TypeDesc_Base*> registry;

      const TypeDesc_Struct* structTypesStart = &StructReflectionStart;
      const TypeDesc_Struct* structTypesEnd = &StructReflectionEnd;
      for (const TypeDesc_Struct* typeDesc = structTypesStart + 1; typeDesc != structTypesEnd; ++typeDesc)
      {
        registry.insert({ typeDesc->name, typeDesc });
      }

      const TypeDesc_Collection* collectionTypesStart = &CollectionReflectionStart;
      const TypeDesc_Collection* collectionTypesEnd = &CollectionReflectionEnd;
      for (const TypeDesc_Collection* typeDesc = collectionTypesStart + 1; typeDesc != collectionTypesEnd; ++typeDesc)
      {
        registry.insert({ typeDesc->name, typeDesc });
      }

      const TypeDesc_Primitive* primitiveTypesStart = &PrimitiveReflectionStart;
      const TypeDesc_Primitive* primitiveTypesEnd = &PrimitiveReflectionEnd;
      for (const TypeDesc_Primitive* typeDesc = primitiveTypesStart + 1; typeDesc != primitiveTypesEnd; ++typeDesc)
      {
        registry.insert({ typeDesc->name, typeDesc });
      }

      return registry;
    }();

  return typeRegistry;
}

inline const TypeDesc_Base* GetTypeDescByName(std::string_view typeName)
{
  const auto& registry = GetTypeRegistry();
  auto it = registry.find(typeName);
  if (it != registry.end())
  {
    return it->second;
  }
  return nullptr;
}
