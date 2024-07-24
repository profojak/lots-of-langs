#include <stdio.h>

#include "object.h"

int CheckHit(const ray_t *ray, const interval_t interval,
    const objects_t *objects, hit_t *hit) {
    int is_hit = 0;
    hit->dist = interval.max;
    for (int i = 0; i < objects->size; i++) {
        hit_t new_hit = *hit;
        if (CheckHitSphere(ray, (interval_t){ interval.min, hit->dist },
            &objects->spheres[i], &new_hit)) {
            is_hit = 1;
            hit->point = new_hit.point;
            hit->normal = new_hit.normal;
            hit->mat = new_hit.mat;
            hit->dist = new_hit.dist;
            hit->front_face = new_hit.front_face;
        }
    }
    return is_hit;
}

int CheckHitSphere(const ray_t *ray, const interval_t interval,
    const sphere_t *sphere, hit_t *hit) {
    const vec3f_t oc = Subtract3f(sphere->center, ray->orig);
    const float a = Length23f(ray->dir);
    const float b = DotProduct3f(oc, ray->dir);
    const float c = Length23f(oc) - sphere->radius * sphere->radius;
    const float discriminant = b * b - a * c;

    if (discriminant < 0)
        return 0;
    const float square_root = sqrtf(discriminant);
    float dist = (b - square_root) / a;
    if (!IntervalSurrounds(dist, interval)) {
        dist = (b + square_root) / a;
        if (!IntervalSurrounds(dist, interval))
            return 0;
    }

    const vec3f_t point = RayPointAt(dist, ray);
    const vec3f_t normal = Divide3f(Subtract3f(point, sphere->center),
        sphere->radius);
    const int front_face = DotProduct3f(ray->dir, normal) < 0;

    hit->point = point;
    hit->normal = front_face ? normal : Multiply3f(normal, -1);
    hit->mat = &sphere->mat;
    hit->dist = dist;
    hit->front_face = front_face;

    return 1;
}
