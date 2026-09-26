#pragma once

template <typename T>
struct Traits
{
  // Self-calling lambda returning a function pointer 
  // that calls the constructor T if it is default constructible, otherwise returns nullptr.
  inline static constexpr void(*defaultConstructor)(void*) = []() -> void(*)(void*)
    {
      if constexpr (std::is_default_constructible_v<T>)
      {
        return [](void* obj)-> void { new (obj) T(); };
      }
      else
      {
        return nullptr;
      }
    }();

  // Just a lambda calling destructor of T if it is not trivially destructible, otherwise does nothing.
  inline static constexpr void (*destructor)(void*) = [](void* obj)
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
      {
        static_cast<T*>(obj)->~T();
      }
      else
      {
        (void)obj;
      }
    };
};
