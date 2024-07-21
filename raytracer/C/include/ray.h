#ifndef _RAY_H_
#define _RAY_H_

#include "vector.h"

/*! @brief Ray. */
typedef struct {
    /*! @brief Origin. */
    vec3f_t orig;
    /*! @brief Direction. */
    vec3f_t dir;
} ray_t;

/**
 * @brief Get point at given distance along ray.
 * @param[in] t Distance along ray.
 * @param[in] ray Ray.
 * @return Point at specified distance along ray.
 */
static inline vec3f_t RayPointAt(float t, const ray_t *ray) {
    return Add3f(ray->orig, Multiply3f(ray->dir, t));
}

/*! @brief Interval. */
typedef struct {
    /*! @brief Minimum distance. */
    float min;
    /*! @brief Maximum distance. */
    float max;
} interval_t;

/**
 * @brief Check if interval contains value.
 * @param[in] value Value to check.
 * @param[in] interval Interval.
 * @return Non-zero if interval contains value, zero otherwise.
 */
inline int IntervalContains(float value, const interval_t interval) {
    return value >= interval.min && value <= interval.max;
}

/**
 * @brief Check if interval surrounds value.
 * @param[in] value Value to check.
 * @param[in] interval Interval.
 * @return Non-zero if interval surrounds value, zero otherwise.
 */
inline int IntervalSurrounds(float value, const interval_t interval) {
    return value > interval.min && value < interval.max;
}

/**
 * @brief Clamp value to interval.
 * @param[in] value Value to clamp.
 * @param[in] interval Interval.
 * @return Clamped value.
 */
inline float IntervalClamp(float value, const interval_t interval) {
    return value < interval.min ? interval.min :
        (value > interval.max ? interval.max : value);
}

#endif // _RAY_H_
