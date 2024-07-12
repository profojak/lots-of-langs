module;

#include <iostream>
#include <limits>
#include <ranges>
#include <thread>
#include <vector>

#include <cmath>

import Random;

module Camera;

namespace ray {

Camera::Camera(Orientation orientation, Image image, Lens lens,
    Sampling sampling) :
    orientation_configuration(orientation), image_configuration(image),
    lens_configuration(lens), sampling_configuration(sampling),
    image_height{static_cast<int>(image.image_width / image.aspect_ratio)} {

    file.open("image.ppm", std::ios::out);
    if(!file.is_open())
        throw std::runtime_error("Failed to open image file for writing");

    const auto viewport_height = 2.0f *
        std::tan((lens_configuration.vertical_fov * M_PI / 180.0f) / 2.0f) *
        lens_configuration.focus_distance;
    const auto viewport_width = viewport_height *
        (static_cast<float>(image.image_width) / image_height);

    const auto w = math::Normalize(orientation_configuration.look_from -
        orientation_configuration.look_at);
    const auto u = math::Cross(orientation_configuration.up, w).Normalize();
    const auto v = math::Cross(w, u);
    const auto viewport_u = viewport_width * u;
    const auto viewport_v = viewport_height * -v;
    const auto viewport_up_left = orientation_configuration.look_from -
        (lens_configuration.focus_distance * w) - viewport_u / 2.0f -
        viewport_v / 2.0f;

    pixel_delta_u = viewport_u / image.image_width;
    pixel_delta_v = viewport_v / image_height;
    pixel_up_left = viewport_up_left + 0.5f * (pixel_delta_u + pixel_delta_v);
    pixel_sample_weight = 1.0f / sampling.samples;

    const auto defocus_radius = lens_configuration.focus_distance *
        std::tan((lens_configuration.defocus_angle * M_PI / 180.0f) / 2.0f);
    defocus_disk_delta_u = defocus_radius * u;
    defocus_disk_delta_v = defocus_radius * v;
}

Camera::~Camera() noexcept {
    if(file.is_open())
        file.close();
}

void Camera::Render(const Object& scene) {
    const auto threads_count = std::thread::hardware_concurrency();
    const auto segment = image_height / threads_count;

    std::vector<std::thread> threads;
    threads.resize(threads_count);

    std::vector<std::vector<Color>> colors;
    colors.resize(threads_count);
    for(auto& color : colors)
        color.resize(image_configuration.image_width * segment);

    const auto RenderSegment = [&](const int i,
        const int y_start, const int y_end) -> void {
        for(const auto& y : std::ranges::views::iota(y_start, y_end)) {
            for(const auto& x :
                std::ranges::views::iota(0, image_configuration.image_width)) {
                Color color{0.0f, 0.0f, 0.0f};
                for([[maybe_unused]] const auto& sample :
                    std::ranges::views::iota(
                        0, sampling_configuration.samples)) {
                    const auto ray = GetRay(x, y);
                    color += TraceRay(ray, scene,
                        sampling_configuration.max_depth);
                }
                colors[i][(y - y_start) * image_configuration.image_width + x] =
                    pixel_sample_weight * color;
            }
        }
    };

    std::cout << "Ray tracing on " << threads_count << " threads..." <<
        std::endl;

    for(auto i = 0u; i < threads_count; ++i) {
        const auto y_start = i * segment;
        const auto y_end = (i + 1u) * segment;
        if(i == threads_count - 1u)
            threads[i] = std::thread(RenderSegment, i, y_start, image_height);
        else
            threads[i] = std::thread(RenderSegment, i, y_start, y_end);
    }
    for(auto& thread : threads)
        thread.join();

    file << "P3\n" << image_configuration.image_width << ' ' <<
        image_height << "\n255\n";
    for(const auto& color : colors)
        for(const auto& pixel : color)
            WriteColor(pixel);
    std::cout << "Done!" << std::endl;
}

auto Camera::GetRay(const int x, const int y) const noexcept -> Ray {
    const auto offset = std::make_pair(Random::Number<float>() - 0.5f,
        Random::Number<float>() - 0.5f);
    const auto pixel_sample = pixel_up_left +
        (x + offset.first) * pixel_delta_u +
        (y + offset.second) * pixel_delta_v;
    const auto point = Random::VectorUnitDisk<float, 3>();
    const auto origin = orientation_configuration.look_from +
        point[0] * defocus_disk_delta_u + point[1] * defocus_disk_delta_v;
    return Ray(origin, pixel_sample - origin);
}

auto Camera::TraceRay(const Ray& ray, const Object& scene, const int depth)
    const -> Color {
    if(depth <= 0)
        return Color{0.0f, 0.0f, 0.0f};

    if(const auto hit = scene.CheckHit(ray, Interval{1e-4f,
        std::numeric_limits<float>::infinity()})) {
        if(const auto scatter = hit->material->Scatter(ray, *hit)) {
            const auto [attenuation, scattered] = *scatter;
            return attenuation * TraceRay(scattered, scene, depth - 1);
        }
        return Color{0.0f, 0.0f, 0.0f};
    }

    const auto gradient = 0.5f * (ray.Direction().Normalize()[1] + 1.0f);
    return (1.0f - gradient) * Color{1.0f, 1.0f, 1.0f} +
        gradient * Color{0.5f, 0.7f, 1.0f};
}

void Camera::WriteColor(const Color& color) noexcept {
    const auto GammaCorrect = [](const float value) noexcept {
        if(value > 0.0f)
            return std::sqrt(value);
        return 0.0f;
    };

    static const auto intensity = Interval{0.0f, 1.0f - 1e-4f};
    const auto r = GammaCorrect(color[0]);
    const auto g = GammaCorrect(color[1]);
    const auto b = GammaCorrect(color[2]);
    file << static_cast<int>(255.999f * intensity.Clamp(r)) << ' ' <<
        static_cast<int>(255.999f * intensity.Clamp(g)) << ' ' <<
        static_cast<int>(255.999f * intensity.Clamp(b)) << '\n';
}

} // namespace ray
