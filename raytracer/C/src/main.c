#include <stdlib.h>

#include "camera.h"
#include "material.h"
#include "object.h"

/*! @brief Main function. */
int main(void) {
    objects_t o = { NULL, 10 };
    o.spheres = (sphere_t *)malloc(o.size * sizeof(sphere_t));

    lambertian_t mat_ground = { 0.115, 0.115, 0.105 };
    lambertian_t mat_artic = { 0.27, 0.74, 0.7 };
    lambertian_t mat_blue = { 0.1, 0.2, 0.5 };
    lambertian_t mat_light = { 0.73, 0.73, 0.73 };
    dielectric_t mat_bubble_outside = { 1.5 };
    dielectric_t mat_bubble_inside = { 1.0 / 1.5 };
    metal_t mat_bronze = { { 0.8, 0.6, 0.2 }, 0.9 };
    metal_t mat_gold = { { 0.8, 0.7, 0.1 }, 0.2 };
    metal_t mat_mirror = { { 0.7, 0.6, 0.5 }, 0.0 };
    dielectric_t mat_glass = { 1.5 };

    o.spheres[0].center = (vec3f_t){ 0, -100.5, -1 };
    o.spheres[0].radius = 100;
    o.spheres[0].mat.type = MATERIAL_LAMBERTIAN;
    o.spheres[0].mat.mat = &mat_ground;

    o.spheres[1].center = (vec3f_t){ 0, 0, -1.2 };
    o.spheres[1].radius = 0.5;
    o.spheres[1].mat.type = MATERIAL_LAMBERTIAN;
    o.spheres[1].mat.mat = &mat_artic;

    o.spheres[2].center = (vec3f_t){ -1, 0, -1 };
    o.spheres[2].radius = 0.5;
    o.spheres[2].mat.type = MATERIAL_DIELECTRIC;
    o.spheres[2].mat.mat = &mat_bubble_outside;

    o.spheres[3].center = (vec3f_t){ -1, 0, -1 };
    o.spheres[3].radius = 0.4;
    o.spheres[3].mat.type = MATERIAL_DIELECTRIC;
    o.spheres[3].mat.mat = &mat_bubble_inside;

    o.spheres[4].center = (vec3f_t){ 1, 0, -1 };
    o.spheres[4].radius = 0.5;
    o.spheres[4].mat.type = MATERIAL_METAL;
    o.spheres[4].mat.mat = &mat_bronze;

    o.spheres[5].center = (vec3f_t){ -0.3, 0.5, -2.5 };
    o.spheres[5].radius = 1;
    o.spheres[5].mat.type = MATERIAL_METAL;
    o.spheres[5].mat.mat = &mat_mirror;

    o.spheres[6].center = (vec3f_t){ 0.55, -0.38, -0.65 };
    o.spheres[6].radius = 0.2;
    o.spheres[6].mat.type = MATERIAL_DIELECTRIC;
    o.spheres[6].mat.mat = &mat_glass;

    o.spheres[7].center = (vec3f_t){ -0.4, -0.38, -0.4 };
    o.spheres[7].radius = 0.2;
    o.spheres[7].mat.type = MATERIAL_LAMBERTIAN;
    o.spheres[7].mat.mat = &mat_blue;

    o.spheres[8].center = (vec3f_t){ 0, 0.65, 1.2 };
    o.spheres[8].radius = 1;
    o.spheres[8].mat.type = MATERIAL_LAMBERTIAN;
    o.spheres[8].mat.mat = &mat_light;

    o.spheres[9].center = (vec3f_t){ -4.7, 2.4, 3.1 };
    o.spheres[9].radius = 3;
    o.spheres[9].mat.type = MATERIAL_METAL;
    o.spheres[9].mat.mat = &mat_gold;

    camera_orientation_t orientation = { { 1, 1, 1 }, { 0, 0, 0 },
        { 0, 1, 0 } };
    camera_image_t image = { 100, 1 };
    camera_lens_t lens = { 30, 6, 3.4 };
    camera_sampling_t sampling = { 10, 10 };

    orientation.look_from = (vec3f_t){ -2, 2, 1 };
    orientation.look_at = (vec3f_t){ 0, 0, -1 };
    orientation.up = (vec3f_t){ 0, 1, 0 };
    image.img_w = 640;
    image.aspect_ratio = 16.0 / 9.0;

    camera_t camera = CameraCreate(orientation, image, lens, sampling);
    CameraRender(&o, &camera);
    CameraDestroy(&camera);
    free(o.spheres);

    return 0;
}
