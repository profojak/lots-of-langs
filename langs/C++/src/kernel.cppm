module;

#include <limits>
#include <numbers>

export module kernel;

import vector;

export namespace pbf {

constexpr float smoothing_radius = 0.2f;
constexpr float radius_2 = smoothing_radius * smoothing_radius;
constexpr float radius_4 = radius_2 * radius_2;
constexpr float poly6_coefficient =
    315.0f / (64.0f * std::numbers::pi_v<float> * smoothing_radius * radius_4 * radius_4);
constexpr float spiky_coefficient = -45.0f / (std::numbers::pi_v<float> * radius_2 * radius_4);

[[nodiscard]] constexpr float Poly6(const float length_squared) {
  if (length_squared > radius_2)
    return 0.0f;
  const float diff = radius_2 - length_squared;
  return poly6_coefficient * diff * diff * diff;
}

[[nodiscard]] constexpr float Poly6(const Vec3f &r) {
  return Poly6(r.LengthSquared());
}

[[nodiscard]] const Vec3f Spiky(const Vec3f &r) {
  const float length = r.Length();
  if (length < std::numeric_limits<float>::epsilon() || length > smoothing_radius)
    return {};
  const float diff = smoothing_radius - length;
  return (spiky_coefficient * diff * diff / length) * r;
}

constexpr float delta_q = 0.2f * smoothing_radius;
constexpr float poly6_delta_q = Poly6(delta_q * delta_q);

} // namespace pbf
