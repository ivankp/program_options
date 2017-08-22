#ifndef IVANP_TYPE_TRAITS_HH
#define IVANP_TYPE_TRAITS_HH

#include "meta.hh"

namespace ivanp {

template <typename T> struct rm_rref { using type = T; };
template <typename T> struct rm_rref<T&&> { using type = T; };
template <typename T> using rm_rref_t = typename rm_rref<T>::type;

template <typename T, typename... Args> // T(Args...)
class is_callable {
  template <typename, typename = void> struct impl: std::false_type { };
  template <typename U> struct impl<U,
    void_t<decltype( std::declval<U&>()(std::declval<Args>()...) )>
  > : std::true_type { };
public:
  using type = impl<T>;
  static constexpr bool value = type::value;
};

template <typename T, typename Arg> // T = Arg
class is_assignable {
  template <typename, typename = void> struct impl: std::false_type { };
  template <typename U> struct impl<U,
    void_t<decltype( std::declval<U&>() = std::declval<Arg>() )>
  > : std::true_type { };
public:
  using type = impl<T>;
  static constexpr bool value = type::value;
};

template <typename T> using value_type = typename T::value_type;

template <typename T> struct is_tuple: std::false_type { };
template <typename... T> struct is_tuple<std::tuple<T...>>: std::true_type { };

template <typename T> struct is_std_array: std::false_type { };
template <typename T, size_t N>
struct is_std_array<std::array<T,N>>: std::true_type { };

template <typename T> struct is_std_vector: std::false_type { };
template <typename T, typename Alloc>
struct is_std_vector<std::vector<T,Alloc>>: std::true_type { };

#ifdef __cpp_variable_templates
template <typename T, typename... Args>
constexpr bool is_callable_v = is_callable<T,Args...>::value;

template <typename T, typename... Args>
constexpr bool is_assignable_v = is_assignable<T,Args...>::value;

template <typename T>
constexpr bool has_value_type = is_detected<value_type,T>::value;

template <typename T> constexpr bool is_tuple_v = is_tuple<T>::value;
template <typename T> constexpr bool is_std_array_v = is_std_array<T>::value;
template <typename T> constexpr bool is_std_vector_v = is_std_vector<T>::value;
#endif

} // end namespace ivanp

#endif

