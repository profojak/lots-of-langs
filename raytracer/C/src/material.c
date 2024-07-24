#include <math.h>
#include <stdio.h>

#include "material.h"
#include "random.h"

int LambertianScatter(const hit_t *hit, color_t *attenuation,
    ray_t *scattered) {
    lambertian_t *lambertian = (lambertian_t *)hit->mat->mat;
    vec3f_t dir = Add3f(hit->normal, RandomVectorUnitSphere());
    if (fabs(dir.data[0]) < 0.0001 && fabs(dir.data[1]) < 0.0001 &&
        fabs(dir.data[2]) < 0.0001)
        dir = hit->normal;
    for (int i = 0; i < 3; i++)
        attenuation->data[i] = lambertian->albedo.data[i];
    scattered->orig = hit->point;
    scattered->dir = dir;

    return 1;
}

int MetalScatter(const ray_t *ray, const hit_t *hit, color_t *attenuation,
    ray_t *scattered) {
    metal_t *metal = (metal_t *)hit->mat->mat;
    vec3f_t dir = Reflect3f(ray->dir, hit->normal);
    dir = Add3f(Normalize3f(dir), Multiply3f(RandomVectorUnitSphere(),
        metal->fuzz));
    if (DotProduct3f(dir, hit->normal) > 0) {
        for (int i = 0; i < 3; i++)
            attenuation->data[i] = metal->albedo.data[i];
        scattered->orig = hit->point;
        scattered->dir = dir;
        return 1;
    }
    return 0;
}

/**
 * @brief Calculate material reflectance.
 * @param cos_theta Cosine of angle.
 * @param index Refraction index.
 * @return Reflectance.
 */
float DielectricReflectance(const float cos_theta, const float index) {
    float r0 = (1 - index) / (1 + index);
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - cos_theta, 5);
}

int DielectricScatter(const ray_t *ray, const hit_t *hit, color_t *attenuation,
    ray_t *scattered) {
    dielectric_t *dielectric = (dielectric_t *)hit->mat->mat;
    const float index = hit->front_face ? 1 / dielectric->ri : dielectric->ri;
    const vec3f_t unit_dir = Normalize3f(ray->dir);
    const float cos_theta = fmin(DotProduct3f(Multiply3f(unit_dir, -1),
        hit->normal), 1);
    const float sin_theta = sqrt(1 - cos_theta * cos_theta);

    for (int i = 0; i < 3; i++)
        attenuation->data[i] = 1;
    scattered->orig = hit->point;

    if (index * sin_theta > 1 || DielectricReflectance(cos_theta, index) >
        RandomFloat()) {
        vec3f_t dir = Reflect3f(unit_dir, hit->normal);
        scattered->dir = dir;
    } else {
        vec3f_t dir = Refract3f(unit_dir, hit->normal, index);
        scattered->dir = dir;
    }

    return 1;
}

int MaterialScatter(const ray_t *ray, const hit_t *hit, color_t *attenuation,
    ray_t *scattered) {
    switch (hit->mat->type) {
        case MATERIAL_LAMBERTIAN:
            return LambertianScatter(hit, attenuation, scattered);
            break;
        case MATERIAL_METAL:
            return MetalScatter(ray, hit, attenuation, scattered);
            break;
        case MATERIAL_DIELECTRIC:
            return DielectricScatter(ray, hit, attenuation, scattered);
            break;
        default:
            fprintf(stderr, "Unknown material type\n");
            break;
    }
    return 0;
}
