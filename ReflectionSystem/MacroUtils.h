#pragma once

#define _CAT2(a, b) a##b
#define CAT(a, b) _CAT2(a, b)

#define EXPAND(...) __VA_ARGS__
#define REMOVE_PARENS(...) EXPAND __VA_ARGS__

// Deferred-expansion recursive FOR_EACH (C++20, requires __VA_OPT__)
#define _PARENS ()

#define _EXPAND(...)  _EXPAND4(_EXPAND4(_EXPAND4(_EXPAND4(__VA_ARGS__))))
#define _EXPAND4(...) _EXPAND3(_EXPAND3(_EXPAND3(_EXPAND3(__VA_ARGS__))))
#define _EXPAND3(...) _EXPAND2(_EXPAND2(_EXPAND2(_EXPAND2(__VA_ARGS__))))
#define _EXPAND2(...) _EXPAND1(_EXPAND1(_EXPAND1(_EXPAND1(__VA_ARGS__))))
#define _EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) \
    __VA_OPT__(_EXPAND(_FOR_EACH_HELPER(macro, __VA_ARGS__)))

#define _FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1) \
    __VA_OPT__(_FOR_EACH_AGAIN _PARENS (macro, __VA_ARGS__))

#define _FOR_EACH_AGAIN() _FOR_EACH_HELPER
