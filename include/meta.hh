#ifndef IVANP_META_HH
#define IVANP_META_HH

namespace ivanp {

// void_t
template <typename... T> struct make_void { typedef void type; };
template <typename... T> using void_t = typename make_void<T...>::type;

// allows to emulate comma fold expressions
template <typename... Args> constexpr void fold(Args...) noexcept { };

// Maybe ============================================================
struct nothing { };
template <typename T> struct just { using type = T; };
template <typename T> struct maybe;
template <> struct maybe<nothing> { using type = nothing; };
template <typename T> struct maybe<just<T>> { using type = just<T>; };
template <typename T> struct is_nothing: std::false_type { };
template <> struct is_nothing<maybe<nothing>>: std::true_type { };
template <typename T> struct is_just: std::true_type { };
template <> struct is_just<maybe<nothing>>: std::false_type { };
template <typename T> struct extract;
template <typename T> struct extract<maybe<just<T>>> { using type = T; };
template <typename T> using extract_t = typename extract<T>::type;

// enable
template <typename M, typename T = void> struct enable_if_just;
template <typename J, typename T>
struct enable_if_just<just<J>> { using type = T };
template <typename N, typename T>
struct enable_if_just<nothing<N>> { };

template <typename M, typename T = void> struct enable_if_nothing;
template <typename N, typename T>
struct enable_if_nothing<nothing<N>> { using type = T };
template <typename J, typename T>
struct enable_if_nothing<just<J>> { };

template <typename M, typename T>
using enable_if_just_t = typename enable_if_just<M,T>::type;
template <typename M, typename T>
using enable_if_nothing_t = typename enable_if_nothing<M,T>::type;

// Find first =======================================================
// first pack element matching predicate
template <template<typename> typename Pred, typename... Args>
struct find_first: maybe<nothing> { };

template <template<typename> typename Pred, typename Arg1, typename... Args>
class find_first<Pred,Arg1,Args...> {
  template <typename, typename = void>
  struct impl: find_first_impl<Pred,Args...> { };
  template <typename Arg>
  struct impl<Arg,std::enable_if_t<Pred<Arg>::value>>: maybe<just<Arg>> { };
public:
  using type = typename impl<Arg1>::type;
};

template <template<typename> typename Pred, typename... Args>
using find_first_t = typename find_first<Pred,Args...>::type;

} // end namespace ivanp

#endif
