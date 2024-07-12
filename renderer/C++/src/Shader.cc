module;

#include <array>
#include <limits>

#include <cmath>

import Image;

module Shader;

namespace render {

auto Shader::Barycentric(const Vector3f t0, const Vector3f t1,
    const Vector3f t2, const Vector3f p) -> Vector3f {
    std::array<Vector3f, 2> s;
    for(auto i: {0, 1}) {
        s[i][0] = t2[i] - t0[i];
        s[i][1] = t1[i] - t0[i];
        s[i][2] = t0[i] - p[i];
    }
    const auto u = s[0].Cross(s[1]);
    if(std::abs(u[2]) > 1e-2f)
        return Vector3f{1.0f - (u[0] + u[1]) / u[2], u[1] / u[2], u[0] / u[2]};
    return Vector3f{-1.0f, 1.0f, 1.0f};
}

void Shader::LoadTriangle(const std::size_t face) {
    for(auto i: {0, 1, 2}) {
        vertices[i] = model.GetVertex(face, i);
        normals[i] = model.GetNormal(face, i);
        texels[i] = model.GetTexel(face, i);
    }
}

void Shader::RenderTriangle(Image& image) {
    for(auto i: {0, 1, 2}) {
        vertices[i][0] = static_cast<int>((vertices[i][0] + 1.0f) *
            image.GetWidth() / 2.0f + 0.5f);
        vertices[i][1] = static_cast<int>((vertices[i][1] + 1.0f) *
            image.GetHeight() / 2.0f + 0.5f);
    }

    Vector2f bbox_min{std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()};
    Vector2f bbox_max{-std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max()};
    Vector2f clamp{image.GetWidth() - 1, image.GetHeight() - 1};
    for(auto i: {0, 1, 2}) {
        for(auto j: {0, 1}) {
            bbox_min[j] = std::max(0.0f, std::min(bbox_min[j], vertices[i][j]));
            bbox_max[j] = std::min(clamp[j],
                std::max(bbox_max[j], vertices[i][j]));
        }
    }

    Vector3f p;
    for(p[0] = bbox_min[0]; p[0] <= bbox_max[0]; ++p[0]) {
        for(p[1] = bbox_min[1]; p[1] <= bbox_max[1]; ++p[1]) {
            const auto barycentric = Barycentric(vertices, p);
            if(barycentric[0] < 0 || barycentric[1] < 0 || barycentric[2] < 0)
                continue;
            p[2] = 0;
            for(auto i: {0, 1, 2})
                p[2] += vertices[i][2] * barycentric[i];
            Vector2f uv{};
            for(auto i: {0, 1, 2})
                uv += texels[i] * barycentric[i];
            const auto diffuse = model.GetTexturePixel(uv);
            image.SetPixel(p[0], p[1], p[2], diffuse);
        }
    }
}

} // namespace render
