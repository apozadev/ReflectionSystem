# ReflectionSystem

A small C++20 experimentation project that demonstrates a lightweight compile-time reflection system for structs, primitives, and collections.

## What this project does

This codebase provides a minimal reflection framework that:

- registers type descriptors for primitive types
- registers reflection metadata for struct/class types and their members
- registers reflection metadata for collection-like types
- exposes a type registry that can be queried by name

It has some serious limitations:

- Only public class members can be registered (in order to keep it non-intrusive)
- Incremental linking must be disabled
- All registered types must be default-constructible

The sample application prints the registered type names and demonstrates how the reflection metadata is exposed.

## Project structure

- Reflection.h: core reflection definitions, type concepts, and macros.
- Reflection.cpp: defines the sentinel objects that delimit the registered reflection sections.
- CollectionTraits.h: traits used to describe collection behavior.
- TypeName.h: Utility to get type name strings built at compile time.
- MacroUtils.h: Supporting macro helpers.
- CRC.h: Generate CRC from strings at compile time. Currently unused.
- App.cpp: Example app. Entry point that prints registered type names.
- MyStruct.h / MyStruct.cpp: example types and reflection declarations/implementations.

## Build and run

This project is configured as a Visual Studio C++ console application targeting x64 with C++20.

### Prerequisites

- Microsoft Visual Studio 2022 with C++ workload
- Windows SDK
- C++20-capable compiler toolset (the project uses v143)
