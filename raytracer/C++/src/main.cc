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
        Color(0.115, 0.115, 0.105));
    const auto material_arctic = std::make_shared<Lambertian>(
        Color(0.27, 0.74, 0.7));
    const auto material_blue = std::make_shared<Lambertian>(
        Color(0.1, 0.2, 0.5));
    const auto material_light = std::make_shared<Lambertian>(
        Color(0.73, 0.73, 0.73));
    const auto material_bubble_outside = std::make_shared<Dielectric>(1.5);
    const auto material_bubble_inside = std::make_shared<Dielectric>(1.0 / 1.5);
    const auto material_bronze = std::make_shared<Metal>(
        Color(0.8, 0.6, 0.2), 0.9);
    const auto material_gold = std::make_shared<Metal>(
        Color(0.8, 0.7, 0.1), 0.2);
    const auto material_mirror = std::make_shared<Metal>(
        Color(0.7, 0.6, 0.5), 0.0);
    const auto material_glass = std::make_shared<Dielectric>(1.5);

    objects.Add(std::make_shared<Sphere>(Vector3f(0, -100.5, -1), 100,
        material_ground));
    objects.Add(std::make_shared<Sphere>(Vector3f(0, 0, -1.2), 0.5,
        material_arctic));
    objects.Add(std::make_shared<Sphere>(Vector3f(-1, 0, -1), 0.5,
        material_bubble_outside));
    objects.Add(std::make_shared<Sphere>(Vector3f(-1, 0, -1), 0.4,
        material_bubble_inside));
    objects.Add(std::make_shared<Sphere>(Vector3f(1, 0, -1), 0.5,
        material_bronze));
    objects.Add(std::make_shared<Sphere>(Vector3f(-0.3, 0.5, -2.5), 1,
        material_mirror));
    objects.Add(std::make_shared<Sphere>(Vector3f(0.55, -0.38, -0.65), 0.2,
        material_glass));
    objects.Add(std::make_shared<Sphere>(Vector3f(-0.4, -0.38, -0.4), 0.2,
        material_blue));
    objects.Add(std::make_shared<Sphere>(Vector3f(0, 0.65, 1.2), 1,
        material_light));
    objects.Add(std::make_shared<Sphere>(Vector3f(-4.7, 2.4, 3.1), 3,
        material_gold));

    Camera::Orientation orientation;
    orientation.look_from = {-2, 2, 1};
    orientation.look_at = {0, 0, -1};
    orientation.up = {0, 1, 0};

    Camera::Image image;
    image.image_width = 640;
    image.aspect_ratio = 16.0 / 9.0;
    
    auto camera = Camera(orientation, image);
    camera.Render(objects);

    return 0;
}
