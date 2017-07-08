#ifndef IVANP_TYPE_TRAITS_HH
#define IVANP_TYPE_TRAITS_HH

#include "meta.hh"

namespace ivanp {

template <typename T> struct rm_rref { using type = T; };
template <typename T> struct rm_rref<T&&> { using type = T; };
template <typename T> using rm_rref_t = typename rm_rref<T>::type;

template <typename F>
struct is_fcn_ptr: std::integral_constant<bool,
  std::is_pointer<F>::value
  && std::is_function<std::remove_pointer_t<F>>::value
> { };

template <typename T, typename... Args> // T(Args...)
class is_callable {
  template <typename, typename = void>
  struct impl: std::false_type { };
  template <typename U>
  struct impl<U,
    void_t<decltype( std::declval<U&>()(std::declval<Args>()...) )>
  > : std::true_type { };
public:
  using type = impl<T>;
  static constexpr bool value = type::value;
};

template <typename T> struct is_tuple: std::false_type { };
template <typename... T> struct is_tuple<std::tuple<T...>>: std::true_type { };

} // end namespace ivanp

#endif

