module;

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <numeric>
#include <utility>

export module vector;

namespace pbf {

export template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

export template <arithmetic T, std::size_t N>
  requires(N > 0)
class NormalizedVector;

export template <arithmetic T, std::size_t N>
  requires(N > 0)
class Vector {
protected:
  std::array<T, N> data{};

public:
  enum class Error : std::uint8_t { ZeroLength };

  constexpr Vector() = default;

  template <typename... Args>
    requires(sizeof...(Args) == N && (std::convertible_to<Args, T> && ...))
  explicit(N == 1) constexpr Vector(Args... args)
      : data{static_cast<T>(args)...} {}

  template <typename Self>
  constexpr decltype(auto) operator[](this Self &&self,
                                      const std::size_t index) {
    return std::forward<Self>(self).data[index];
  }

  constexpr Vector &operator+=(const Vector &other) noexcept {
    std::ranges::transform(data, other.data, data.begin(), std::plus<>{});
    return *this;
  }

  constexpr Vector &operator-=(const Vector &other) noexcept {
    std::ranges::transform(data, other.data, data.begin(), std::minus<>{});
    return *this;
  }

  constexpr Vector &operator*=(T scalar) noexcept {
    std::ranges::for_each(data, [scalar](T &value) { value *= scalar; });
    return *this;
  }

  constexpr Vector &operator/=(T scalar) noexcept {
    std::ranges::for_each(data, [scalar](T &value) { value /= scalar; });
    return *this;
  }

  [[nodiscard]] constexpr Vector operator-() const noexcept {
    Vector res;
    std::ranges::transform(data, res.data.begin(), std::negate<>{});
    return res;
  }

  [[nodiscard]] friend constexpr Vector operator+(Vector left,
                                                  const Vector &right) {
    left += right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator-(Vector left,
                                                  const Vector &right) {
    left -= right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator*(Vector left,
                                                  T right) noexcept {
    left *= right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator*(T left,
                                                  Vector right) noexcept {
    right *= left;
    return right;
  }

  [[nodiscard]] friend constexpr Vector operator/(Vector left,
                                                  T right) noexcept {
    left /= right;
    return left;
  }

  [[nodiscard]] friend constexpr auto Dot(const Vector &left,
                                          const Vector &right) noexcept {
    using Promoted = std::conditional_t<std::floating_point<T>, T, double>;
    return std::transform_reduce(left.data.begin(), left.data.end(),
                                 right.data.begin(), Promoted{}, std::plus<>{},
                                 std::multiplies<>{});
  }

  [[nodiscard]] friend constexpr auto Cross(const Vector &left,
                                            const Vector &right) noexcept {
    return Vector{left[1] * right[2] - left[2] * right[1],
                  left[2] * right[0] - left[0] * right[2],
                  left[0] * right[1] - left[1] * right[0]};
  }

  [[nodiscard]] auto Length() const noexcept {
    using Promoted = std::conditional_t<std::floating_point<T>, T, double>;
    Promoted sum{};
    std::ranges::for_each(data, [&](const T value) {
      Promoted promoted = static_cast<Promoted>(value);
      sum += promoted * promoted;
    });
    return std::sqrt(sum);
  }

  [[nodiscard]] std::expected<NormalizedVector<T, N>, Error> Normalize() const;
};

export template <arithmetic T, std::size_t N>
  requires(N > 0)
class NormalizedVector : public Vector<T, N> {
  friend class Vector<T, N>;

  explicit constexpr NormalizedVector(Vector<T, N> &&other) noexcept
      : Vector<T, N>(std::move(other)) {}

public:
  constexpr NormalizedVector() {
    this->data.fill(T{0});
    this->data[0] = T{1};
  }

  explicit NormalizedVector(const Vector<T, N> &other)
      : NormalizedVector(other.Normalize().value_or(NormalizedVector{})) {}

  [[nodiscard]] constexpr const T &
  operator[](const std::size_t index) const noexcept {
    return Vector<T, N>::operator[](index);
  }

  constexpr Vector<T, N> &operator+=(const Vector<T, N> &) = delete;
  constexpr Vector<T, N> &operator-=(const Vector<T, N> &) = delete;

  [[nodiscard]] const NormalizedVector &Normalize() const noexcept {
    return *this;
  }
};

template <arithmetic T, std::size_t N>
  requires(N > 0)
std::expected<NormalizedVector<T, N>, typename Vector<T, N>::Error>
Vector<T, N>::Normalize() const {
  auto length = Length();
  using Promoted = decltype(length);
  if (length == Promoted{})
    return std::unexpected{Error::ZeroLength};

  [[assume(length != Promoted{})]];
  Vector<T, N> other;
  std::ranges::transform(data, other.data.begin(), [&](const T value) {
    return static_cast<T>(static_cast<Promoted>(value) / length);
  });
  return NormalizedVector<T, N>{std::move(other)};
}

export using Vec3u = Vector<std::size_t, 3>;
export using Vec3f = Vector<float, 3>;
export using NVec3f = NormalizedVector<float, 3>;

} // namespace pbf
