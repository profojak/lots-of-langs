module;

#include <iostream>

module Camera;

namespace ray {

Camera::Camera(Orientation orientation, Image image, Lens lens,
    Sampling sampling) :
    orientation_configuration(orientation), image_configuration(image),
    lens_configuration(lens), sampling_configuration(sampling),
    image_height{static_cast<int>(image.image_width / image.aspect_ratio)} {
}

Camera::~Camera() noexcept {
    if (file.is_open())
        file.close();
}

void Camera::Render() {
    std::cout << "Lots of Langs!" << std::endl;
}

} // namespace ray
