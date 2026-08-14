#pragma once

#include <array>
#include <string_view>

template<typename T>
struct TypeNameStorage
{
private:
  static constexpr std::string_view Sig()
  {
    return __FUNCSIG__;
  }

  static constexpr std::pair<size_t, size_t> Bounds()
  {
    std::string_view sig = Sig();

#if defined(_MSC_VER)
    constexpr std::string_view prefixes[] = {
            "TypeNameStorage<class ",
            "TypeNameStorage<struct ",
            "TypeNameStorage<enum ",
            "TypeNameStorage<union ",
            "TypeNameStorage<"
    };

    constexpr std::string_view suffix = ">::Sig(void)";
#elif defined(__clang__)
    constexpr std::string_view prefixes[] = {
        "[T = "
    };

    constexpr std::string_view suffix = "]";
#elif defined(__GNUC__)
    constexpr std::string_view prefixes[] = {
        "T = "
    };

    constexpr std::string_view suffix = ";";
#endif

    size_t begin = std::string_view::npos;
    for (auto p : prefixes)
    {
      size_t pos = sig.find(p);
      if (pos != std::string_view::npos)
      {
        begin = pos + p.size();
        break;
      }
    }

    //static_assert(begin != std::string_view::npos, "Type name not found in signature");

    size_t end = sig.rfind(suffix);

    // Trim whitespaces from beggining and end of the type name
    while (begin < end && sig[begin] == ' ')
    {
      begin++;
    }
    while (end > begin && sig[end-1] == ' ')
    {
      end--;
    }

    return { begin, end };
  }

  static constexpr auto Create()
  {
    constexpr auto bounds = Bounds();

    constexpr size_t begin = bounds.first;
    constexpr size_t end = bounds.second;
    constexpr size_t size = end - begin;

    std::string_view sig = Sig();

    std::array<char, size + 1> result{};
    for (size_t i = 0; i < size; i++)
    {
      result[i] = sig[begin + i];
    }
    result[size] = '\0';
    return result;
  }

public:
  inline static constexpr auto name = Create();
};

template<typename T>
constexpr const char* GetTypeName()
{
  return TypeNameStorage<T>::name.data();
}
