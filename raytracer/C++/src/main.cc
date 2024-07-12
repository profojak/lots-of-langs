#include <iostream>
#include <memory>

import Camera;
import Material;
import Object;

using namespace ray;

/*! @brief Main function. */
int main() {
    std::ios_base::sync_with_stdio(false);

    Objects objects;

    const auto material_ground = std::make_shared<Lambertian>(
        Color{0.115f, 0.115f, 0.105f});
    const auto material_arctic = std::make_shared<Lambertian>(
        Color{0.27f, 0.74f, 0.7f});
    const auto material_blue = std::make_shared<Lambertian>(
        Color{0.1f, 0.2f, 0.5f});
    const auto material_light = std::make_shared<Lambertian>(
        Color{0.73f, 0.73f, 0.73f});
    const auto material_bubble_outside = std::make_shared<Dielectric>(1.5f);
    const auto material_bubble_inside = std::make_shared<Dielectric>(
        1.0f / 1.5f);
    const auto material_bronze = std::make_shared<Metal>(
        Color{0.8f, 0.6f, 0.2f}, 0.9f);
    const auto material_gold = std::make_shared<Metal>(
        Color{0.8f, 0.7f, 0.1f}, 0.2f);
    const auto material_mirror = std::make_shared<Metal>(
        Color{0.7f, 0.6f, 0.5f}, 0.0f);
    const auto material_glass = std::make_shared<Dielectric>(1.5f);

    objects.Add(std::make_shared<Sphere>(Vector3f{0.0f, -100.5f, -1.0f}, 100.0f,
        material_ground));
    objects.Add(std::make_shared<Sphere>(Vector3f{0.0f, 0.0f, -1.2f}, 0.5f,
        material_arctic));
    objects.Add(std::make_shared<Sphere>(Vector3f{-1.0f, 0.0f, -1.0f}, 0.5f,
        material_bubble_outside));
    objects.Add(std::make_shared<Sphere>(Vector3f{-1.0f, 0.0f, -1.0f}, 0.4f,
        material_bubble_inside));
    objects.Add(std::make_shared<Sphere>(Vector3f{1.0f, 0.0f, -1.0f}, 0.5f,
        material_bronze));
    objects.Add(std::make_shared<Sphere>(Vector3f{-0.3f, 0.5f, -2.5f}, 1.0f,
        material_mirror));
    objects.Add(std::make_shared<Sphere>(Vector3f{0.55f, -0.38f, -0.65f}, 0.2f,
        material_glass));
    objects.Add(std::make_shared<Sphere>(Vector3f{-0.4f, -0.38f, -0.4f}, 0.2f,
        material_blue));
    objects.Add(std::make_shared<Sphere>(Vector3f{0.0f, 0.65f, 1.2f}, 1.0f,
        material_light));
    objects.Add(std::make_shared<Sphere>(Vector3f{-4.7f, 2.4f, 3.1f}, 3.0f,
        material_gold));

    Camera::Orientation orientation;
    orientation.look_from = {-2.0f, 2.0f, 1.0f};
    orientation.look_at = {0.0f, 0.0f, -1.0f};
    orientation.up = {0.0f, 1.0f, 0.0f};

    Camera::Image image;
    image.image_width = 640;
    image.aspect_ratio = 16.0f / 9.0f;
    
    auto camera = Camera(orientation, image);
    camera.Render(objects);

    return 0;
}
