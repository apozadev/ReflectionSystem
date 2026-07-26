#include "Reflection.h"

#pragma section("reflection$a", read) // Struct: start
#pragma section("reflection$c", read) // Struct: end
#pragma section("reflection$d", read) // Primitive: start
#pragma section("reflection$f", read) // Primitive: end
#pragma section("reflection$g", read) // Collection: start
#pragma section("reflection$j", read) // Collection: end

__declspec(allocate("reflection$a"))
const TypeDesc_Struct StructReflectionStart = {};

__declspec(allocate("reflection$c"))
const TypeDesc_Struct StructReflectionEnd = {};

__declspec(allocate("reflection$d"))
const TypeDesc_Primitive PrimitiveReflectionStart = {};

__declspec(allocate("reflection$f"))
const TypeDesc_Primitive PrimitiveReflectionEnd = {};

__declspec(allocate("reflection$g"))
const TypeDesc_Collection CollectionReflectionStart = {};

__declspec(allocate("reflection$j"))
const TypeDesc_Collection CollectionReflectionEnd = {};