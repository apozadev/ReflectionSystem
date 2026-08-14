#include "Reflection.h"

#include <array>

#include <cstdio>

int main()
{

  for (auto entry : GetTypeRegistry())
  {
    printf("%s\n", entry.first.data());
  }
  const TypeDesc_Collection* arrayType = static_cast<const TypeDesc_Collection*>(GetTypeDescByName("std::array<int,10>"));

  void* obj = malloc(arrayType->size);

  for (int i = 0; i < arrayType->getSize(obj); i++)
  {
    arrayType->setElement(obj, i, &i);
  }

  for(int i = 0; i < arrayType->getSize(obj); i++)
  {
    printf("%d\n", *static_cast<const int*>(arrayType->getElement(obj, i)));
  }

  if (arrayType->isResizable)
  {
    arrayType->resize(obj, 5);
  }

  return 0;
}
