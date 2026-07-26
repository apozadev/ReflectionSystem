#include "Reflection.h"

#include <cstdio>

// TODO: add namespace to struct macro

int main()
{

  for (auto entry : GetTypeRegistry())
  {
    printf("%s\n", entry.first.data());
  }

  printf("%s\n", GetTypeName<std::array<int, 10>>());

  return 0;
}
