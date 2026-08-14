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
  size_t size;
};

struct StructMember
{
  const char* name;
  size_t offset;
  const TypeDesc_Base* type;
};

struct TypeDesc_Struct : public TypeDesc_Base
{  
  const StructMember* members;
  const unsigned int memberCount;
  const TypeDesc_Struct* parentTypes;
  const unsigned int parentTypeCount;
};

struct TypeDesc_Primitive : public TypeDesc_Base
{
};

struct TypeDesc_Collection : public TypeDesc_Base
{
  const TypeDesc_Base* elementType;

  bool isResizable;

  void (*resize)(void* collection, size_t newSize);
  size_t(*getSize)(const void* collection);
  const void* (*getElement)(const void* collection, int index);
  void (*setElement)(void* collection, int index, const void* element);
};

// Reflection type concepts

template<typename T>
concept CollectionReflectionType =
  requires
{
  typename CollectionElement<T>::type;
}
&&
  requires(const T& collection, T& mutableCollection, size_t index)
{
  collection[index];
  mutableCollection[index];
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

#define DECLARE_STRUCT_TYPE(...) \
    template<> constexpr const TypeDesc_Struct* GetTypeDesc<__VA_ARGS__>();

#define DECLARE_PRIMITIVE_TYPE(...) \
    template<> constexpr const TypeDesc_Primitive* GetTypeDesc<__VA_ARGS__>();

#define DECLARE_COLLECTION_TYPE(...) \
    template<> constexpr const TypeDesc_Collection* GetTypeDesc<__VA_ARGS__>();    

// Struct reflection implementation macros

#define IMPLEMENT_STRUCT_BASE(Base) \
    GetTypeDesc<Base>(),

#define IMPLEMENT_STRUCT_MEMBER(member) \
    {#member, offsetof(__Type, member), GetTypeDesc<decltype(__Type::member)>()},

#define __IMPLEMENT_STRUCT_TYPE_IMPL(T, ID, BASES, MEMBERS) \
    static_assert(StructReflectionType<REMOVE_PARENS(T)>, "Type must be a struct/class type.");  \
    namespace CAT(Refl_NS_, ID) { \
      using __Type = REMOVE_PARENS(T); \
      static constexpr const TypeDesc_Struct* g_bases[] = {  \
          FOR_EACH(IMPLEMENT_STRUCT_BASE, REMOVE_PARENS(BASES)) \
          nullptr \
      }; \
      static constexpr StructMember g_members[] = { \
          FOR_EACH(IMPLEMENT_STRUCT_MEMBER, REMOVE_PARENS(MEMBERS)) \
      };  \
      __declspec(allocate("reflection$b")) \
      constexpr TypeDesc_Struct g_type = { \
          GetTypeName<REMOVE_PARENS(T)>(),  \
          sizeof(__Type),  \
          g_members,  \
          static_cast<unsigned int>(sizeof(g_members) / sizeof(StructMember)), \
          g_bases[0],  \
          static_cast<unsigned int>((sizeof(g_bases) / sizeof(TypeDesc_Struct*)) - 1)  \
      };  \
    } \
    template<>  \
    constexpr const TypeDesc_Struct* GetTypeDesc<REMOVE_PARENS(T)>() {  \
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
    static_assert(CollectionReflectionType<REMOVE_PARENS(T)>, "Type must be a collection type.");  \
    namespace CAT(Refl_NS_, ID) { \
      using __Type = REMOVE_PARENS(T); \
      __declspec(allocate("reflection$h"))  \
      constexpr TypeDesc_Collection g_type = {  \
          GetTypeName<REMOVE_PARENS(T)>(),  \
          sizeof(__Type),  \
          GetTypeDesc<typename CollectionTraits<__Type>::Element>(),  \
          CollectionTraits<__Type>::isResizable,  \
          [](void* obj, size_t size) -> void {  \
            CollectionTraits<__Type>::Resize(static_cast<std::add_pointer_t<__Type>>(obj), size); \
          },  \
          [](const void* obj) -> size_t { \
            return CollectionTraits<__Type>::GetSize(static_cast<std::add_pointer_t<std::add_const_t<__Type>>>(obj)); \
          },  \
          [](const void* obj, int index) -> const void* { \
            return static_cast<const void*>(CollectionTraits<__Type>::GetElement(static_cast<std::add_pointer_t<std::add_const_t<__Type>>>(obj), index)); \
          },  \
          [](void* obj, int index, const void* elem) -> void { \
            CollectionTraits<__Type>::SetElement(static_cast<std::add_pointer_t<__Type>>(obj), index, static_cast<CollectionTraits<__Type>::ElementConstPtr>(elem)); \
          } \
      };  \
    } \
    template<>  \
    constexpr const TypeDesc_Collection* GetTypeDesc<REMOVE_PARENS(T)>() {  \
        return &CAT(Refl_NS_, ID)::g_type;  \
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
