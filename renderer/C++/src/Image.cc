module;

#include <string>

module Image;

namespace render {

void Image::ReadTgaFile(const std::string& filename) {}

void Image::WriteTgaFile(const std::string& filename, const bool rle)
    const {}

void Image::SetPixel(const int x, const int y, const float z,
    const Pixel& pixel) {}

auto Image::GetPixel(const int x, const int y) const -> Pixel {
    return Pixel{};
}

void Image::FlipHorizontally() {}

void Image::FlipVertically() {}

void Image::LoadRleData(std::ifstream& in) {}

void Image::UnloadRleData(std::ofstream& out) const {}

} // namespace render
