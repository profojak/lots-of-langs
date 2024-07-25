#ifndef _SHADER_H_
#define _SHADER_H_

#include "model.h"
#include "image.h"

/*! @brief Shader. */
typedef struct {
    /*! @brief Model. */
    model_t *model;
    /*! @brief Triangle vertices. */
    vec3f_t verts[3];
    /*! @brief Triangle normals. */
    vec3f_t norms[3];
    /*! @brief Triangle texture coordinates. */
    vec2f_t texels[3];
} shader_t;

/**
 * @brief Compute barycentric coordinates.
 * @param[in] t0 First point.
 * @param[in] t1 Second point.
 * @param[in] t2 Third point.
 * @param[in] p Point to compute barycentric coordinates for.
 */
vec3f_t Barycentric(const vec3f_t t0, const vec3f_t t1, const vec3f_t t2,
    const vec3f_t p);

/**
 * @brief Compute barycentric coordinates.
 * @param[in] t Triangle.
 * @param[in] p Point to compute barycentric coordinates for.
 */
inline vec3f_t BarycentricTriangle(const vec3f_t t[3], const vec3f_t p) {
    return Barycentric(t[0], t[1], t[2], p);
}

/**
 * @brief Load triangle to shader.
 * @param[in] face Triangle facet.
 * @param[in] shader Shader.
 */
void ShaderLoadTriangle(const size_t face, shader_t *shader);

/**
 * @brief Rasterize triangle.
 * @param[in,out] image Image.
 * @param[in] shader Shader.
 */
void ShaderRenderTriangle(image_t *image, shader_t *shader);

#endif // _SHADER_H_
