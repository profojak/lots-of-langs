#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "material.h"
#include "ray.h"
#include "vector.h"

struct sphere_t;

/*! @brief Container of hittable objects. */
typedef struct {
    /*! @brief Array of spheres. */
    struct sphere_t *spheres;
    /*! @brief Number of spheres. */
    int size;
} objects_t;

/*! @brief Sphere object. */
typedef struct sphere_t {
    /*! @brief Center. */
    vec3f_t center;
    /*! @brief Radius. */
    float radius;
    /*! @brief Material. */
    material_t mat;
} sphere_t;

/**
 * @brief Check if ray hits object.
 * @param[in] ray Ray.
 * @param[in] interval Interval of minimum and maximum distances.
 * @param[in] objects Objects to check.
 * @param[out] hit Hit record.
 * @return Zero on miss, non-zero on hit.
 */
int CheckHit(const ray_t *ray, const interval_t interval,
    const objects_t *objects, hit_t *hit);

/**
 * @brief Check if ray hits sphere.
 * @param[in] ray Ray.
 * @param[in] interval Interval of minimum and maximum distances.
 * @param[in] sphere Sphere to check.
 * @param[out] hit Hit record.
 * @return Zero on miss, non-zero on hit.
 */
int CheckHitSphere(const ray_t *ray, const interval_t interval,
    sphere_t *sphere, hit_t *hit);

#endif // _OBJECT_H_
