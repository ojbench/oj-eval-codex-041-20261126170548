#ifndef SJTU_UTILITY_HPP
#define SJTU_UTILITY_HPP
#include <utility>
namespace sjtu {
template<class T1, class T2> class pair {
public:
    T1 first;
    T2 second;
    constexpr pair() = default;
    constexpr pair(const pair &) = default;
    constexpr pair(pair &&) = default;
    template<class U1 = T1, class U2 = T2>
    constexpr pair(U1 &&x, U2 &&y)
        : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}
    template<class U1, class U2>
    constexpr pair(const pair<U1, U2> &x) : first(x.first), second(x.second) {}
    template<class U1, class U2>
    constexpr pair(pair<U1, U2> &&x)
        : first(std::forward<U1>(x.first)), second(std::forward<U2>(x.second)) {}
};
#if __cplusplus >= 201703L
template<class T1, class T2> pair(T1, T2) -> pair<T1, T2>;
#endif
}
#endif
