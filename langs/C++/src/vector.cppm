module;

#include <concepts>
#include <type_traits>

export module vector;

namespace pbf {

export template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

export template <std::integral auto N, arithmetic T>
  requires(N > 0)
class Vector {};

} // namespace pbf
