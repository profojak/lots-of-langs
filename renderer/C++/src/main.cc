#include <iostream>

import Image;
import Model;
import Shader;

using namespace render;

/*! @brief Image width. */
constexpr auto WIDTH = 640;
/*! @brief Image height. */
constexpr auto HEIGHT = 640;

/*! @brief Main function. */
int main(const int argc, const char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " model.obj" << std::endl;
        std::exit(1);
    }

    Image image(WIDTH, HEIGHT, Image::Format::RGB);
    for(auto arg = 1; arg < argc; ++arg) {
        auto model = Model::Load("../.obj/" + std::string{argv[arg]});
        Shader shader(model);
        for(auto i = 0u; i < model.FacesCount(); ++i) {
            shader.LoadTriangle(i);
            shader.RenderTriangle(image);
        }
    }

    image.FlipVertically();
    image.WriteTgaFile("image.tga");

    return 0;
}
