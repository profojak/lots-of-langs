#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"

image_t ImageCreate(const int w, const int h, const uint8_t bpp) {
    image_t image;
    image.data = (uint8_t *)malloc(w * h * bpp);
    memset(image.data, 0, w * h * bpp);
    image.zbuf = (float *)malloc(w * h * sizeof(float));
    for (int i = 0; i < w * h; i++)
        image.zbuf[i] = -FLT_MAX;
    image.w = w;
    image.h = h;
    image.bpp = bpp;
    return image;
}

void ImageDestroy(image_t *image) {
    free(image->data);
    free(image->zbuf);
    image->data = NULL;
    image->zbuf = NULL;
    image->w = 0;
    image->h = 0;
    image->bpp = 0;
}

int ImageReadTgaFile(const char *filename, image_t *image) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    header_t header = {0};
    if (fread(&header, sizeof(header_t), 1, fp) != 1) {
        fprintf(stderr, "Error reading header\n");
        fclose(fp);
        return 1;
    }

    image->w = header.w;
    image->h = header.h;
    image->bpp = header.pixel_depth / 8;
    if (image->w <= 0 || image->h <= 0 ||
        (image->bpp != 1 && image->bpp != 3 && image->bpp != 4)) {
        fprintf(stderr, "Bad image format\n");
        fclose(fp);
        return 1;
    }

    image->data = (uint8_t *)malloc(image->w * image->h * image->bpp);
    image->zbuf = (float *)malloc(image->w * image->h * sizeof(float));
    for (int i = 0; i < image->w * image->h; i++)
        image->zbuf[i] = -FLT_MAX;

    if (3 == header.img_type || 2 == header.img_type) {
        if (fread(image->data, image->w * image->h * image->bpp, 1, fp) != 1) {
            fprintf(stderr, "Error reading data\n");
            fclose(fp);
            return 1;
        }
    } else if (10 == header.img_type || 11 == header.img_type) {
        if (ImageLoadRleData(fp, image)) {
            fprintf(stderr, "Error reading data\n");
            fclose(fp);
            return 1;
        }
    } else {
        fprintf(stderr, "Unknown file format\n");
        fclose(fp);
        return 1;
    }

    if(!(header.img_desc & 0x20))
        ImageFlipVertically(image);
    if(header.img_desc & 0x10)
        ImageFlipHorizontally(image);

    fclose(fp);
    return 0;
}

