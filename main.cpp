#include <array>
#include <string>
#include <iostream>
#include <type_traits>

#include "pp_repeat.hpp"
#include "type_list.hpp"
#include "stopwatch.hpp"

////////////////////////////////////////////////////////////////
// Define the literal string & helper types

template <char...>
struct literal_string { static const char to_string[]; };

template <char... S>
const char literal_string<S...>::to_string[] = { S... };

template <class... LS>
struct literal_array { static const std::array<std::string, sizeof...(LS)> to_array; };

template <class... LS>
const std::array<std::string, sizeof...(LS)> literal_array<LS...>::to_array = { LS::to_string... };

////////////////////////////////////////////////////////////////
// Operations

/* insert */

template <bool Valid, char C, char... S>
constexpr auto insert(literal_string<S...>) noexcept
{
    return typename std::conditional<Valid, literal_string<C, S...>, literal_string<S...>>::type{};
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
    return insert<V != C, C>(remove<V>(literal_string<S...>{}));
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
    return insert<true, (V1 == C) ? V2 : C>(replace<V1, V2>(literal_string<S...>{}));
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
    return insert<(B <= N) && (N < E), C>(substr<B, E, N + 1>(literal_string<S...>{}));
}

/* at */

template <size_t N>
constexpr auto at(size_t n, const char(&str)[N]) noexcept
{
    return (n < N) ? str[n] : '\0';
}

// split

#if defined(_MSC_VER)
template <char V, class LS> struct find_ { enum: size_t { value = find<V>(LS{}) }; };
template <char Sep, class LS, size_t = find_<Sep, LS>::value>
#else
template <char Sep, class LS, size_t = find<Sep>(LS{})>
#endif
struct split_;

template <char Sep>
struct split_<Sep, literal_string<>, npos>
{
    using type = literal_array<>;
};

template <char Sep, char... S>
struct split_<Sep, literal_string<S...>, npos>
{
    using type = literal_array<literal_string<S...>>;
};

template <char Sep, size_t Pos, char... S>
struct split_<Sep, literal_string<S...>, Pos>
{
    using head = decltype(substr< 0, Pos>(literal_string<S...>{}));
    using tail = decltype(substr<Pos + 1>(literal_string<S...>{}));
    using type = capo::types_link_t<literal_array<head>, typename split_<Sep, tail>::type>;
};

template <char Sep, char... S>
constexpr auto split(literal_string<S...>) noexcept
{
    return typename split_<Sep, literal_string<S...>>::type{};
}

////////////////////////////////////////////////////////////////
// Preprocessor

#define LITERAL_C(N, STR)       , at(N, STR)
#define LITERAL_S(STR)          substr<0, sizeof(STR)>(literal_string<at(0, STR) CAPO_PP_REPEAT_MAX_(LITERAL_C, STR)>{})
#define LITERAL_SPLIT(SEP, STR) decltype(split<SEP>(LITERAL_S(STR)))::to_array

////////////////////////////////////////////////////////////////

template <unsigned N>
std::array<std::string, N> runtime_split(const char delimiter, const std::string& s)
{
    size_t start = 0;
    size_t end = s.find_first_of(delimiter);

    std::array<std::string, N> output;

    size_t i = 0;
    while (end <= std::string::npos)
    {
        output[i++] = std::move(s.substr(start, end - start));
        if (end == std::string::npos)
            break;

        start = end + 2;
        end = s.find_first_of(delimiter, start);
    }

    return output;
}

int main(void)
{
    std::array<std::string, 10> arr;
    constexpr size_t loop = 10000000;
    capo::stopwatch<> sw;

    sw.start();
    for (size_t i = 0; i < loop; ++i)
    {
        arr = LITERAL_SPLIT(',', "1,2,3,4,5,6,7,8,9,0");
    }
    std::cout << sw.elapsed<std::chrono::milliseconds>() << "ms" << std::endl;

    sw.start();
    for (size_t i = 0; i < loop; ++i)
    {
        arr = runtime_split<10>(',', "1,2,3,4,5,6,7,8,9,0");
    }
    std::cout << sw.elapsed<std::chrono::milliseconds>() << "ms" << std::endl;

    return 0;
}
