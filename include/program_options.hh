#ifndef IVANP_PROGRAM_OPTIONS_HH
#define IVANP_PROGRAM_OPTIONS_HH

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <type_traits>
#include <stdexcept>

#define TEST(var) \
  std::cout <<"\033[36m"<< #var <<"\033[0m"<< " = " << var << std::endl;

#include "type.hh"
#include "type_traits.hh"
#include "catstr.hh"
#include "tuple_alg.hh"
#include "seq_alg.hh"

namespace ivanp { namespace po {
struct error : std::runtime_error {
  using std::runtime_error::runtime_error;
};
}}

#include "program_options/opt_match.hh"
#include "program_options/opt_def.hh"

namespace ivanp { namespace po {

class program_options {
  std::vector<std::unique_ptr<detail::opt_def>> opt_defs;
  std::array<std::vector<std::pair<
    std::unique_ptr<const detail::opt_match_base>,
    detail::opt_def*
  >>,3> matchers;

  template <typename T, typename... Props>
  inline auto* add_opt_def(T* x, std::string&& descr, Props&&... p) {
    static_assert( !std::is_const<T>::value,
      "\033[33mpointer to const in program option definition\033[0m");

    using props_types = std::tuple<std::decay_t<Props>...>;
    const auto props  = std::forward_as_tuple(std::forward<Props>(p)...);

#define UNIQUE_PROP_ASSERT(NAME) \
    using NAME##_i = get_indices_of_t<_::is_##NAME, props_types>; \
    static_assert( NAME##_i::size() <= 1, \
      "\033[33mrepeated \"" #NAME "\" in program option definition\033[0m");

    UNIQUE_PROP_ASSERT(name)
    UNIQUE_PROP_ASSERT(switch_init)
    UNIQUE_PROP_ASSERT(pos)
    UNIQUE_PROP_ASSERT(req)
    UNIQUE_PROP_ASSERT(multi)

#undef UNIQUE_PROP_ASSERT

    using parser_i = get_indices_of_t<
      _::is_parser<T>::template type, props_types>;
    static_assert( parser_i::size() <= 1,
      "\033[33mrepeated parser in program argument definition\033[0m");

    using prop_seq = seq_join_t<
      parser_i,
      name_i,
      switch_init_i,
      multi_i,
      pos_i,
      req_i
    >;

    static_assert( seq::size() == sizeof...(Props),
      "\033[33munrecognized argument in program option definition\033[0m");

    auto *opt = detail::make_opt_def(x, std::move(descr), props, prop_seq{});
    opt_defs.emplace_back(opt);

    using opt_t = std::decay_t<decltype(*opt)>; // TEST
    prt_type_size<opt_t>(); // TEST

    return opt;
  }

  template <typename Matcher>
  inline void add_opt_match(
    Matcher&& matcher, detail::opt_def* opt
  ) {
    auto&& m = detail::make_opt_match(std::forward<Matcher>(matcher));
    matchers[m.second].emplace_back(std::move(m.first),opt);
  }
  template <typename... M, size_t... I>
  inline void add_opt_matches(
    const std::tuple<M...>& matchers, detail::opt_def* opt,
    std::index_sequence<I...>
  ) {
#ifdef __cpp_fold_expressions
    (add_opt_match(std::get<I>(matchers),opt),...);
#else
    fold((add_opt_match(std::get<I>(matchers),opt),0)...);
#endif
  }

public:
  void parse(int argc, char const * const * argv);
  // void help(); // FIXME

  template <typename T, typename... Props>
  parser& operator()(T* x,
    std::initializer_list<const char*> matchers,
    std::string descr={}, Props&&... p
  ) {
    auto *opt = add_opt(x,std::move(descr),std::forward<Props>(p)...);
    for (const char* m : matchers) add_opt_match(m,opt);
    return *this;
  }

  template <typename T, typename Matcher, typename... Props>
  std::enable_if_t<!is_tuple<std::decay_t<Matcher>>::value,parser&>
  operator()(T* x,
    Matcher&& matcher,
    std::string descr={}, Props&&... p
  ) {
    auto *opt = add_opt(x,std::move(descr),std::forward<Props>(p)...);
    add_opt_match(matcher,opt);
    return *this;
  }

  template <typename T, typename... Matchers, typename... Props>
  parser& operator()(T* x,
    const std::tuple<Matchers...>& matchers,
    std::string descr={}, Props&&... p
  ) {
    static_assert( sizeof...(Matchers) > 0,
      "\033[33mempty tuple in program argument definition\033[0m");
    auto *opt = add_opt(x,std::move(descr),std::forward<Props>(p)...);
    add_opt_matches(matchers,opt,std::index_sequence_for<Matchers...>{});
    return *this;
  }
};

}} // end namespace ivanp

#endif
