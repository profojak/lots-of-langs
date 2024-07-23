#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <math.h>

#define TYPE_MAP(T) TYPE_MAP_##T
#define TYPE_MAP_f float
#define TYPE_MAP_d double

/**
 * @brief Vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 */
#define DEFINE_VECTOR(N, T)                                                    \
typedef struct {                                                               \
    TYPE_MAP(T) data[N];                                                       \
} vec##N##T##_t;

/**
 * @brief Negate vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @return Vector.
 */
#define DEFINE_NEGATE(N, T) \
static inline vec##N##T##_t Negate##N##T(vec##N##T##_t v) {                    \
    vec##N##T##_t result;                                                      \
    for (int i = 0; i < N; i++) {                                              \
        result.data[i] = -v.data[i];                                           \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Add two vectors.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Vector.
 */
#define DEFINE_ADD(N, T) \
static inline vec##N##T##_t Add##N##T(vec##N##T##_t v1, vec##N##T##_t v2) {    \
    vec##N##T##_t result;                                                      \
    for (int i = 0; i < N; i++) {                                              \
        result.data[i] = v1.data[i] + v2.data[i];                              \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Subtract two vectors.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Vector.
 */
#define DEFINE_SUBTRACT(N, T)                                                  \
static inline vec##N##T##_t Subtract##N##T(vec##N##T##_t v1,                   \
    vec##N##T##_t v2) {                                                        \
    vec##N##T##_t result;                                                      \
    for (int i = 0; i < N; i++) {                                              \
        result.data[i] = v1.data[i] - v2.data[i];                              \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Multiply vector by scalar.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @param s Scalar.
 * @return Vector.
 */
#define DEFINE_MULTIPLY(N, T) \
static inline vec##N##T##_t Multiply##N##T(vec##N##T##_t v, TYPE_MAP(T) s) {   \
    vec##N##T##_t result;                                                      \
    for (int i = 0; i < N; i++) {                                              \
        result.data[i] = v.data[i] * s;                                        \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Divide vector by scalar.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @param s Scalar.
 * @return Vector.
 */
#define DEFINE_DIVIDE(N, T) \
static inline vec##N##T##_t Divide##N##T(vec##N##T##_t v, TYPE_MAP(T) s) {     \
    vec##N##T##_t result;                                                      \
    for (int i = 0; i < N; i++) {                                              \
        result.data[i] = v.data[i] / s;                                        \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Compute squared length of vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @return Squared length.
 */
#define DEFINE_LENGTH_SQUARED(N, T)                                            \
static inline TYPE_MAP(T) Length2##N##T(vec##N##T##_t v) {                     \
    TYPE_MAP(T) result = 0;                                                    \
    for (int i = 0; i < N; i++) {                                              \
        result += v.data[i] * v.data[i];                                       \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Compute length of vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @return Length.
 */
#define DEFINE_LENGTH(N, T)                                                    \
static inline TYPE_MAP(T) Length##N##T(vec##N##T##_t v) {                      \
    return sqrt(Length2##N##T(v));                                             \
}

/**
 * @brief Compute dot product of vectors.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Dot product.
 */
#define DEFINE_DOT_PRODUCT(N, T)                                               \
static inline TYPE_MAP(T) DotProduct##N##T(vec##N##T##_t v1,                   \
    vec##N##T##_t v2) {                                                        \
    TYPE_MAP(T) result = 0;                                                    \
    for (int i = 0; i < N; i++) {                                              \
        result += v1.data[i] * v2.data[i];                                     \
    }                                                                          \
    return result;                                                             \
}

/**
 * @brief Compute cross product of 3D vectors.
 * @tparam T Type of elements.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Cross product.
 */
#define DEFINE_CROSS_PRODUCT(T) \
static inline vec3##T##_t CrossProduct##T(vec3##T##_t v1, vec3##T##_t v2) {    \
    vec3##T##_t result;                                                        \
    result.data[0] = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1];        \
    result.data[1] = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2];        \
    result.data[2] = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];        \
    return result;                                                             \
}

/**
 * @brief Normalize vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @return Normalized vector.
 */
#define DEFINE_NORMALIZE(N, T)                                                 \
static inline vec##N##T##_t Normalize##N##T(vec##N##T##_t v) {                 \
    return Divide##N##T(v, Length##N##T(v));                                   \
}

