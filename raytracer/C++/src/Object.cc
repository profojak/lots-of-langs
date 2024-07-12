module;

#include <optional>

#include <cmath>

module Object;

namespace ray {

auto Objects::CheckHit(const Ray& ray, const Interval interval) const ->
    std::optional<Hit> {
    bool is_hit{false};
    Hit hit{};
    hit.distance = interval.Max();
    for(const auto& object : objects)
        if(const auto new_hit = object->CheckHit(ray,
            Interval{interval.Min(), hit.distance})) {
            is_hit = true;
            hit = *new_hit;
        }
    return is_hit ? std::make_optional(hit) : std::nullopt;
}

auto Sphere::CheckHit(const Ray& ray, const Interval interval) const ->
    std::optional<Hit> {
    const auto origin_center = center - ray.Origin();
    const auto a = ray.Direction().Length2();
    const auto b = origin_center.Dot(ray.Direction());
    const auto c = origin_center.Length2() - radius * radius;
    const auto discriminant = b * b - a * c;

    if(discriminant < 0.0f)
        return std::nullopt;
    const auto square_root = std::sqrt(discriminant);
    auto distance = (b - square_root) / a;
    if(!interval.Surrounds(distance)) {
        distance = (b + square_root) / a;
        if(!interval.Surrounds(distance))
            return std::nullopt;
    }

    const auto point = ray.PointAt(distance);
    const auto normal = (point - center) / radius;
    const auto front_face = ray.Direction().Dot(normal) < 0.0f;

    Hit hit{
        .point = point,
        .normal = front_face ? normal : -normal,
        .material = material,
        .distance = distance,
        .front_face = front_face
    };

    return std::make_optional(hit);
}

} // namespace ray
