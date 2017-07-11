#ifndef IVANP_EMPLACE_HH
#define IVANP_EMPLACE_HH

#include "meta.hh"

namespace ivanp {

#define DEFINE_EMPLACE_TRAIT(EMPLACE) \
  template <typename T, typename... Args> using EMPLACE##_t = decltype( \
    std::declval<T&>().EMPLACE(std::declval<Args>()) ); \
  template <typename T, typename... Args> constexpr bool has_##EMPLACE \
    = is_detected_v<EMPLACE##_t,Args...>;

DEFINE_EMPLACE_TRAIT(emplace_back);
DEFINE_EMPLACE_TRAIT(emplace);
DEFINE_EMPLACE_TRAIT(emplace_front);

#undef DEFINE_EMPLACE_TRAIT

template <typename T, typename... Args>
constexpr bool can_emplace =
  has_emplace_back<T,Args...> ||
  has_emplace<T,Args...> ||
  has_emplace_front<T,Args...>;

template <typename T, typename... Args>
std::enable_if_t<
  has_emplace_back<T,Args...>,
  emplace_back_t<T,Args...>
> emplace(T& x, Args&&... args)
noexcept(noexcept(x.emplace_back(std::forward<Args>(args)...)))
{ x.emplace_back(std::forward<Args>(args)...); }

template <typename T, typename... Args>
std::enable_if_t<
  !has_emplace_back<T,Args...> &&
  has_emplace<T,Args...>,
  emplace_t<T,Args...>
> emplace(T& x, Args&&... args)
noexcept(noexcept(x.emplace(std::forward<Args>(args)...)))
{ x.emplace(std::forward<Args>(args)...); }

template <typename T, typename... Args>
std::enable_if_t<
  !has_emplace_back<T,Args...> &&
  !has_emplace<T,Args...> &&
  has_emplace_front<T,Args...>,
  emplace_t<T,Args...>
> emplace(T& x, Args&&... args)
noexcept(noexcept(x.emplace_front(std::forward<Args>(args)...)))
{ x.emplace_front(std::forward<Args>(args)...); }

}

#define
