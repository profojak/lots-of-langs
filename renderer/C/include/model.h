#ifndef _MODEL_H_
#define _MODEL_H_

#include "vector.h"
#include "image.h"

/*! @brief Model. */
typedef struct {
    /*! @brief Vertices. */
    vec3f_t *verts;
    /*! @brief Normals. */
    vec3f_t *norms;
    /*! @brief Texture coordinates. */
    vec2f_t *texels;
    /*! @brief Facet vertices. */
    int *facet_verts;
    /*! @brief Facet normals. */
    int *facet_norms;
    /*! @brief Facet texture coordinates. */
    int *facet_texels;
    /*! @brief Number of vertices. */
    int verts_num;
    /*! @brief Number of normals. */
    int norms_num;
    /*! @brief Number of texels. */
    int texels_num;
    /*! @brief Number of facet vertices. */
    int facet_verts_num;
    /*! @brief Number of facet normals. */
    int facet_norms_num;
    /*! @brief Number of facet texture coordinates. */
    int facet_texels_num;
    /*! @brief Texture. */
    image_t texture;
} model_t;

/**
 * @brief Load model from file.
 * @param[in] filename Filename.
 * @return Model.
 */
model_t ModelLoad(const char *filename);

/**
 * @brief Get n-th vertex of i-th face.
 * @param[in] face Face index.
 * @param[in] vertex Vertex index.
 * @paramp[in] model Model.
 */
inline vec3f_t ModelGetVertex(const size_t face, const size_t vertex,
    const model_t *model) {
    return model->verts[model->facet_verts[face * 3 + vertex]];
}

/**
 * @brief Get normal of n-th vertex of i-th face.
 * @param[in] face Face index.
 * @param[in] vertex Vertex index.
 * @param[in] model Model.
 */
inline vec3f_t ModelGetNormal(const size_t face, const size_t vertex,
    const model_t *model) {
    return model->norms[model->facet_norms[face * 3 + vertex]];
}

/**
 * @brief Get texture coordinate of n-th vertex of i-th face.
 * @param[in] face Face index.
 * @param[in] vertex Vertex index.
 * @param[in] model Model.
 */
inline vec2f_t ModelGetTexel(const size_t face, const size_t vertex,
    const model_t *model) {
    return model->texels[model->facet_texels[face * 3 + vertex]];
}

/**
 * @brief Sample texture.
 * @param[in] uv Texture coordinates.
 * @param[in] model Model.
 * @return Pixel.
 */
inline pixel_t ModelGetTexturePixel(const vec2f_t uv, const model_t *model) {
    return ImageGetPixel(uv.data[0] * model->texture.w,
            uv.data[1] * model->texture.h, &model->texture);
}

#endif // _MODEL_H_
