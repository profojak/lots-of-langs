#include <float.h>
#include <math.h>

#include "shader.h"

vec3f_t Barycentric(const vec3f_t t0, const vec3f_t t1, const vec3f_t t2,
    const vec3f_t p) {
    vec3f_t s[2];
    for (int i = 2; i--; ) {
        s[i].data[0] = t2.data[i] - t0.data[i];
        s[i].data[1] = t1.data[i] - t0.data[i];
        s[i].data[2] = t0.data[i] - p.data[i];
    }
    vec3f_t u = CrossProductf(s[0], s[1]);
    if (fabs(u.data[2]) > 1e-2)
        return (vec3f_t){1.0f - (u.data[0] + u.data[1]) / u.data[2],
            u.data[1] / u.data[2], u.data[0] / u.data[2]};
    return (vec3f_t){-1, 1, 1};
}

void ShaderLoadTriangle(const size_t face, shader_t *shader) {
    for (int i = 0; i < 3; i++) {
        shader->verts[i] = ModelGetVertex(face, i, shader->model);
        shader->norms[i] = ModelGetNormal(face, i, shader->model);
        shader->texels[i] = ModelGetTexel(face, i, shader->model);
    }
}

void ShaderRenderTriangle(image_t *image, shader_t *shader) {
    for (int i = 0; i < 3; i++) {
        shader->verts[i].data[0] = (int)((shader->verts[i].data[0] + 1.0f) *
            0.5f * image->w + 0.5f);
        shader->verts[i].data[1] = (int)((shader->verts[i].data[1] + 1.0f) *
            0.5f * image->h + 0.5f);
    }

    vec2f_t bbox_min = {FLT_MAX, FLT_MAX};
    vec2f_t bbox_max = {-FLT_MAX, -FLT_MAX};
    vec2f_t clamp = {image->w - 1.0f, image->h - 1.0f};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bbox_min.data[j] = fmaxf(0.0f, fminf(bbox_min.data[j],
                shader->verts[i].data[j]));
            bbox_max.data[j] = fminf(clamp.data[j], fmaxf(bbox_max.data[j],
                shader->verts[i].data[j]));
        }
    }

    vec3f_t p;
    for (p.data[0] = bbox_min.data[0]; p.data[0] <= bbox_max.data[0];
        p.data[0]++) {
        for (p.data[1] = bbox_min.data[1]; p.data[1] <= bbox_max.data[1];
            p.data[1]++) {
            vec3f_t bc = BarycentricTriangle(shader->verts, p);
            if (bc.data[0] < 0 || bc.data[1] < 0 || bc.data[2] < 0)
                continue;
            p.data[2] = 0;
            for (int i = 0; i < 3; i++)
                p.data[2] += shader->verts[i].data[2] * bc.data[i];
            vec2f_t uv = {0, 0};
            for (int i = 0; i < 3; i++)
                uv = Add2f(uv, Multiply2f(shader->texels[i], bc.data[i]));
            const pixel_t pixel = ModelGetTexturePixel(uv, shader->model);
            ImageSetPixel(p.data[0], p.data[1], p.data[2], pixel, image);
        }
    }
}
