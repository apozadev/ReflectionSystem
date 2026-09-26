#include "Reflection.h"

#pragma section("reflection$a", read) // Struct: start
#pragma section("reflection$c", read) // Struct: end
#pragma section("reflection$d", read) // Primitive: start
#pragma section("reflection$f", read) // Primitive: end
#pragma section("reflection$g", read) // Collection: start
#pragma section("reflection$j", read) // Collection: end

__REFLECTION_SECTION("reflection$a")
const TypeDesc_Struct StructReflectionStart = {};

__REFLECTION_SECTION("reflection$c")
const TypeDesc_Struct StructReflectionEnd = {};

__REFLECTION_SECTION("reflection$d")
const TypeDesc_Primitive PrimitiveReflectionStart = {};

__REFLECTION_SECTION("reflection$f")
const TypeDesc_Primitive PrimitiveReflectionEnd = {};

__REFLECTION_SECTION("reflection$g")
const TypeDesc_Collection CollectionReflectionStart = {};

__REFLECTION_SECTION("reflection$j")
const TypeDesc_Collection CollectionReflectionEnd = {};