#include "MyStruct.h"

#include <vector>

IMPLEMENT_PRIMITIVE_TYPE(int)

IMPLEMENT_PRIMITIVE_TYPE(char)

IMPLEMENT_COLLECTION_TYPE((char[5]))

IMPLEMENT_COLLECTION_TYPE((std::array<int, 10>))

IMPLEMENT_STRUCT_TYPE((MyBase), (), (x))

IMPLEMENT_STRUCT_TYPE((MyStruct), (MyBase), (a, b, c))

IMPLEMENT_COLLECTION_TYPE((std::vector<MyStruct>))

IMPLEMENT_STRUCT_TYPE((MyNamespace::MyTemplateStruct<MyStruct>), (), (a, b))