int ImageWriteTgaFile(const char *filename, const int rle,
    const image_t *image) {
    const uint8_t developer_area_ref[4] = {0};
    const uint8_t extension_area_ref[4] = {0};
    const char footer[18] = "TRUEVISION-XFILE.";

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    header_t header = {0};
    header.pixel_depth = image->bpp * 8;
    header.w = image->w;
    header.h = image->h;
    header.img_type = (image->bpp == 1 ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header.img_desc = 0x20;
    if (fwrite(&header, sizeof(header_t), 1, fp) != 1) {
        fprintf(stderr, "Error writing header\n");
        fclose(fp);
        return 1;
    }

    if (!rle) {
        if (fwrite(image->data, image->w * image->h * image->bpp, 1, fp) != 1) {
            fprintf(stderr, "Error writing data\n");
            fclose(fp);
            return 1;
        }
    } else {
        if (ImageUnloadRleData(fp, image)) {
            fprintf(stderr, "Error writing data\n");
            fclose(fp);
            return 1;
        }
    }

    if (fwrite(developer_area_ref, sizeof(uint8_t), 4, fp) != 4) {
        fprintf(stderr, "Error writing data\n");
        fclose(fp);
        return 1;
    }

    if (fwrite(extension_area_ref, sizeof(uint8_t), 4, fp) != 4) {
        fprintf(stderr, "Error writing data\n");
        fclose(fp);
        return 1;
    }

    if (fwrite(footer, sizeof(char), 18, fp) != 18) {
        fprintf(stderr, "Error writing data\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}

int ImageLoadRleData(FILE *fp, image_t *image) {
    const int pcount = image->w * image->h;
    int p = 0;
    int byte = 0;
    pixel_t pixel = {0};

    do {
        uint8_t chunk_header = 0;
        if (fread(&chunk_header, sizeof(uint8_t), 1, fp) != 1) {
            fprintf(stderr, "Error reading chunk header\n");
            return 1;
        }
        
        if (chunk_header < 128) {
            chunk_header++;
            for (uint8_t i = 0; i < chunk_header; i++) {
                if (fread(&pixel, image->bpp, 1, fp) != 1) {
                    fprintf(stderr, "Error reading data\n");
                    return 1;
                }
                for (uint8_t t = 0; t < image->bpp; t++)
                    image->data[byte++] = pixel.data[t];
                p++;
                if (p > pcount) {
                    fprintf(stderr, "Too many pixels read\n");
                    return 1;
                }
            }
        } else {
            chunk_header -= 127;
            if (fread(&pixel, image->bpp, 1, fp) != 1) {
                fprintf(stderr, "Error reading data\n");
                return 1;
            }
            for (uint8_t i = 0; i < chunk_header; i++) {
                for (uint8_t t = 0; t < image->bpp; t++)
                    image->data[byte++] = pixel.data[t];
                p++;
                if (p > pcount) {
                    fprintf(stderr, "Too many pixels read\n");
                    return 1;
                }
            }
        }
    } while (p < pcount);

    return 0;
}

int ImageUnloadRleData(FILE *fp, const image_t *image) {
    const uint8_t max_chunk_len = 128;
    int pcount = image->w * image->h;
    int p = 0;

    while (p < pcount) {
        const int chunk_start = p * image->bpp;
        int byte = p * image->bpp;
        int run_len = 1;
        int raw = 1;

        while (p + run_len < pcount && run_len < max_chunk_len) {
            int succ_eq = 1;
            for (uint8_t t = 0; succ_eq && t < image->bpp; t++)
                succ_eq = (image->data[byte + t] ==
                    image->data[(byte + image->bpp) + t]) ? 1 : 0;
            byte += image->bpp;
            if (1 == run_len)
                raw = succ_eq ? 0 : 1;
            if (raw && succ_eq) {
                run_len--;
                break;
            }
            if (!raw && !succ_eq)
                break;
            run_len++;
        }

        p += run_len;
        if (fputc(raw ? run_len - 1 : run_len + 127, fp) == EOF) {
            fprintf(stderr, "Error writing to file\n");
            return 1;
        }

        if (fwrite(image->data + chunk_start,
            (raw ? run_len * image->bpp : image->bpp), 1, fp) != 1) {
            fprintf(stderr, "Error writing to file\n");
            return 1;
        }
    }

    return 0;
}

void ImageSetPixel(const int x, const int y, const float z, const pixel_t pixel,
    image_t *image) {
    if (x < 0 || y < 0 || x >= image->w || y >= image->h ||
        z < image->zbuf[x + y * image->w])
        return;
    memcpy(image->data + (x + y * image->w) * image->bpp, pixel.data,
        image->bpp);
    image->zbuf[x + y * image->w] = z;
}

pixel_t ImageGetPixel(const int x, const int y, const image_t *image) {
    pixel_t pixel = {0};
    if (x < 0 || y < 0 || x >= image->w || y >= image->h)
        return pixel;
    memcpy(pixel.data, image->data + (x + y * image->w) * image->bpp,
        image->bpp);
    return pixel;
}

void ImageFlipHorizontally(image_t *image) {
    const int half = image->w / 2;
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < image->h; j++) {
            for (uint8_t b = 0; b < image->bpp; b++) {
                const int i1 = (i + j * image->w) * image->bpp + b;
                const int i2 = (image->w - 1 - i + j * image->w) * image->bpp + b;
                const uint8_t t = image->data[i1];
                image->data[i1] = image->data[i2];
                image->data[i2] = t;
            }
        }
    }
}

void ImageFlipVertically(image_t *image) {
    const int half = image->h / 2;
    for (int i = 0; i < image->w; i++) {
        for (int j = 0; j < half; j++) {
            for (uint8_t b = 0; b < image->bpp; b++) {
                const int i1 = (i + j * image->w) * image->bpp + b;
                const int i2 = (i + (image->h - 1 - j) * image->w) * image->bpp + b;
                const uint8_t t = image->data[i1];
                image->data[i1] = image->data[i2];
                image->data[i2] = t;
            }
        }
    }
}
