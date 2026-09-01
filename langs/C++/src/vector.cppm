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
  explicit(N == 1) constexpr Vector(Args... args) : data{static_cast<T>(std::move(args))...} {}

  explicit constexpr Vector(T scalar)
    requires(N > 1)
  {
    data.fill(static_cast<T>(scalar));
  }

  template <typename Self>
  constexpr decltype(auto) operator[](this Self &&self, const std::size_t index) {
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

  constexpr Vector &operator*=(const Vector &other) noexcept {
    std::ranges::transform(data, other.data, data.begin(), std::multiplies<>{});
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

  [[nodiscard]] friend constexpr Vector operator+(Vector left, const Vector &right) noexcept {
    left += right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator-(Vector left, const Vector &right) noexcept {
    left -= right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator*(Vector left, T right) noexcept {
    left *= right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator*(T left, Vector right) noexcept {
    right *= left;
    return right;
  }

  [[nodiscard]] friend constexpr Vector operator*(Vector left, const Vector &right) noexcept {
    left *= right;
    return left;
  }

  [[nodiscard]] friend constexpr Vector operator/(Vector left, T right) noexcept {
    left /= right;
    return left;
  }

  [[nodiscard]] friend constexpr auto Dot(const Vector &left, const Vector &right) noexcept {
    using Promoted = std::conditional_t<std::floating_point<T>, T, double>;
    return std::transform_reduce(left.data.begin(), left.data.end(), right.data.begin(), Promoted{},
                                 std::plus<>{}, std::multiplies<>{});
  }

  [[nodiscard]] friend constexpr auto Cross(const Vector &left, const Vector &right) noexcept
    requires(N == 3)
  {
    return Vector{left[1] * right[2] - left[2] * right[1], left[2] * right[0] - left[0] * right[2],
                  left[0] * right[1] - left[1] * right[0]};
  }

  [[nodiscard]] constexpr auto LengthSquared() const noexcept { return Dot(*this, *this); }

  [[nodiscard]] auto Length() const noexcept { return std::sqrt(LengthSquared()); }

  [[nodiscard]] auto Normalize() const -> std::expected<NormalizedVector<T, N>, Error>
    requires std::floating_point<T>;
};

export template <arithmetic T, std::size_t N>
  requires(N > 0)
class NormalizedVector : public Vector<T, N> {
  static_assert(std::floating_point<T>, "`NormalizedVector` requires floating-point `T`!");

  friend class Vector<T, N>;

  explicit constexpr NormalizedVector(Vector<T, N> &&other) noexcept
      : Vector<T, N>(std::move(other)) {}

public:
  NormalizedVector(const NormalizedVector &) = default;
  NormalizedVector(NormalizedVector &&) = default;
  NormalizedVector &operator=(const NormalizedVector &) = default;
  NormalizedVector &operator=(NormalizedVector &&) = default;

  [[nodiscard]] constexpr const T &operator[](const std::size_t index) const noexcept {
    return Vector<T, N>::operator[](index);
  }

  constexpr Vector<T, N> &operator+=(const Vector<T, N> &) = delete;
  constexpr Vector<T, N> &operator-=(const Vector<T, N> &) = delete;
  constexpr Vector<T, N> &operator*=(T) = delete;
  constexpr Vector<T, N> &operator*=(const Vector<T, N> &) = delete;
  constexpr Vector<T, N> &operator/=(T) = delete;

  [[nodiscard]] const NormalizedVector &Normalize() const noexcept { return *this; }
};

template <arithmetic T, std::size_t N>
  requires(N > 0)
auto Vector<T, N>::Normalize() const -> std::expected<NormalizedVector<T, N>, Error>
  requires std::floating_point<T>
{
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
