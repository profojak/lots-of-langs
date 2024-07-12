module;

#include <string>
#include <vector>

#include <cstdint>
#include <cstring>

module Image;

namespace render {

void Image::ReadTgaFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if(!in.is_open())
        throw ImageError("Error opening file " + filename);

    Header header;
    in.read(reinterpret_cast<char *>(&header), sizeof(header));
    if(!in.good())
        throw ImageError("Error reading header");

    width = header.width;
    height = header.height;
    bytes_per_pixel = header.bits_per_pixel / 8;
    if(width <= 0 || height <= 0 ||
        (bytes_per_pixel != 1 && bytes_per_pixel != 3 && bytes_per_pixel != 4))
        throw ImageError("Bad image format");

    const auto bytes_count = bytes_per_pixel * width * height;
    data = std::vector<uint8_t>(bytes_count, 0);
    z_buffer = std::vector<float>(bytes_count,
        -std::numeric_limits<float>::max());

    if(3 == header.data_type_code || 2 == header.data_type_code) {
        in.read(reinterpret_cast<char *>(data.data()), bytes_count);
        if(!in.good())
            throw ImageError("Error reading data");
    } else if(10 == header.data_type_code || 11 == header.data_type_code)
        try {
            LoadRleData(in);
        } catch(ImageError& error) {
            throw error;
        }
    else
        throw ImageError("Unknown file format");

    if(!(header.image_descriptor & 0x20))
        FlipVertically();
    if(header.image_descriptor & 0x10)
        FlipHorizontally();
    in.close();
}

void Image::WriteTgaFile(const std::string& filename, const bool rle)
    const {
    const std::array<uint8_t, 4> developer_area_ref{};
    const std::array<uint8_t, 4> extension_area_ref{};
    const auto footer = "TRUEVISION-XFILE.\0";

    std::ofstream out;
    out.open(filename, std::ios::binary);
    if(!out.is_open())
        throw ImageError("Error opening file " + filename);

    Header header;
    header.bits_per_pixel = bytes_per_pixel * 8;
    header.width  = width;
    header.height = height;
    header.data_type_code = (bytes_per_pixel == 1 ?
        (rle ? 11 : 3) : (rle ? 10 : 2));
    header.image_descriptor = 0x20;
    out.write(reinterpret_cast<char *>(&header), sizeof(header));
    if(!out.good())
        throw ImageError("Error writing header");

    if(!rle) {
        out.write(reinterpret_cast<const char *>(data.data()),
            width * height * bytes_per_pixel);
        if(!out.good())
            throw ImageError("Error writing data");
    } else {
        try {
            UnloadRleData(out);
        } catch (ImageError& error) {
            throw error;
        }
    }

    out.write(reinterpret_cast<const char *>(developer_area_ref.data()),
        sizeof(developer_area_ref));
    if(!out.good())
        throw ImageError("Error writing data");

    out.write(reinterpret_cast<const char *>(extension_area_ref.data()),
        sizeof(extension_area_ref));
    if(!out.good())
        throw ImageError("Error writing data");

    out.write(reinterpret_cast<const char *>(footer), sizeof(footer));
    if(!out.good())
        throw ImageError("Error writing data");
    out.close();
}

void Image::SetPixel(const int x, const int y, const float z,
    const Pixel& pixel) {
    if(x < 0 || y < 0 || x >= width || y >= height ||
        z < z_buffer[x + y * width])
        return;
    memcpy(data.data() + (x + y * width) * bytes_per_pixel, pixel.data.data(),
        bytes_per_pixel);
    z_buffer[x + y * width] = z;
}

auto Image::GetPixel(const int x, const int y) const -> Pixel {
    Pixel pixel;
    if(x < 0 || y < 0 || x >= width || y >= height)
        return pixel;
    memcpy(pixel.data.data(), data.data() + (x + y * width) * bytes_per_pixel,
        bytes_per_pixel);
    return pixel;
}

void Image::FlipHorizontally() {
    const auto half = width / 2;
    for(auto i = 0; i < half; ++i)
        for(auto j = 0; j < height; ++j)
            for(auto b = 0; b < bytes_per_pixel; ++b)
                std::swap(data[(i + j * width) * bytes_per_pixel + b],
                    data[(width - 1 - i + j * width) * bytes_per_pixel + b]);
}

void Image::FlipVertically() {
    const auto half = height / 2;
    for(auto i = 0; i < width; ++i)
        for(auto j = 0; j < half; ++j)
            for(auto b = 0; b < bytes_per_pixel; ++b)
                std::swap(data[(i + j * width) * bytes_per_pixel + b],
                    data[(i + (height - 1 - j) * width) * bytes_per_pixel + b]);
}

void Image::LoadRleData(std::ifstream& in) {
    const auto pixel_count = width * height;
    auto current_pixel = 0;
    auto current_byte = 0;
    Pixel pixel;

    do{
        uint8_t chunk_header = 0;
        chunk_header = in.get();
        if(!in.good())
            throw ImageError("Error reading chunk header");

        if(chunk_header < 128) {
            chunk_header++;
            for(auto i = 0; i < chunk_header; ++i) {
                in.read(reinterpret_cast<char *>(pixel.data.data()),
                    bytes_per_pixel);
                if(!in.good())
                    throw ImageError("Error reading data");

                for(auto t = 0; t < bytes_per_pixel; ++t)
                    data[current_byte++] = pixel.data[t];
                current_pixel++;
                if(current_pixel > pixel_count)
                    throw ImageError("Too many pixels read");
            }
        } else {
            chunk_header -= 127;
            in.read(reinterpret_cast<char *>(pixel.data.data()),
                bytes_per_pixel);
            if(!in.good())
                throw ImageError("Error reading data");

            for(auto i = 0; i < chunk_header; ++i) {
                for(auto t = 0; t < bytes_per_pixel; ++t)
                    data[current_byte++] = pixel.data[t];
                current_pixel++;
                if(current_pixel > pixel_count)
                    throw ImageError("Too many pixels read");
            }
        }
    } while(current_pixel < pixel_count);
}

void Image::UnloadRleData(std::ofstream& out) const {
    const uint8_t max_chunk_length = 128;
    const auto pixel_count = width * height;
    auto current_pixel = 0;

    while(current_pixel < pixel_count) {
        const auto chunk_start = current_pixel * bytes_per_pixel;
        auto current_byte = current_pixel * bytes_per_pixel;
        uint8_t run_length = 1;
        auto raw = true;

        while(current_pixel + run_length < pixel_count &&
            run_length < max_chunk_length) {
            auto succ_eq = true;
            for(auto t = 0; succ_eq && t < bytes_per_pixel; ++t)
                succ_eq = (data[current_byte + t] ==
                    data[current_byte + t + bytes_per_pixel]);
            current_byte += bytes_per_pixel;
            if(1 == run_length)
                raw = !succ_eq;
            if(raw && succ_eq) {
                run_length--;
                break;
            }
            if(!raw && !succ_eq)
                break;
            run_length++;
        }

        current_pixel += run_length;
        out.put(raw ? run_length - 1 : run_length + 127);
        if(!out.good())
            throw ImageError("Error writing to file");

        out.write(reinterpret_cast<const char *>(data.data() + chunk_start),
            (raw ? run_length * bytes_per_pixel : bytes_per_pixel));
        if(!out.good())
            throw ImageError("Error writing to file");
    }
}

} // namespace render
