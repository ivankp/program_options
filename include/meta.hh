#ifndef IVANP_META_HH
#define IVANP_META_HH

#include <type_traits>

namespace ivanp {

// void_t
template <typename... T> struct make_void { typedef void type; };
template <typename... T> using void_t = typename make_void<T...>::type;

// allows to emulate comma fold expressions
template <typename... Args> constexpr void fold(Args...) noexcept { };

// bool const
template <bool B> using bool_constant = std::integral_constant<bool, B>;

// curry
template <template<typename,typename> typename Pred, typename T1>
struct bind_first {
  template <typename T2> using type = Pred<T1,T2>;
};
template <template<typename,typename> typename Pred, typename T2>
struct bind_second {
  template <typename T1> using type = Pred<T1,T2>;
};

// list type from monadic context
template <typename T> using extract = typename T::type;

// Maybe ============================================================
struct nothing { };
template <typename T> struct just { using type = T; };
template <typename T> struct maybe;
template <> struct maybe<nothing> { using type = nothing; };
template <typename T> struct maybe<just<T>> { using type = just<T>; };

template <typename T> struct is_nothing;
template <> struct is_nothing<nothing>: std::true_type { };
template <typename T> struct is_nothing<just<T>>: std::false_type { };

template <typename T> struct is_just;
template <> struct is_just<nothing>: std::false_type { };
template <typename T> struct is_just<just<T>>: std::true_type { };

// enable
template <typename M, typename T = void>
using enable_if_just_t = std::enable_if_t<is_just<M>::value,T>;
template <typename M, typename T = void>
using enable_if_nothing_t = std::enable_if_t<is_nothing<M>::value,T>;

// is
template <template<typename> typename Pred, typename M, bool N = false>
struct maybe_is;
template <template<typename> typename Pred, bool N>
struct maybe_is<Pred,nothing,N>: bool_constant<N> { };
template <template<typename> typename Pred, typename T, bool N>
struct maybe_is<Pred,just<T>,N>: bool_constant<Pred<T>::value> { };

// First in pack ====================================================
template <typename...> struct first: maybe<nothing> { };
template <typename T, typename... Other>
struct first<T,Other...>: maybe<just<T>> { };
template <typename... Args> using first_t = typename first<Args...>::type;

// Find first =======================================================
// first pack element matching predicate
template <template<typename> typename Pred, typename... Args>
struct find_first: maybe<nothing> { };

template <template<typename> typename Pred, typename Arg1, typename... Args>
class find_first<Pred,Arg1,Args...> {
  template <typename, typename = void>
  struct impl: find_first<Pred,Args...> { };
  template <typename Arg>
  struct impl<Arg,std::enable_if_t<Pred<Arg>::value>>: maybe<just<Arg>> { };
public:
  using type = typename impl<Arg1>::type;
};

template <template<typename> typename Pred, typename... Args>
using find_first_t = typename find_first<Pred,Args...>::type;

} // end namespace ivanp

#endif
