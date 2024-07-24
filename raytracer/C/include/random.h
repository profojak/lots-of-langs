#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <stdlib.h>

#include "vector.h"

/*! @brief Generate random float. */
static inline float RandomFloat(void) {
    return (float)rand() / (float)RAND_MAX;
}

/**
 * @brief Generate random float within range.
 * @param min Minimum value.
 * @param max Maximum value.
 */
static inline float RandomFloatRange(const float min, const float max) {
    return min + (max - min) * RandomFloat();
}

/*! @brief Generate random vector. */
static inline vec3f_t RandomVector(void) {
    return (vec3f_t) { RandomFloat(), RandomFloat(), RandomFloat() };
}

/**
 * @brief Generate random vector within range.
 * @param min Minimum value.
 * @param max Maximum value.
 */
static inline vec3f_t RandomVectorRange(const float min, const float max) {
    return (vec3f_t) { RandomFloatRange(min, max), RandomFloatRange(min, max),
        RandomFloatRange(min, max) };
}

/*! @brief Generate random vector within unit sphere. */
static inline vec3f_t RandomVectorUnitSphere(void) {
    vec3f_t p;
    do {
        p = RandomVectorRange(-1, 1);
    } while (Length23f(p) >= 1);
    return Normalize3f(p);
}

/*! @brief Generate random vector within unit disk. */
static inline vec3f_t RandomVectorUnitDisk(void) {
    vec3f_t p;
    do {
        p = (vec3f_t) { RandomFloatRange(-1, 1), RandomFloatRange(-1, 1), 0 };
    } while (Length23f(p) >= 1);
    return p;
}

#endif // _RANDOM_H_
