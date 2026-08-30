module;

#include <limits>
#include <numbers>

export module kernel;

import vector;

export namespace pbf {

constexpr float radius = 0.2f;
constexpr float radius_2 = radius * radius;
constexpr float radius_4 = radius_2 * radius_2;
constexpr float poly6_coefficient =
    315.0f / (64.0f * std::numbers::pi_v<float> * radius * radius_4 * radius_4);
constexpr float spiky_coefficient =
    -45.0f / (std::numbers::pi_v<float> * radius_2 * radius_4);

[[nodiscard]] constexpr float Poly6(const float length) {
  if (length > radius)
    return 0.0f;
  const float diff = radius_2 - length * length;
  return poly6_coefficient * diff * diff * diff;
}

[[nodiscard]] constexpr float Poly6(const Vec3f &r) {
  return Poly6(r.Length());
}

[[nodiscard]] const Vec3f Spiky(const Vec3f &r) {
  const float length = r.Length();
  if (length < std::numeric_limits<float>::epsilon() || length > radius)
    return {};
  const float diff = radius - length;
  return (spiky_coefficient * diff * diff / length) * r;
}

} // namespace pbf
