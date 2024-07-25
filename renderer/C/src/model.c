#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model.h"

model_t ModelLoad(const char *filename) {
    model_t model = {0};
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error loading model file %s\n", filename);
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'v' && line[1] == ' ') {
            vec3f_t v;
            sscanf(line, "v %f %f %f\n", &v.data[0], &v.data[1], &v.data[2]);
            model.verts = (vec3f_t *)realloc(model.verts,
                (model.verts_num + 1) * sizeof(vec3f_t));
            model.verts[model.verts_num++] = v;

        } else if (line[0] == 'v' && line[1] == 'n') {
            vec3f_t n;
            sscanf(line, "vn %f %f %f\n", &n.data[0], &n.data[1], &n.data[2]);
            model.norms = (vec3f_t *)realloc(model.norms,
                (model.norms_num + 1) * sizeof(vec3f_t));
            model.norms[model.norms_num++] = n;

        } else if (line[0] == 'v' && line[1] == 't') {
            vec2f_t uv;
            sscanf(line, "vt %f %f\n", &uv.data[0], &uv.data[1]);
            model.texels = (vec2f_t *)realloc(model.texels,
                (model.texels_num + 1) * sizeof(vec2f_t));
            model.texels[model.texels_num++] =
                (vec2f_t){ uv.data[0], 1 - uv.data[1] };

        } else if (line[0] == 'f' && line[1] == ' ') {
            int face[3], texel[3], normal[3];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", &face[0], &texel[0],
                &normal[0], &face[1], &texel[1], &normal[1], &face[2],
                &texel[2], &normal[2]);
            for (int i = 0; i < 3; i++) {
                model.facet_verts = (int *)realloc(model.facet_verts,
                    (model.facet_verts_num + 1) * sizeof(int));
                model.facet_texels = (int *)realloc(model.facet_texels,
                    (model.facet_texels_num + 1) * sizeof(int));
                model.facet_norms = (int *)realloc(model.facet_norms,
                    (model.facet_norms_num + 1) * sizeof(int));
                model.facet_verts[model.facet_verts_num++] = face[i] - 1;
                model.facet_texels[model.facet_texels_num++] = texel[i] - 1;
                model.facet_norms[model.facet_norms_num++] = normal[i] - 1;
            }
        }
    }

    char *dot = strrchr(filename, '.');
    if (dot) {
        char texture[128] = {0};
        strncpy(texture, filename, dot - filename + 1);
        strcpy(texture + (dot - filename + 1), "tga");
        if (ImageReadTgaFile(texture, &model.texture)) {
            fprintf(stderr, "Error loading texture file %s\n", texture);
            exit(1);
        }
    } else {
        fprintf(stderr, "Error opening texture file %s\n", filename);
        exit(1);
    }

    fclose(fp);
    return model;
}
