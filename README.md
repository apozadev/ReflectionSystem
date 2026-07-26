# ReflectionSystem

A small C++20 experimentation project that demonstrates a lightweight compile-time reflection system for structs, primitives, and collections.

## What this project does

This codebase provides a minimal reflection framework that:

- registers type descriptors for primitive types
- registers reflection metadata for struct/class types and their members
- registers reflection metadata for collection-like types
- exposes a type registry that can be queried by name

The sample application prints the registered type names and demonstrates how the reflection metadata is exposed.

## Project structure

- App.cpp: entry point that prints registered type names.
- Reflection.h: core reflection definitions, type concepts, and macro helpers.
- Reflection.cpp: defines the sentinel objects that delimit the registered reflection sections.
- MyStruct.h / MyStruct.cpp: example types and reflection declarations/implementations.
- CollectionTraits.h: traits used to describe collection behavior.
- MacroUtils.h / ReflectionMacrosDecl.h / TypeName.h: supporting helpers and type-name utilities.
- ReflectionSystem.vcxproj: Visual Studio C++ project file.

## Build and run

This project is configured as a Visual Studio C++ console application targeting x64 with C++20.

### Prerequisites

- Microsoft Visual Studio 2022 with C++ workload
- Windows SDK
- C++20-capable compiler toolset (the project uses v143)
