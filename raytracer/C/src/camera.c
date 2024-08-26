#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "camera.h"
#include "random.h"

#define M_PI 3.14159265358979323846

camera_t CameraCreate(const camera_orientation_t orientation,
    const camera_image_t image, const camera_lens_t lens,
    const camera_sampling_t sampling) {
    camera_t camera;
    camera.orientation = orientation;
    camera.image = image;
    camera.lens = lens;
    camera.sampling = sampling;

    camera.img_h = (int)(image.img_w / image.aspect_ratio);
    camera.file = fopen("image.ppm", "w");
    if (!camera.file) {
        fprintf(stderr, "Failed to open image file\n");
        exit(1);
    }

    const float viewport_h = 2 * tanf((lens.vertical_fov * M_PI / 180) / 2) *
        lens.focus_distance;
    const float viewport_w = viewport_h * ((float)image.img_w / camera.img_h);

    const vec3f_t w = Normalize3f(Subtract3f(orientation.look_from,
        orientation.look_at));
    const vec3f_t u = Normalize3f(CrossProductf(orientation.up, w));
    const vec3f_t v = CrossProductf(w, u);
    const vec3f_t viewport_u = Multiply3f(u, viewport_w);
    const vec3f_t viewport_v = Multiply3f(Negate3f(v), viewport_h);
    vec3f_t viewport_up_left = Subtract3f(orientation.look_from,
        Multiply3f(w, lens.focus_distance));
    viewport_up_left = Subtract3f(viewport_up_left,
        Multiply3f(viewport_u, 0.5));
    viewport_up_left = Subtract3f(viewport_up_left,
        Multiply3f(viewport_v, 0.5));

    camera.px_delta_u = Divide3f(viewport_u, image.img_w);
    camera.px_delta_v = Divide3f(viewport_v, camera.img_h);
    camera.px_up_left = Add3f(viewport_up_left,
        Multiply3f(Add3f(camera.px_delta_u, camera.px_delta_v), 0.5));
    camera.px_sample_w = 1.0 / sampling.samples;

    const float defocus_radius = lens.focus_distance *
        tanf((lens.defocus_ang * M_PI / 180) / 2);
    camera.defocus_disk_u = Multiply3f(u, defocus_radius);
    camera.defocus_disk_v = Multiply3f(v, defocus_radius);

    return camera;
}

void CameraDestroy(camera_t *camera) {
    fclose(camera->file);
}

void CameraRender(const objects_t *scene, const camera_t *camera) {
    color_t colors[camera->image.img_w * camera->img_h];

    printf("Ray tracing on 1 thread...\n");

    for (int y = 0; y < camera->img_h; y++) {
        for (int x = 0; x < camera->image.img_w; x++) {
            color_t color = { 0 };
            for (int s = 0; s < camera->sampling.samples; s++) {
                ray_t ray = CameraGetRay(x, y, camera);
                color = Add3f(color, CameraTraceRay(&ray, scene,
                    camera->sampling.max_depth));
            }
            colors[y * camera->image.img_w + x] =
                Multiply3f(color, camera->px_sample_w);
        }
    }

    fprintf(camera->file, "P3\n%d %d\n255\n", camera->image.img_w,
        camera->img_h);
    for (int i = 0; i < camera->img_h; i++)
        for (int j = 0; j < camera->image.img_w; j++)
            CameraWriteColor(colors[i * camera->image.img_w + j], camera);
    printf("Done!\n");
}

ray_t CameraGetRay(const int x, const int y, const camera_t *camera) {
    float offset_x = RandomFloat() - 0.5;
    float offset_y = RandomFloat() - 0.5;
    vec3f_t pixel_sample = Add3f(camera->px_up_left,
        Multiply3f(camera->px_delta_u, x + offset_x));
    pixel_sample = Add3f(pixel_sample, Multiply3f(camera->px_delta_v,
        y + offset_y));
    vec3f_t p = RandomVectorUnitDisk();
    vec3f_t orig = Add3f(camera->orientation.look_from,
        Multiply3f(camera->defocus_disk_u, p.data[0]));
    orig = Add3f(orig, Multiply3f(camera->defocus_disk_v, p.data[1]));
    return (ray_t){ orig, Subtract3f(pixel_sample, orig) };
}

color_t CameraTraceRay(const ray_t *ray, const objects_t *scene,
    const int depth) {
    if (depth <= 0)
        return (color_t){ 0, 0, 0 };

    hit_t hit = { 0 };
    if (CheckHit(ray, (interval_t){ 0.001, INFINITY }, scene, &hit)) {
        color_t attenuation = { 0 };
        ray_t scattered = { 0 };
        if (MaterialScatter(ray, &hit, &attenuation, &scattered)) {
            color_t color = CameraTraceRay(&scattered, scene, depth - 1);
            for (int i = 0; i < 3; i++)
                color.data[i] = attenuation.data[i] * color.data[i];
            return color;
        }
        return (color_t){ 0, 0, 0 };
    }

    const float t = 0.5 * (Normalize3f(ray->dir).data[1] + 1);
    return Add3f(Multiply3f((color_t){ 1, 1, 1 }, 1 - t),
        Multiply3f((color_t){ 0.5, 0.7, 1 }, t));
}

void CameraWriteColor(const color_t color, const camera_t *camera) {
    interval_t intensity = { 0, 1 - 0.0001 };
    float r = color.data[0];
    float g = color.data[1];
    float b = color.data[2];
    r = r > 0 ? sqrtf(r) : 0;
    g = g > 0 ? sqrtf(g) : 0;
    b = b > 0 ? sqrtf(b) : 0;
    fprintf(camera->file, "%d %d %d\n",
        (int)(256 * IntervalClamp(r, intensity)),
        (int)(256 * IntervalClamp(g, intensity)),
        (int)(256 * IntervalClamp(b, intensity)));
}
