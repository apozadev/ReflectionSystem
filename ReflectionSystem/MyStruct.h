#pragma once

#include "Reflection.h"

struct MyBase
{
  int x;
};

struct MyStruct : public MyBase
{
  int a, b;
  char c[5];
};

namespace MyNamespace
{
  template <typename T>
  class MyTemplateStruct
  {
  public:
    T a, b;
  };
}

template<> constexpr const TypeDesc_Primitive* GetTypeDesc<int>();

DECLARE_PRIMITIVE_TYPE(char)

DECLARE_COLLECTION_TYPE(char[5])

DECLARE_COLLECTION_TYPE(std::array<int, 10>)
DECLARE_COLLECTION_TYPE(std::vector<MyStruct>)

DECLARE_STRUCT_TYPE(MyBase)
DECLARE_STRUCT_TYPE(MyStruct)
DECLARE_STRUCT_TYPE(MyNamespace::MyTemplateStruct<MyStruct>)