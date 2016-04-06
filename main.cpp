#include <array>
#include <iostream>

#include "pp_repeat.hpp"

template <char...>
struct literal_string {};

template <char... S>
constexpr auto array(literal_string<S...>) noexcept
{
    return std::array<char, sizeof...(S) + 1>{ S... };
}

template <bool = false>
struct insert
{
    template <char C, char... S>
    constexpr static auto _(literal_string<S...>) noexcept
    {
        return literal_string<C, S...>{};
    }
};

template <>
struct insert<true>
{
    template <char C, char... S>
    constexpr static auto _(literal_string<S...>) noexcept
    {
        return literal_string<S...>{};
    }
};

template <char V>
constexpr auto remove(literal_string<>) noexcept
{
    return literal_string<>{};
}

template <char V, char C, char... S>
constexpr auto remove(literal_string<C, S...>) noexcept
{
    return insert<V == C>::template _<C>(remove<V>(literal_string<S...>{}));
}

template <size_t N>
constexpr auto at(size_t n, const char(& str)[N]) noexcept
{
    return (n < N) ? str[n] : '\0';
}

template <size_t N, typename LS>
struct cut_;

template <>                  struct cut_<0, literal_string<>>        { using type = literal_string<>; };
template <char C, char... S> struct cut_<0, literal_string<C, S...>> { using type = literal_string<>; };
template <size_t N>          struct cut_<N, literal_string<>>        { using type = literal_string<>; };

template <size_t N, char C, char... S>
struct cut_<N, literal_string<C, S...>>
{
    using type = decltype(insert<>::template _<C>(typename cut_<N - 1, literal_string<S...>>::type{}));
};

template <size_t N, typename LS>
using cut = typename cut_<N, LS>::type;

#define LITERAL_C(N, STR)       , at(N, STR)
#define LITERAL_S(STR)          cut<sizeof(STR), literal_string<at(0, STR) CAPO_PP_REPEAT_MAX_(LITERAL_C, STR)>>{}
#define LITERAL_SPLIT(SEP, STR) array(remove<SEP>(LITERAL_S(STR)))

int main(void)
{
    auto arr = LITERAL_SPLIT(',', "1, 2, 3, 4, 5, 6, 7, 8, 9, 0");
    std::cout << arr.data() << std::endl;
    return 0;
}
