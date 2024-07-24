#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "ray.h"
#include "vector.h"

/*! @brief Color. */
typedef vec3f_t color_t;

/*! @brief Material type. */
enum material_type_t {
    MATERIAL_LAMBERTIAN,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC
};

struct material_t;

/*! @brief Hit record. */
typedef struct {
    /*! @brief Point of intersection. */
    vec3f_t point;
    /*! @brief Normal at point of intersection. */
    vec3f_t normal;
    /*! @brief Material of intersected object. */
    const struct material_t *mat;
    /*! @brief Distance from ray origin. */
    float dist;
    /*! @brief Whether ray hit front face. */
    int front_face;
} hit_t;

/*! @brief Material interface. */
typedef struct material_t {
    /*! @brief Material type. */
    enum material_type_t type;
    /*! @brief Material. */
    void *mat;
} material_t;

/*! @brief Lambertian material. */
typedef struct {
    /*! @brief Albedo. */
    color_t albedo;
} lambertian_t;

/*! @brief Metal material. */
typedef struct {
    /*! @brief Albedo. */
    color_t albedo;
    /*! @brief Fuzziness. */
    float fuzz;
} metal_t;

/*! @brief Dielectric material. */
typedef struct {
    /*! @brief Refraction index. */
    float ri;
} dielectric_t;

/**
 * @brief Scatter ray if object is hit.
 * @param[in] ray Ray.
 * @param[in] hit Hit record.
 * @param[out] attenuation Attenuation.
 * @param[out] scattered Scattered ray.
 * @return Non-zero if ray was scattered, zero otherwise.
 */
int MaterialScatter(const ray_t *ray, const hit_t *hit, color_t *attenuation,
    ray_t *scattered);

#endif // _MATERIAL_H_
