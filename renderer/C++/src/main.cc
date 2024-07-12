#include <iostream>

import Image;

using namespace render;

constexpr auto WIDTH = 640;
constexpr auto HEIGHT = 640;

/*! @brief Main function. */
int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " model.obj" << std::endl;
        std::exit(1);
    }

    Image image(WIDTH, HEIGHT, Image::Format::RGB);

    return 0;
}
