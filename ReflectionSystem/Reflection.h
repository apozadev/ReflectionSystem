#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <string_view>

#include "CollectionTraits.h"
#include "Traits.h"
#include "TypeName.h"
#include "MacroUtils.h"

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

  void (*construct)(void*);
  void (*destruct)(void*);
};

struct TypeDesc_Primitive : public TypeDesc_Base
{
};

struct alignas(8) TypeDesc_Collection : public TypeDesc_Base
{
  const TypeDesc_Base* elementType;

  void (*resize)(void* collection, size_t newSize);
  size_t(*getSize)(const void* collection);
  const void* (*getElement)(const void* collection, int index);
  void (*setElement)(void* collection, int index, const void* element);

  void (*construct)(void*);
  void (*destruct)(void*);
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
      constexpr TypeDesc_Struct g_type = { \
          GetTypeName<REMOVE_PARENS(T)>(),  \
          sizeof(__Type),  \
          g_members,  \
          static_cast<unsigned int>(sizeof(g_members) / sizeof(StructMember)), \
          g_bases[0],  \
          static_cast<unsigned int>((sizeof(g_bases) / sizeof(TypeDesc_Struct*)) - 1),  \
          Traits<__Type>::defaultConstructor,  \
          Traits<__Type>::destructor  \
      };  \
      static Registrar s_typeRegistrar(&g_type);  \
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
    constexpr TypeDesc_Primitive CAT(g_type_, ID) = {  \
        GetTypeName<T>(),  \
        sizeof(T)  \
    };  \
    static Registrar CAT(s_typeRegistrar, ID)(&CAT(g_type_, ID));  \
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
      constexpr TypeDesc_Collection g_type = []() constexpr { \
        TypeDesc_Collection type{}; \
        type.name = GetTypeName<REMOVE_PARENS(T)>(); \
        type.size = sizeof(__Type); \
        type.elementType = GetTypeDesc<typename CollectionTraits<__Type>::Element>(); \
        type.resize = CollectionTraits<__Type>::resize; \
        type.getSize = CollectionTraits<__Type>::GetSize; \
        type.getElement = CollectionTraits<__Type>::GetElement; \
        type.setElement = CollectionTraits<__Type>::SetElement; \
        type.construct = Traits<__Type>::defaultConstructor; \
        type.destruct = Traits<__Type>::destructor; \
        return type; \
      }(); \
      static Registrar s_typeRegistrar(&g_type);  \
    } \
    template<>  \
    constexpr const TypeDesc_Collection* GetTypeDesc<REMOVE_PARENS(T)>() {  \
        return &CAT(Refl_NS_, ID)::g_type;  \
    }

#define IMPLEMENT_COLLECTION_TYPE(T) \
    _IMPLEMENT_COLLECTION_TYPE_IMPL(T, __COUNTER__)

// Type registry for looking up type descriptors by name

struct Registrar;

struct TypeRegistry
{
  static TypeRegistry& Get()
  {
    static TypeRegistry s_registry{};
    return s_registry;
  }  

  const std::unordered_map<std::string_view, const TypeDesc_Base*>& GetTypes() const
  {
    return m_types;
  }

  const TypeDesc_Base* GetTypeDescByName(const char* _sName)
  {
    auto it = m_types.find(_sName);
    if (it != m_types.end())
    {
      return it->second;
    }
    return nullptr;
  }

private:

  friend struct Registrar;

  std::unordered_map<std::string_view, const TypeDesc_Base*> m_types;
};

struct Registrar
{
  Registrar(const TypeDesc_Base* _pTypeDesc)
  {
    TypeRegistry::Get().m_types.insert({ _pTypeDesc->name, _pTypeDesc });
  }
};
