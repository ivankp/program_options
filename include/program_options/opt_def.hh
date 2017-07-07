#ifndef IVANP_OPT_DEF_HH
#define IVANP_OPT_DEF_HH

#include "program_options/opt_parser.hh"

namespace ivanp { namespace po {

// Opt def props ---------------------------------------------------

namespace _ {

struct name { std::string name; };
template <typename T> struct is_name : std::false_type { };
template <> struct is_name<name> : std::true_type { };

struct multi { };
template <typename T> struct is_multi : std::false_type { };
template <> struct is_multi<multi> : std::true_type { };

struct pos { };
template <typename T> struct is_pos : std::false_type { };
template <> struct is_pos<pos> : std::true_type { };

struct npos { unsigned npos; };
template <typename T> struct is_npos : std::false_type { };
template <> struct is_npos<npos> : std::true_type { };

struct req { };
template <typename T> struct is_req : std::false_type { };
template <> struct is_req<req> : std::true_type { };

template <typename T> struct is_parser {
  template <typename F>
  using type = typename is_callable<F,const char*,T&>::type;
};
template <typename T> struct is_parser<const T>: std::false_type { };

template <typename... Args> class switch_init {
  mutable std::tuple<Args...> args;
  template <typename T, size_t... I>
  inline void construct(T& x, std::index_sequence<I...>) const {
    x = { std::get<I>(args)... };
  }
public:
  template <typename... T> // TODO: contruct tuple more directly
  switch_init(std::tuple<T...>&& tup): args(std::move(tup)) { }
  template <typename T> inline void construct(T& x) const {
    construct(x,std::index_sequence_for<Args...>{});
  }
};
template <> struct switch_init<> {
  template <typename T> inline void operator()(T& x) const { x = { }; }
};
template <typename T> struct is_switch_init : std::false_type { };
template <typename... T>
struct is_switch_init<switch_init<T...>> : std::true_type { };

} // end namespace _

template <typename... Args>
inline _::name name(Args&&... args) { return {{std::forward<Args>(args)...}}; }
constexpr _::multi multi() noexcept { return {}; }
constexpr _::pos   pos() noexcept { return {}; }
constexpr _::npos  pos(unsigned n) noexcept { return { n }; }
constexpr _::req   req() noexcept { return {}; }
template <typename... Args>
inline _::switch_init<std::decay_t<Args>...> switch_init(Args&&... args) {
  return { std::forward_as_tuple(std::forward<Args>(args)...) };
}

namespace detail {

// Option definition objects ----------------------------------------
// These provide common interface for invoking argument parsers
// and assigning new values to recepients via pointers
// These are created as a result of calling program_options::operator()

struct opt_def {
  std::string descr;
  unsigned count = 0;

  opt_def(std::string&& descr): descr(std::move(descr)) { }
  virtual ~opt_def() { }
  virtual std::string name() const = 0;
  virtual void parse(const char* arg) = 0;
  virtual void as_switch() = 0;

  virtual bool is_switch() const noexcept = 0;
  virtual bool is_multi() const noexcept = 0;
  virtual bool is_pos() const noexcept = 0;
  virtual bool is_pos_end() const noexcept = 0;
  virtual bool is_req() const noexcept = 0;
  virtual bool is_signed() const noexcept = 0;
};

template <typename T, typename... Props>
class opt_def_impl final: public opt_def, Props... {
  T *x; // recepient of parsed value

public:
  using type = std::decay_t<T>;

#define OPT_PROP_TYPE(NAME) \
  using NAME##_t = find_first_t<_::is_##NAME,Props...>;

  OPT_PROP_TYPE(name)
  OPT_PROP_TYPE(switch_init)
  OPT_PROP_TYPE(pos)
  OPT_PROP_TYPE(npos)
  OPT_PROP_TYPE(req)
  OPT_PROP_TYPE(multi)

#undef OPT_PROP_TYPE

  using parser_t = find_first_t<_::is_parser<T>::template type,Props...>;

private:
  // parse ----------------------------------------------------------
  template <typename U = parser_t> inline enable_if_just_t<U>
  parse_impl(const char* arg) const { extract<parser_t>::operator()(arg,*x); }
  template <typename U = parser_t> inline enable_if_nothing_t<U>
  parse_impl(const char* arg) const { arg_parser<T>()(arg,*x); }

  // switch ---------------------------------------------------------
  template <typename U = switch_init_t> inline enable_if_just_t<U>
  as_switch_impl() const { extract<U>::construct(*x); }
  template <typename U = switch_init_t> inline std::enable_if_t<
    is_nothing<U>::value && std::is_same<type,bool>::value>
  as_switch_impl() const { (*x) = true; }
  template <typename U = switch_init_t> [[noreturn]]
  inline std::enable_if_t<
    is_nothing<U>::value && !std::is_same<type,bool>::value>
  as_switch_impl() const { throw error(name() + " without value"); }

  // name -----------------------------------------------------------
  template <typename U = name_t> inline enable_if_just_t<U,std::string>
  name_impl() const { return extract<U>::name; }
  template <typename U = name_t> inline enable_if_nothing_t<U,std::string>
  name_impl() const { return descr; } // FIXME

public:
  // ----------------------------------------------------------------
  template <typename... M>
  opt_def_impl(T* x, std::string&& descr, M&&... m)
  : opt_def(std::move(descr)), Props(std::forward<M>(m))..., x(x) { };

  inline std::string name() const { return name_impl(); }

  inline void parse(const char* arg) { parse_impl(arg); ++count; }
  inline void as_switch() { as_switch_impl(); ++count; }

  constexpr bool is_switch() const noexcept {
    return std::is_same<type,bool>::value;
  }

  constexpr bool is_multi() const noexcept {
    return is_just<multi_t>::value; // TODO: or T is a container
  }
  constexpr bool is_pos() const noexcept {
    return is_just<pos_t>::value || is_just<npos_t>::value;
  }
  constexpr bool is_pos_end() const noexcept { return is_just<pos_t>::value; }
  constexpr bool is_req() const noexcept { return is_just<req_t>::value; }
  constexpr bool is_signed() const noexcept {
    return std::is_signed<type>::value;
  }
};

// Factory ----------------------------------------------------------
template <typename T, typename Tuple, size_t... I>
inline auto make_opt_def(
  T* x, std::string&& descr, Tuple&& tup, std::index_sequence<I...>
) {
  using type = opt_def_impl<T,
    std::decay_t<std::tuple_element_t<I,std::decay_t<Tuple>>>... >;
  return new type( x, std::move(descr), std::get<I>(tup)... );
}

} // end namespace detail

}}

#endif