/**
 * @brief Reflect vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @param n Normal vector.
 * @return Reflected vector.
 */
#define DEFINE_REFLECT(N, T) \
static inline vec##N##T##_t Reflect##N##T(vec##N##T##_t v, vec##N##T##_t n) {  \
    return Subtract##N##T(v, Multiply##N##T(n, 2 * DotProduct##N##T(v, n)));   \
}

/**
 * @brief Refract vector.
 * @tparam N Number of elements.
 * @tparam T Type of elements.
 * @param v Vector.
 * @param n Normal vector.
 * @param eta Refraction ratio.
 * @return Refracted vector.
 */
#define DEFINE_REFRACT(N, T) \
static inline vec##N##T##_t Refract##N##T(vec##N##T##_t v, vec##N##T##_t n,    \
    TYPE_MAP(T) eta) {                                                         \
    TYPE_MAP(T) cosI = -DotProduct##N##T(v, n);                                \
    TYPE_MAP(T) sinT2 = eta * eta * (1 - cosI * cosI);                         \
    if (sinT2 > 1) {                                                           \
        vec##N##T##_t result = {0};                                            \
        return result;                                                         \
    }                                                                          \
    return Add##N##T(Multiply##N##T(v, eta), Multiply##N##T(n,                 \
        eta * cosI - sqrt(1 - sinT2)));                                        \
}

#define DEFINE_VECTOR_OPERATIONS(N, T)                                         \
DEFINE_ADD(N, T)                                                               \
DEFINE_SUBTRACT(N, T)                                                          \
DEFINE_MULTIPLY(N, T)                                                          \
DEFINE_DIVIDE(N, T)                                                            \
DEFINE_NEGATE(N, T)                                                            \
DEFINE_LENGTH_SQUARED(N, T)                                                    \
DEFINE_LENGTH(N, T)                                                            \
DEFINE_DOT_PRODUCT(N, T)                                                       \
DEFINE_NORMALIZE(N, T)

#define DEFINE_VECTOR_OPERATIONS_3D(T)                                         \
DEFINE_CROSS_PRODUCT(T)                                                        \
DEFINE_REFLECT(3, T)                                                           \
DEFINE_REFRACT(3, T)

DEFINE_VECTOR(2, f)
DEFINE_VECTOR(2, d)
DEFINE_VECTOR(3, f)
DEFINE_VECTOR(3, d)
DEFINE_VECTOR(4, f)
DEFINE_VECTOR(4, d)
DEFINE_VECTOR_OPERATIONS(2, f)
DEFINE_VECTOR_OPERATIONS(2, d)
DEFINE_VECTOR_OPERATIONS(3, f)
DEFINE_VECTOR_OPERATIONS(3, d)
DEFINE_VECTOR_OPERATIONS(4, f)
DEFINE_VECTOR_OPERATIONS(4, d)
DEFINE_VECTOR_OPERATIONS_3D(f)
DEFINE_VECTOR_OPERATIONS_3D(d)

#undef DEFINE_VECTOR
#undef DEFINE_ADD
#undef DEFINE_SUBTRACT
#undef DEFINE_MULTIPLY
#undef DEFINE_DIVIDE
#undef DEFINE_NEGATE
#undef DEFINE_LENGTH_SQUARED

#undef DEFINE_LENGTH
#undef DEFINE_DOT_PRODUCT
#undef DEFINE_CROSS_PRODUCT
#undef DEFINE_NORMALIZE
#undef DEFINE_REFLECT
#undef DEFINE_REFRACT

#undef DEFINE_VECTOR_OPERATIONS
#undef DEFINE_VECTOR_OPERATIONS_3D

#undef TYPE_MAP
#undef TYPE_MAP_f
#undef TYPE_MAP_d

#endif // _VECTOR_H_
