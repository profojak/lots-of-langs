module;

#include <iostream>
#include <ranges>

#include <cmath>

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
        std::tan((lens_configuration.vertical_fov * M_PI / 180) / 2) *
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
        std::tan((lens_configuration.defocus_angle * M_PI / 180) / 2);
    defocus_disk_delta_u = defocus_radius * u;
    defocus_disk_delta_v = defocus_radius * v;
}

Camera::~Camera() noexcept {
    if(file.is_open())
        file.close();
}

void Camera::Render() {
    file << "P3\n" << image_configuration.image_width << ' ' <<
        image_height << "\n255\n";

    for(const auto& y : std::ranges::views::iota(0, image_height)) {
        if((y + 1) % 5 == 0)
            std::cout << '*' << std::flush;

        for(const auto& x :
            std::ranges::views::iota(0, image_configuration.image_width)) {
            for([[maybe_unused]] const auto& sample :
                std::ranges::views::iota(0, sampling_configuration.samples)) {
                [[maybe_unused]] const auto ray = GetRay(x, y);
            }
        }
    }

    std::cout << "\nDone!" << std::endl;
}

auto Camera::GetRay([[maybe_unused]] const int x,
    [[maybe_unused]] const int y) const noexcept -> Ray {
    return {};
}

} // namespace ray
