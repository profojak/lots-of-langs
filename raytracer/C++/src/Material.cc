module;

#include <optional>

#include <cmath>

import Random;

module Material;

namespace ray {

auto Lambertian::Scatter([[maybe_unused]] const Ray& ray, const Hit& hit)
    const -> std::optional<std::pair<Color, Ray>> {
    auto direction = hit.normal + Random::VectorUnitSphere<float, 3>();
    if(std::abs(direction[0]) < 1e-8f && std::abs(direction[1]) < 1e-8f &&
            std::abs(direction[2]) < 1e-8f)
        direction = hit.normal;
    return std::make_pair(albedo, Ray(hit.point, direction));
}

auto Metal::Scatter(const Ray& ray, const Hit& hit) const ->
    std::optional<std::pair<Color, Ray>> {
    auto direction = ray.Direction().Reflect(hit.normal);
    direction = direction.Normalize() +
        fuzziness * Random::VectorUnitSphere<float, 3>();
    const auto result = Ray(hit.point, direction);
    if(result.Direction().Dot(hit.normal) > 0.0f)
        return std::make_pair(albedo, result);
    return std::nullopt;
}

/**
 * @brief Calculate material reflectance.
 * @param cos_theta Cosine of angle.
 * @param index Refraction index.
 * @return Reflectance.
 */
auto Reflectance(const float cos_theta, const float index) -> float {
    auto r0 = (1.0f - index) / (1.0f + index);
    r0 *= r0;
    return r0 + (1.0f - r0) * std::pow(1.0f - cos_theta, 5);
}

auto Dielectric::Scatter(const Ray& ray, const Hit& hit) const ->
    std::optional<std::pair<Color, Ray>> {
    const auto index = hit.front_face ?
        1.0f / refraction_index : refraction_index;
    const auto direction = ray.Direction().Normalize();
    const auto cos_theta = std::min(-direction.Dot(hit.normal), 1.0f);
    const auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

    if(index * sin_theta > 1.0f ||
        Reflectance(cos_theta, index) > Random::Number<float>())
        return std::make_pair(Color{1.0f, 1.0f, 1.0f},
            Ray(hit.point, direction.Reflect(hit.normal)));
    else
        return std::make_pair(Color{1.0f, 1.0f, 1.0f},
            Ray(hit.point, direction.Refract(hit.normal, index)));
}

};
