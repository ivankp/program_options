#ifndef IVANP_META_HH
#define IVANP_META_HH

namespace ivanp {

template <typename... T> struct make_void { typedef void type; };
template <typename... T> using void_t = typename make_void<T...>::type;

// allows to emulate comma fold expressions
template <typename... Args> constexpr void fold(Args...) noexcept { };

} // end namespace ivanp

#endif
