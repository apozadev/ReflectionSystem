#include "Reflection.h"
#include "MyStruct.h"
#include <array>
#include <cstdio>

// TODO: static assert for base types. Check they are (not the same as __Type) and (not derived) to avoid infinite loop at compile-time

int main()
{  

  // Iterate through all registered types and print their names
  for (auto entry : TypeRegistry::Get().GetTypes())
  {
    printf("%s\n", entry.first.data());
  }

  // Retrieve the type descriptor for std::vector<MyStruct> from the type registry
  const TypeDesc_Collection* arrayType = static_cast<const TypeDesc_Collection*>(TypeRegistry::Get().GetTypeDescByName("std::vector<struct MyStruct,class std::allocator<struct MyStruct> >"));

  // Allocate memory for a vector object
  void* obj = malloc(arrayType->size);

  // Construct the vector object
  arrayType->construct(obj);

  // Populate the vector with integer elements
  for (int i = 0; i < arrayType->getSize(obj); i++)
  {
    arrayType->setElement(obj, i, &i);
  }

  // Retrieve and print each element from the vector
  for(int i = 0; i < arrayType->getSize(obj); i++)
  {
    printf("%d\n", *static_cast<const int*>(arrayType->getElement(obj, i)));
  }

  // Resize the vector if it supports dynamic resizing
  if (arrayType->resize)
  {
    arrayType->resize(obj, 5);
  }

  return 0;
}
