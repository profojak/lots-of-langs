#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdio.h>

#include "material.h"
#include "object.h"
#include "ray.h"
#include "vector.h"

/*! @brief Orientation configuration. */
typedef struct {
    /*! @brief Center. */
    vec3f_t look_from;
    /*! @brief Focus point. */
    vec3f_t look_at;
    /*! @brief Up vector. */
    vec3f_t up;
} camera_orientation_t;

/*! @brief Output image configuration. */
typedef struct {
    /*! @brief Image width. */
    int img_w;
    /*! @brief Aspect ratio. */
    float aspect_ratio;
} camera_image_t;

/*! @brief Lens configuration. */
typedef struct {
    /*! @brief Vertical field of view. */
    float vertical_fov;
    /*! @brief Defocus angle. */
    float defocus_ang;
    /*! @brief Focus distance. */
    float focus_distance;
} camera_lens_t;

/*! @brief Sampling configuration. */
typedef struct {
    /*! @brief Number of samples per pixel. */
    int samples;
    /*! @brief Maximum depth of ray tracing recursion. */
    int max_depth;
} camera_sampling_t;

/*! @brief Camera. */
typedef struct {
    /*! @brief Orientation configuration. */
    camera_orientation_t orientation;
    /*! @brief Output image configuration. */
    camera_image_t image;
    /*! @brief Lens configuration. */
    camera_lens_t lens;
    /*! @brief Sampling configuration. */
    camera_sampling_t sampling;

    /*! @brief Output image file. */
    FILE *file;
    /*! @brief Image height. */
    int img_h;
    /*! @brief Weight of single sample on final pixel color. */
    float px_sample_w;

    /*! @brief Position of upper left pixel. */
    vec3f_t px_up_left;
    /*! @brief Horizontal change of pixel position. */
    vec3f_t px_delta_u;
    /*! @brief Vertical change of pixel position. */
    vec3f_t px_delta_v;
    /*! @brief Horizontal change of defocus disk. */
    vec3f_t defocus_disk_u;
    /*! @brief Vertical change of defocus disk. */
    vec3f_t defocus_disk_v;
} camera_t;

/**
 * @brief Create camera.
 * @param[in] orientation Orientation configuration.
 * @param[in] image Output image configuration.
 * @param[in] lens Lens configuration.
 * @param[in] sampling Sampling configuration.
 */
camera_t CameraCreate(const camera_orientation_t orientation,
    const camera_image_t image, const camera_lens_t lens,
    const camera_sampling_t sampling);

/**
 * @brief Destroy camera.
 * @param[in] camera Camera.
 */
void CameraDestroy(camera_t *camera);

/**
 * @brief Render scene.
 * @param[in] scene Scene.
 * @param[in] camera Camera.
 */
void CameraRender(const objects_t *scene, const camera_t *camera);

/**
 * @brief Get ray for given pixel.
 * @param[in] x X coordinate of pixel.
 * @param[in] y Y coordinate of pixel.
 * @param[in] camera Camera.
 */
ray_t CameraGetRay(const int i, const int j, const camera_t *camera);

/**
 * @brief Trace ray.
 * @param[in] ray Ray.
 * @param[in] scene Scene.
 * @param[in] depth Current recursion depth.
 * @return Color of pixel.
 */
color_t CameraTraceRay(const ray_t *ray, const objects_t *objects,
    const int depth);

/**
 * @brief Write pixel color to file.
 * @param[in] color Color.
 * @param[in] camera Camera.
 */
void CameraWriteColor(const color_t color, const camera_t *camera);

#endif // _CAMERA_H_
