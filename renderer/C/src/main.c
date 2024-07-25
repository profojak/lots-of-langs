#include <stdio.h>
#include <stdlib.h>

#include "shader.h"
#include "image.h"

/*! @brief Image width. */
#define WIDTH 640
/*! @brief Image height. */
#define HEIGHT 640

/*! @brief Main function. */
int main(const int argc, const char *argv[]) {
    if (2 > argc) {
        printf("Usage: %s model.obj\n", argv[0]);
        exit(1);
    }

    image_t image = ImageCreate(WIDTH, HEIGHT, RGB);
    for (int arg = 1; arg < argc; arg++) {
        char path[256];
        int len = snprintf(path, sizeof(path), "../.obj/%s", argv[arg]);
        path[len] = '\0';
        model_t model = ModelLoad(path);
        shader_t shader;
        shader.model = &model;
        for (int i = 0; i < model.facet_verts_num / 3; i++) {
            ShaderLoadTriangle(i, &shader);
            ShaderRenderTriangle(&image, &shader);
        }
    }

    ImageFlipVertically(&image);
    ImageWriteTgaFile("image.tga", 1, &image);

    ImageDestroy(&image);
    return 0;
}
