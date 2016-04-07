#include <array>
#include <string>
#include <iostream>

#include "pp_repeat.hpp"

////////////////////////////////////////////////////////////////
// Define the literal string

template <char...>
struct literal_string {};

////////////////////////////////////////////////////////////////
// Operations

/* insert */

template <bool, char C, typename LS>
struct insert_;

template <char C, char... S>
struct insert_<false, C, literal_string<S...>>
{
    using type = literal_string<C, S...>;
};

template <char C, char... S>
struct insert_<true, C, literal_string<S...>>
{
    using type = literal_string<S...>;
};

template <char C, char... S>
constexpr auto insert(literal_string<S...>) noexcept
{
    return typename insert_<false, C, literal_string<S...>>::type{};
}

template <bool Ignored, char C, char... S>
constexpr auto insert(literal_string<S...>) noexcept
{
    return typename insert_<Ignored, C, literal_string<S...>>::type{};
}

/* remove */

template <char V>
constexpr auto remove(literal_string<>) noexcept
{
    return literal_string<>{};
}

template <char V, char C, char... S>
constexpr auto remove(literal_string<C, S...>) noexcept
{
    return insert<V == C, C>(remove<V>(literal_string<S...>{}));
}

/* replace */

template <char V1, char V2>
constexpr auto replace(literal_string<>) noexcept
{
    return literal_string<>{};
}

template <char V1, char V2, char C, char... S>
constexpr auto replace(literal_string<C, S...>) noexcept
{
    return insert<(V1 == C) ? V2 : C>(replace<V1, V2>(literal_string<S...>{}));
}

/* count */

template <char V>
constexpr size_t count(literal_string<>) noexcept
{
    return 0;
}

template <char V, char C, char... S>
constexpr size_t count(literal_string<C, S...>) noexcept
{
    return ((V == C) ? 1 : 0) + count<V>(literal_string<S...>{});
}

/* find */

constexpr size_t npos = static_cast<size_t>(-1);

template <char V, size_t N = 0>
constexpr size_t find(literal_string<>) noexcept
{
    return npos;
}

template <char V, size_t N = 0, char C, char... S>
constexpr size_t find(literal_string<C, S...>) noexcept
{
    return (V == C) ? N : find<V, N + 1>(literal_string<S...>{});
}

/* substr */

template <size_t B, size_t E = npos, size_t N = 0>
constexpr auto substr(literal_string<>) noexcept
{
    return literal_string<>{};
}

template <size_t B, size_t E = npos, size_t N = 0, char C, char... S>
constexpr auto substr(literal_string<C, S...>) noexcept
{
    return insert<(B > N) || (N >= E), C>(substr<B, E, N + 1>(literal_string<S...>{}));
}

/* at */

template <size_t N>
constexpr auto at(size_t n, const char(&str)[N]) noexcept
{
    return (n < N) ? str[n] : '\0';
}

////////////////////////////////////////////////////////////////
// Conversions

template <char... S>
constexpr auto string(literal_string<S...>) noexcept
{
    return std::string{ S... };
}

template <typename ArrT>
struct cast
{
    ArrT arr_;
    cast(const std::string& str) // Not in compile-time...
    {
        size_t i = 0, start = 0;
        while (start < str.size())
        {
            auto pos = str.find('\0', start);
            arr_[i++] = str.substr(start, pos - start);
            start = pos + 1;
        }
    }
};

template <char... S>
constexpr auto array(literal_string<S...>) noexcept
{
    return cast<std::array<std::string, count<'\0'>(literal_string<S...>{})>>{ string(literal_string<S...>{}) }.arr_;
}

////////////////////////////////////////////////////////////////
// Preprocessor

#define LITERAL_C(N, STR)       , at(N, STR)
#define LITERAL_S(STR)          substr<0, sizeof(STR)>(literal_string<at(0, STR) CAPO_PP_REPEAT_MAX_(LITERAL_C, STR)>{})
#define LITERAL_SPLIT(SEP, STR) array(replace<SEP, '\0'>(LITERAL_S(STR)))

////////////////////////////////////////////////////////////////

int main(void)
{
    auto arr = LITERAL_SPLIT(',', "1,2,3,4,5,6,7,8,9,0");
    for (auto& s: arr) std::cout << s << std::endl;
    return 0;
}
